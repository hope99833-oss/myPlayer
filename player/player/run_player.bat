@echo off
set QT_QPA_PLATFORM_PLUGIN_PATH=%~dp0platforms
cd /d %~dp0
player.exe
