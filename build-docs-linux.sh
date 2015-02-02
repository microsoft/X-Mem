#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem documentation via LINUX...

orig_dir=`pwd`
cp README.md src/README.md
doxygen Doxyfile
cd doc/latex
make
cp refman.pdf $orig_dir/X-Mem_Developer_Manual.pdf
cd $orig_dir
