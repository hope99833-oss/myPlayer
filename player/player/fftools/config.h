/*
 * config.h for MSVC Windows x64
 * FFmpeg 9.0.2 shared build
 */

#ifndef FFMPEG_CONFIG_H
#define FFMPEG_CONFIG_H

#define FFMPEG_CONFIGURATION "N/A"
#define FFMPEG_LICENSE "GPL v3"
#define FFMPEG_DATADIR "."
#define AVCONV_DATADIR "."

#define CONFIG_THIS_YEAR 2025
#define FFMPEG_CFLAGS ""
#define CC_IDENT "msvc18"

/* --- Platform --- */
#define ARCH_X86_64 1
#define ARCH_X86_32 0
#define ARCH_AARCH64 0
#define ARCH_ARM 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0
#define HAVE_ARCH_X86 1

/* --- OS --- */
#define HAVE_DOS_PATHS 1
#define HAVE_WINRT 0

/* --- Threading --- */
#define HAVE_W32THREADS 1
#define HAVE_PTHREADS 0
#define HAVE_OS2THREADS 0
#define HAVE_THREADS 1
#define HAVE_STDATOMIC 0
#define HAVE_ATOMICS_WIN32 1

/* --- Windows API --- */
#define HAVE_GETPROCESSTIMES 1
#define HAVE_GETPROCESSMEMORYINFO 1
#define HAVE_SETCONSOLECTRLHANDLER 1
#define HAVE_SETDLLDIRECTORY 1
#define HAVE_COMMANDLINETOARGVW 1
#define HAVE_GETMODULEHANDLE 1
#define HAVE_GETSTDHANDLE 1
#define HAVE_COINIT 1

/* --- POSIX (not available on MSVC) --- */
#define HAVE_IO_H 1
#define HAVE_UNISTD_H 0
#define HAVE_SYS_RESOURCE_H 0
#define HAVE_SYS_SELECT_H 0
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TERMIOS_H 0
#define HAVE_GETRUSAGE 0
#define HAVE_SETRLIMIT 0
#define HAVE_STRUCT_RUSAGE_RU_MAXRSS 0

/* --- MSVC CRT --- */
#define HAVE_KBHIT 1
#define HAVE_INLINE_ASM 0
#define HAVE_PRAGMA_DEPRECATED 1
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDBOOL_H 1

/* --- FFmpeg libraries --- */
#define CONFIG_AVDEVICE 1
#define CONFIG_AVFILTER 1
#define CONFIG_AVFORMAT 1
#define CONFIG_AVCODEC 1
#define CONFIG_AVUTIL 1
#define CONFIG_SWRESAMPLE 1
#define CONFIG_SWSCALE 1
#define CONFIG_POSTPROC 0

/* --- Optional features --- */
#define CONFIG_LIBPLACEBO 0
#define CONFIG_MEDIACODEC 0
#define CONFIG_VAAPI 0
#define CONFIG_QSV 0
#define CONFIG_RESOURCE_COMPRESSION 0

/* --- Network --- */
#define CONFIG_RTSP_DEMUXER 1
#define CONFIG_MMSH_PROTOCOL 1

/* --- License --- */
#define CONFIG_GPL 1
#define CONFIG_GPLV3 1
#define CONFIG_LGPL 0
#define CONFIG_LGPLV3 0
#define CONFIG_NONFREE 0
#define CONFIG_VERSION 1

/* --- Build config --- */
#define CONFIG_SMALL 0
#define CONFIG_GRAY 0
#define CONFIG_HARDCODED_TABLES 0
#define CONFIG_SAFE_BITSTREAM_READER 1
#define CONFIG_SHARED 1
#define CONFIG_STATIC 0

/* --- Math --- */
#define HAVE_LIBC_ISFINITE 0
#define HAVE_ISFINITE 1
#define HAVE_LDEXPF 1
#define HAVE_MALLOC_H 0
#define HAVE_MEMALIGN 0
#define HAVE_POSIX_MEMALIGN 0
#define HAVE_VIRTUALALLOC 1

#define HAVE_ALLOCA_H 0
#define HAVE_ALLOCA 1

#define HAVE_CLOCK_GETTIME 0
#define HAVE_GETTIMEOFDAY 0
#define HAVE_NANOSLEEP 0
#define HAVE_SLEEP 0
#define HAVE_USLEEP 0

#define HAVE_ATANF 1
#define HAVE_ATAN2F 1
#define HAVE_CBRTF 1
#define HAVE_COPYSIGN 1
#define HAVE_COSF 1
#define HAVE_ERF 0
#define HAVE_EXP2 0
#define HAVE_EXP2F 0
#define HAVE_EXPF 1
#define HAVE_HYPOT 1
#define HAVE_ISINF 1
#define HAVE_ISNAN 1
#define HAVE_LLRINT 0
#define HAVE_LLRINTF 0
#define HAVE_LOG10F 1
#define HAVE_LOG2F 1
#define HAVE_LOG2 1
#define HAVE_LOGF 1
#define HAVE_LROUNDF 1
#define HAVE_LROUND 1
#define HAVE_LRINT 1
#define HAVE_LRINTF 1
#define HAVE_POWF 1
#define HAVE_RINTF 1
#define HAVE_ROUNDF 1
#define HAVE_SINF 1
#define HAVE_TRUNC 1
#define HAVE_TRUNCF 1

#define HAVE_FAST_64BIT 1
#define HAVE_FAST_CLZ 1
#define HAVE_FAST_CMOV 1
#define HAVE_FAST_FLOAT16 0

#define HAVE_ATOMIC_WIN32_INTERLOCKED 1

#endif /* FFMPEG_CONFIG_H */
