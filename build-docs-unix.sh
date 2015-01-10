#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem documentation via UNIX...

orig_dir=`pwd`
cp README.md src\README.md
doxygen Doxyfile
cd docs/latex
make
cd $orig_dir
