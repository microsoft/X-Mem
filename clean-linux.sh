#!/bin/bash

# Author: Mark Gottscho <mgottscho@ucla.edu>

# Clean build
echo Cleaning build of X-Mem for GNU/Linux...

scons -c -f SConstruct_linux
rm -rf build/linux
rm xmem-linux

echo Done!
