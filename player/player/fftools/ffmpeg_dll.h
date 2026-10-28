#ifndef FFMPEG_DLL_H
#define FFMPEG_DLL_H

#ifdef FFMPEG_BUILD_DLL
#define FFMPEG_API __declspec(dllexport)
#else
#define FFMPEG_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* One-shot ffmpeg CLI entry point (thin wrapper around ffmpeg_task_run). */
FFMPEG_API int ffmpeg_main(int argc, char **argv);

/*
 * Reusable, self-contained ffmpeg job object.
 *
 * Each FFmpegTask owns all of its own state (an internal FFmpegContext), so
 * multiple tasks may be created and run concurrently, one per host thread,
 * each with its own command line. A task never terminates the host process;
 * errors are reported through ffmpeg_task_run()'s return value.
 */
typedef struct FFmpegTask FFmpegTask;

/* Allocate a task object. Returns NULL on allocation failure. */
FFMPEG_API FFmpegTask *ffmpeg_task_create(void);

/*
 * Free a task object and set *ptask to NULL. Must not be called while
 * ffmpeg_task_run() is executing on the same task.
 */
FFMPEG_API void ffmpeg_task_destroy(FFmpegTask **ptask);

/*
 * Run one ffmpeg command line in this task. Blocking (synchronous); returns
 * the ffmpeg exit code (0 on success, or a negative AVERROR on error).
 */
FFMPEG_API int ffmpeg_task_run(FFmpegTask *task, int argc, char **argv);

/*
 * Request a graceful abort of a task currently running. Safe to call from a
 * thread other than the one executing ffmpeg_task_run().
 */
FFMPEG_API void ffmpeg_task_cancel(FFmpegTask *task);

#ifdef __cplusplus
}
#endif

#endif /* FFMPEG_DLL_H */
