#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for UNIX...

# Clean build
echo Cleaning build...
scons -c -f SConstruct_unix
rmdir -rf build/unix

# Build
echo Building...
scons -f SConstruct_unix

# Copy executable
cp build/unix/release/xmem .
echo Done! The executable is at the top of the project tree: xmem
