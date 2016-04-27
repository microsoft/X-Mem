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
::
:: Run the entire suite of standard X-Mem microbenchmarks across entire memory hierarchy.
:: Working set sizes are iterated in multiples of 2.
:: This could take a long time depending on the system.

@echo off
::setlocal enableDelayedExpansion

set xmem_dir=%1
set arch=%2
set max_num_threads=%3
set max_working_set_size=%4
set output_dir=%5

mkdir %output_dir%

set /a working_set_size=4

:: Nested loops using goto. Sorry... batch for loops are lacking.
:loop1
set /a threads=1
:loop2
echo %working_set_size% KB, %threads% threads...

:: Large pages are not used by default. If you want them, specify -L.
%xmem_dir%\bin\xmem-win-%arch% -a -v -w%working_set_size% -j%threads% -f %output_dir%\xmem_w%working_set_size%_j%threads%.csv > %output_dir%\xmem_w%working_set_size%_j%threads%.txt 2>&1

if %threads%==%max_num_threads% goto exitLoop2
set /a threads=threads+1
goto loop2
:exitLoop2
if %working_set_size%==%max_working_set_size% goto exitLoop1
set /a working_set_size=%working_set_size%*2
goto loop1
:exitLoop1
echo DONE
