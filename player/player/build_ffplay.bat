@echo off
set INCLUDE=D:\myPlayer\player\player\include;D:\myPlayer\player\player\include\sdl2;D:\myPlayer\player\player\fftools;D:\FFmpeg;D:\FFmpeg\compat;D:\FFmpeg\compat\atomics\win32;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
set LIB=D:\myPlayer\player\player\lib;D:\myPlayer\player\player\bin;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64
cd /d D:\myPlayer\player\player

REM Compile all fftools source files
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc /utf-8 /DFFPLAY_BUILD_DLL fftools\ffplayer.cpp
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\ffplay_renderer.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\cmdutils.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\opt_common.c

REM Link ffplay.dll
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\link.exe" /nologo /DLL /out:bin\ffplay.dll ffplayer.obj ffplay_renderer.obj cmdutils.obj opt_common.obj avcodec.lib avformat.lib avutil.lib swscale.lib swresample.lib avfilter.lib avdevice.lib SDL2.lib shell32.lib
