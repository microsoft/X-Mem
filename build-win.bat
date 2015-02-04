@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for Windows...

:: Do a little trick to ensure build datetime are correct
:: DO NOT remove this code -- otherwise X-Mem will fail to build.
set build_datetime=%DATE% %TIME%
echo #ifndef __BUILD_DATETIME_H>src\include\build_datetime.h
echo #define __BUILD_DATETIME_H>>src\include\build_datetime.h
echo #define BUILD_DATETIME "%build_datetime%">>src\include\build_datetime.h
echo #endif>>src\include\build_datetime.h

:: Build
call scons -f SConstruct_win
::MSBuild

:: Copy executable
copy build\win\release\xmem.exe .\xmem-win.exe

echo Done! The executable is at the top of the project tree: xmem-win.exe
