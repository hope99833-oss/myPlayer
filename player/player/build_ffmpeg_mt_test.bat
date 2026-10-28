@echo off
set INCLUDE=D:\myPlayer\player\player;D:\myPlayer\player\player\include;D:\myPlayer\player\player\fftools;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
set LIB=D:\myPlayer\player\player\bin;D:\myPlayer\player\player\lib;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64
cd /d D:\myPlayer\player\player

"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /nologo /std:c11 /experimental:c11atomics test_ffmpeg_mt.c /I fftools bin\ffmpeg.lib /Fe:bin\test_ffmpeg_mt.exe

if errorlevel 1 exit /b 1

REM run from bin\ so ffmpeg.dll and the av*.dll are found next to the exe
pushd bin
test_ffmpeg_mt.exe .
set RC=%errorlevel%
popd
exit /b %RC%
