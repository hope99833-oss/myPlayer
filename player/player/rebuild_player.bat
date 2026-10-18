@echo off
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" D:\myPlayer\player\player\player.sln /t:player:Rebuild /p:Configuration=Debug /p:Platform=x64 /m /nologo /v:minimal
