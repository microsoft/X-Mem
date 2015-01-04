@echo off
set starting_dir=%CD%
copy README.md src\README.md
doxygen Doxyfile
:: cd doc\html
:: cd doc\latex
:: call make
:: copy refman.pdf %starting_dir%\X-Mem_Developer_Manual.pdf
cd %starting_dir%
