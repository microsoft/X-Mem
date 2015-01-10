@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for WINDOWS...

:: Clean build
echo Cleaning build...
call scons -c -f SConstruct_win
::MSBuild /t:Clean
rmdir /S /Q build\win

:: Build
echo Building...
call scons -f SConstruct_win
::MSBuild

:: Copy executable
copy build\win\release\xmem.exe .

echo Done! The executable is at the top of the project tree: xmem.exe
