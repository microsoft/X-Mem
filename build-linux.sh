#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for GNU/Linux...

# Build
scons -f SConstruct_linux

# Copy executable
cp build/linux/release/xmem ./xmem-linux
echo Done! The executable is at the top of the project tree: xmem-linux
