#!/bin/bash

orig_dir=`pwd`
cp README.md src\README.md
doxygen Doxyfile
# cd docs/latex
# make
cd $orig_dir
