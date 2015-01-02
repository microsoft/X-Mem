#!/bin/bash

doxygen Doxyfile
orig_dir=`pwd`
cd docs/latex
make
cd $orig_dir
