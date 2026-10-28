/*
 * Concurrency self-test for the objectised FFmpegTask API.
 *
 * Proves that several ffmpeg jobs can run at the same time, each on its own
 * thread, with its own command line, without sharing any mutable global state.
 * Uses the lavfi "testsrc" source so no input file is required.
 *
 * Build (see build_ffmpeg_mt_test.bat):
 *   cl.exe /nologo /std:c11 test_ffmpeg_mt.c /I fftools bin\ffmpeg.lib \
 *          /Fe:bin\test_ffmpeg_mt.exe
 *
 * Run from bin\ so that ffmpeg.dll and the av*.dll are found next to the exe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdatomic.h>

#include "ffmpeg_dll.h"

#define N_TASKS 4

typedef struct {
    int         index;
    FFmpegTask *task;
    int         argc;
    char      **argv;
    int         ret;
} Job;

/* simple startup barrier so all worker threads begin the transcode together */
static atomic_int ready_count = 0;
static atomic_int start_flag  = 0;

static char *sdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

static void free_argv(int argc, char **argv)
{
    int i;
    if (!argv)
        return;
    for (i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}

/* Build a per-task command line. Each task transcodes a different-sized
 * testsrc so the produced files must differ in size if isolation holds. */
static int build_argv(int index, const char *out_dir,
                      int *argc_out, char ***argv_out)
{
    char input[160];
    char output[512];
    int w = 160 + index * 24;
    int h = 120 + index * 24;
    int argc = 12;
    char **argv;

    snprintf(input,  sizeof(input),  "testsrc=duration=2:size=%dx%d:rate=30", w, h);
    snprintf(output, sizeof(output), "%s\\out_mt_%d.avi", out_dir, index);

    argv = (char **)calloc((size_t)argc, sizeof(char *));
    if (!argv)
        return -1;

    argv[0]  = sdup("ffmpeg_test");
    argv[1]  = sdup("-hide_banner");
    argv[2]  = sdup("-loglevel");
    argv[3]  = sdup("error");
    argv[4]  = sdup("-f");
    argv[5]  = sdup("lavfi");
    argv[6]  = sdup("-i");
    argv[7]  = sdup(input);
    argv[8]  = sdup("-c:v");
    argv[9]  = sdup("mpeg4");
    argv[10] = sdup("-y");
    argv[11] = sdup(output);

    *argc_out = argc;
    *argv_out = argv;
    return 0;
}

static DWORD WINAPI worker(LPVOID p)
{
    Job *j = (Job *)p;

    atomic_fetch_add(&ready_count, 1);
    while (!atomic_load(&start_flag))
        Sleep(0);   /* spin until the main thread releases all workers */

    j->ret = ffmpeg_task_run(j->task, j->argc, j->argv);
    return 0;
}

static long long file_size(const char *path)
{
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER sz;
    if (h == INVALID_HANDLE_VALUE)
        return -1;
    GetFileSizeEx(h, &sz);
    CloseHandle(h);
    return (long long)sz.QuadPart;
}

static int run_concurrency_test(const char *out_dir)
{
    Job jobs[N_TASKS];
    HANDLE threads[N_TASKS];
    int i;
    int failures = 0;
    long long sizes[N_TASKS];
    DWORD t0, t1;

    memset(jobs, 0, sizeof(jobs));

    for (i = 0; i < N_TASKS; i++) {
        jobs[i].index = i;
        jobs[i].task  = ffmpeg_task_create();
        if (!jobs[i].task || build_argv(i, out_dir, &jobs[i].argc, &jobs[i].argv) < 0) {
            printf("[concurrency] FAIL: setup for task %d failed\n", i);
            return 1;
        }
    }

    atomic_store(&ready_count, 0);
    atomic_store(&start_flag, 0);

    t0 = GetTickCount();
    for (i = 0; i < N_TASKS; i++) {
        threads[i] = CreateThread(NULL, 0, worker, &jobs[i], 0, NULL);
        if (!threads[i]) {
            printf("[concurrency] FAIL: CreateThread %d failed\n", i);
            return 1;
        }
    }

    /* release all workers at once once they are all ready */
    while (atomic_load(&ready_count) < N_TASKS)
        Sleep(1);
    atomic_store(&start_flag, 1);

    WaitForMultipleObjects(N_TASKS, threads, TRUE, INFINITE);
    t1 = GetTickCount();

    for (i = 0; i < N_TASKS; i++) {
        CloseHandle(threads[i]);
        sizes[i] = -1;
    }

    printf("[concurrency] all %d tasks finished in %lu ms (wall clock)\n",
           N_TASKS, (unsigned long)(t1 - t0));

    for (i = 0; i < N_TASKS; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s\\out_mt_%d.avi", out_dir, i);

        if (jobs[i].ret != 0) {
            printf("[concurrency] FAIL: task %d returned %d\n", i, jobs[i].ret);
            failures++;
            continue;
        }

        sizes[i] = file_size(path);
        if (sizes[i] <= 0) {
            printf("[concurrency] FAIL: task %d output missing/empty (%s)\n",
                   i, path);
            failures++;
        }
    }

    /* every output must differ in size -> proves parameters were not crossed */
    for (i = 1; i < N_TASKS; i++) {
        if (sizes[i] > 0 && sizes[i - 1] > 0 && sizes[i] == sizes[i - 1]) {
            printf("[concurrency] FAIL: task %d and %d produced identical sizes (%lld)\n",
                   i - 1, i, sizes[i]);
            failures++;
        }
    }

    for (i = 0; i < N_TASKS; i++) {
        printf("[concurrency] task %d -> ret=%d size=%lld bytes\n",
               i, jobs[i].ret, sizes[i]);
        ffmpeg_task_destroy(&jobs[i].task);
        free_argv(jobs[i].argc, jobs[i].argv);
    }

    printf("[concurrency] %s\n", failures ? "FAILED" : "PASSED");
    return failures ? 1 : 0;
}

/* Cancellation smoke test: a near-infinite job must stop promptly on cancel. */
static int run_cancel_test(const char *out_dir)
{
    FFmpegTask *task = ffmpeg_task_create();
    int argc = 11;
    char **argv;
    DWORD t0, t1;
    int ret;

    (void)out_dir;

    if (!task)
        return 1;

    argv = (char **)calloc((size_t)argc, sizeof(char *));
    if (!argv) {
        ffmpeg_task_destroy(&task);
        return 1;
    }

    argv[0]  = sdup("ffmpeg_test");
    argv[1]  = sdup("-hide_banner");
    argv[2]  = sdup("-loglevel");
    argv[3]  = sdup("error");
    argv[4]  = sdup("-f");
    argv[5]  = sdup("lavfi");
    argv[6]  = sdup("-i");
    argv[7]  = sdup("testsrc=duration=10:size=64x64:rate=5");
    argv[8]  = sdup("-f");
    argv[9]  = sdup("null");
    argv[10] = sdup("-");

    t0 = GetTickCount();
    /* run on a worker thread so the main thread can request the cancel */
    {
        Job j;
        HANDLE h;
        j.index = 0;
        j.task  = task;
        j.argc  = argc;
        j.argv  = argv;
        j.ret   = -1;
        h = CreateThread(NULL, 0, worker, &j, 0, NULL);
        if (!h) {
            ffmpeg_task_destroy(&task);
            free_argv(argc, argv);
            return 1;
        }
        atomic_store(&start_flag, 1);   /* release the worker barrier */
        Sleep(1000);                    /* let it run for ~1s */
        ffmpeg_task_cancel(task);       /* request graceful abort */
        WaitForSingleObject(h, 15000);  /* must finish well before 3600s */
        CloseHandle(h);
        ret = j.ret;
    }
    t1 = GetTickCount();

    printf("[cancel] task returned %d after %lu ms (expect << 3600000)\n",
           ret, (unsigned long)(t1 - t0));
    printf("[cancel] %s\n", (t1 - t0) < 15000 ? "PASSED" : "FAILED");

    ffmpeg_task_destroy(&task);
    free_argv(argc, argv);
    return (t1 - t0) < 15000 ? 0 : 1;
}

int main(int argc, char **argv)
{
    const char *out_dir = argc > 1 ? argv[1] : ".";
    int r1, r2;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("=== FFmpegTask concurrency self-test ===\n");

    r1 = run_concurrency_test(out_dir);
    r2 = run_cancel_test(out_dir);

    printf("=== %s ===\n", (r1 == 0 && r2 == 0) ? "ALL PASSED" : "SOME FAILED");
    return (r1 == 0 && r2 == 0) ? 0 : 1;
}
