@echo off

:: Author: Mark Gottscho
:: mgottscho@ucla.edu

set wget_dir=wget\bin\

set VCREDIST_X86=http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe
set VCREDIST_X64=http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe
set VCREDIST_ARM=http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_arm.exe
set SCONS=http://sourceforge.net/projects/scons/files/scons/2.3.4/scons-2.3.4-setup.exe
set PYTHON_X86=https://www.python.org/ftp/python/2.7.9/python-2.7.9.msi
set PYTHON_X64=https://www.python.org/ftp/python/2.7.9/python-2.7.9.amd64.msi
set DOXYGEN=http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.9.1-setup.exe
set MIKTEX=http://mirrors.ctan.org/systems/win32/miktex/setup/basic-miktex-2.9.5105.exe

%wget_dir%\wget %VCREDIST_X86%
%wget_dir%\wget %VCREDIST_X64%
%wget_dir%\wget %VCREDIST_ARM%
%wget_dir%\wget %SCONS%
%wget_dir%\wget %PYTHON_X86%
%wget_dir%\wget %PYTHON_X64%
%wget_dir%\wget %DOXYGEN%
%wget_dir%\wget %MIKTEX%
