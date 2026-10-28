@echo off
set INCLUDE=D:\myPlayer\player\player;D:\myPlayer\player\player\include;D:\myPlayer\player\player\include\sdl2;D:\myPlayer\player\player\fftools;D:\FFmpeg;D:\FFmpeg\compat;D:\FFmpeg\compat\atomics\win32;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
set LIB=D:\myPlayer\player\player\lib;D:\myPlayer\player\player\bin;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64
cd /d D:\myPlayer\player\player

REM Compile all fftools source files for ffmpeg DLL
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 /DFFMPEG_BUILD_DLL fftools\ffmpeg.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_dec.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_demux.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_enc.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_filter.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_hw.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_mux.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_mux_init.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_opt.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c11 fftools\ffmpeg_sched.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\cmdutils.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\opt_common.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\sync_queue.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\thread_queue.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\graph\graphprint.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\avtextformat.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_compact.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_default.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_flat.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_ini.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_json.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_mermaid.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tf_xml.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tw_avio.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tw_buffer.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\textformat\tw_stdout.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\resources\resman.c
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo fftools\resources\graph_resources.c

REM Link ffmpeg.dll
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\link.exe" /nologo /DLL /out:bin\ffmpeg.dll ffmpeg.obj ffmpeg_dec.obj ffmpeg_demux.obj ffmpeg_enc.obj ffmpeg_filter.obj ffmpeg_hw.obj ffmpeg_mux.obj ffmpeg_mux_init.obj ffmpeg_opt.obj ffmpeg_sched.obj cmdutils.obj opt_common.obj sync_queue.obj thread_queue.obj graphprint.obj avtextformat.obj tf_compact.obj tf_default.obj tf_flat.obj tf_ini.obj tf_json.obj tf_mermaid.obj tf_xml.obj tw_avio.obj tw_buffer.obj tw_stdout.obj resman.obj graph_resources.obj avcodec.lib avformat.lib avutil.lib swscale.lib swresample.lib avfilter.lib avdevice.lib shell32.lib
