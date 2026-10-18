@echo off
set INCLUDE=D:\myPlayer\player\player\include;D:\myPlayer\player\player\include\sdl2;D:\myPlayer\player\player\fftools;E:\Qt\5.15.2\msvc2019_64\include;E:\Qt\5.15.2\msvc2019_64\include\QtWidgets;E:\Qt\5.15.2\msvc2019_64\include\QtGui;E:\Qt\5.15.2\msvc2019_64\include\QtCore;C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
cd /d D:\myPlayer\player\player
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\cl.exe" /c /nologo /std:c++17 /EHsc player.cpp
