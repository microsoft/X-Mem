#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for UNIX...

# Clean build
echo Cleaning build...
make clean

# Build
echo Building...
make

echo Done! The executable is at the top of the project tree: xmem
