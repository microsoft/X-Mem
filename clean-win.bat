@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>


:: Clean build
echo Cleaning build of X-Mem for Windows x86-64...
call scons -c -f SConstruct_win_x64
::MSBuild /t:Clean
rmdir /S /Q build\win
del xmem-win.exe

echo Done!
