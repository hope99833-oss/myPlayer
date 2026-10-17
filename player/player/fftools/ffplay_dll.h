#ifndef FFPLAY_DLL_H
#define FFPLAY_DLL_H

#ifdef FFPLAY_BUILD_DLL
#define FFPLAY_API __declspec(dllexport)
#else
#define FFPLAY_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

FFPLAY_API int ffplay_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* FFPLAY_DLL_H */
