#!/bin/bash

# Author: Mark Gottscho <mgottscho@ucla.edu>

# Clean build
echo Cleaning build of X-Mem for UNIX...

scons -c -f SConstruct_unix
rm -rf build/unix

echo Done!
