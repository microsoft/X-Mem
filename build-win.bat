@echo off

:: Clean build
echo Cleaning build...
MSBuild /t:Clean
rmdir /S /Q x64
echo Building...
MSBuild
copy x64\Release\xmem.exe .
echo Done! The executable is at the top of the project tree: xmem.exe
