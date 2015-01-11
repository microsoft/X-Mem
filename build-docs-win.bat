@echo off

:: Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem documentation via WINDOWS...

set starting_dir=%CD%
copy README.md src\README.md
doxygen Doxyfile
cd doc\latex
call make
copy refman.pdf %starting_dir%\X-Mem_Developer_Manual.pdf
cd %starting_dir%
