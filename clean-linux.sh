#!/bin/bash
#
# Author: Mark Gottscho
# <mgottscho@ucla.edu>

ARGC=$# # Get number of arguments, not including script name

if [[ "$ARGC" != 1 ]]; then # Bad number of arguments
	echo "Usage: clean-linux.sh <ARCH>"
	echo "<ARCH> can be x64_avx (RECOMMENDED), x64, x86, or ARM."
fi

# Clean build
echo Cleaning build of X-Mem for GNU/Linux on $ARCH\...

scons -c -f SConstruct_linux_$ARCH
rm -rf build/linux/$ARCH
rm xmem

echo Done!
