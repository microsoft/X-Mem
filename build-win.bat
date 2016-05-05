:: The MIT License (MIT)
:: 
:: Copyright (c) 2014 Microsoft
:: 
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
:: 
:: The above copyright notice and this permission notice shall be included in all
:: copies or substantial portions of the Software.
:: 
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
:: SOFTWARE.
::
:: Author: Mark Gottscho <mgottscho@ucla.edu>

@echo off

set ARCH=%1
set NUM_THREADS=%2
echo Building X-Mem for Windows on %ARCH% using %NUM_THREADS% threads...

:: Do a little trick to ensure build datetime are correct
:: DO NOT remove this code -- otherwise X-Mem will fail to build.
set build_datetime=%DATE% %TIME%
echo #ifndef __BUILD_DATETIME_H>src\include\build_datetime.h
echo #define __BUILD_DATETIME_H>>src\include\build_datetime.h
echo #define BUILD_DATETIME "%build_datetime%">>src\include\build_datetime.h
echo #endif>>src\include\build_datetime.h

:: Build
call scons arch=%ARCH% -j%NUM_THREADS%

:: Check if build was successful
if ERRORLEVEL 1 goto buildFailure

:: Copy executable
mkdir bin
copy build\windows\%ARCH%\release\xmem.exe bin\xmem-win-%ARCH%.exe
copy build\windows\%ARCH%\release\xmem.pdb bin\xmem-win-%ARCH%.pdb
echo Done! The executable xmem is at bin\xmem-win-%ARCH%
exit /B 0

:buildFailure
echo X-Mem for Windows build FAILED.
exit /B 1
