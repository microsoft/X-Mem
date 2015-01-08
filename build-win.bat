@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for WINDOWS...

:: Clean build
echo Cleaning build...
MSBuild /t:Clean
rmdir /S /Q x64

:: Build
echo Building...
MSBuild

:: Copy executable
copy x64\Release\xmem.exe .
echo Done! The executable is at the top of the project tree: xmem.exe
