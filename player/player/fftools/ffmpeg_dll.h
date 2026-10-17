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

FFMPEG_API int ffmpeg_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* FFMPEG_DLL_H */
