@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>


:: Clean build
echo Cleaning build of X-Mem for Windows...
call scons -c -f SConstruct_win
::MSBuild /t:Clean
rmdir /S /Q build\win
del xmem-win.exe

echo Done!
