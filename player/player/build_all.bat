@echo off
set INCLUDE=D:\myPlayer\player\player\include;D:\myPlayer\player\player\include\sdl2;D:\myPlayer\player\player\fftools;E:\Qt\5.15.2\msvc2019_64\include;E:\Qt\5.15.2\msvc2019_64\include\QtWidgets;E:\Qt\5.15.2\msvc2019_64\include\QtGui;E:\Qt\5.15.2\msvc2019_64\include\QtCore;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
set LIB=D:\myPlayer\player\player\lib;D:\myPlayer\player\player\bin;E:\Qt\5.15.2\msvc2019_64\lib;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64
cd /d D:\myPlayer\player\player

REM Generate moc file
E:\Qt\5.15.2\msvc2019_64\bin\moc.exe player.h -o moc_player.cpp

REM Compile all source files
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc /utf-8 main.cpp
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc /utf-8 player.cpp
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc /utf-8 moc_player.cpp

REM Generate resource file
E:\Qt\5.15.2\msvc2019_64\bin\rcc.exe player.qrc -o qrc_player.cpp
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc qrc_player.cpp

REM Link
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\link.exe" /nologo /out:player.exe player.obj main.obj moc_player.obj qrc_player.obj Qt5Widgets.lib Qt5Gui.lib Qt5Core.lib ffplay.lib ffmpeg.lib SDL2.lib
