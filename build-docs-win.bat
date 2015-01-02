@echo off
doxygen Doxyfile
set starting_dir=%CD%
cd doc\html
cd doc\latex
call make
copy refman.pdf %starting_dir%\xmem_developer_manual.pdf
cd %starting_dir%
