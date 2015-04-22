@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>

set ARCH=%1
echo Building X-Mem for Windows on %ARCH%...

:: Do a little trick to ensure build datetime are correct
:: DO NOT remove this code -- otherwise X-Mem will fail to build.
set build_datetime=%DATE% %TIME%
echo #ifndef __BUILD_DATETIME_H>src\include\build_datetime.h
echo #define __BUILD_DATETIME_H>>src\include\build_datetime.h
echo #define BUILD_DATETIME "%build_datetime%">>src\include\build_datetime.h
echo #endif>>src\include\build_datetime.h

:: Build
call scons -f SConstruct_win_%ARCH%

:: Check if build was successful
if ERRORLEVEL 1 goto buildFailure

:: Copy executable
copy build\win\%ARCH%\release\xmem.exe .\xmem.exe
echo Done! The executable is at the top of the project tree: xmem.exe
exit /B 0

:buildFailure
echo X-Mem for Windows build FAILED.
exit /B 1
