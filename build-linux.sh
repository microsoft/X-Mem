#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

ARGC=$# # Get number of arguments, not including script name

if [[ "$ARGC" != 1 ]]; then # Bad number of arguments
	echo "Usage: build-linux.sh <ARCH>"
	echo "<ARCH> can be x64_avx (RECOMMENDED), x64, x86, or ARM."
	exit 1
fi

ARCH=$1
echo Building X-Mem for GNU/Linux on $ARCH\...

# Do a little trick to ensure build datetime are correct
# DO NOT remove this code -- otherwise X-Mem will fail to build.
build_datetime=`date`
echo "#ifndef __BUILD_DATETIME_H" > src/include/build_datetime.h
echo "#define __BUILD_DATETIME_H" >> src/include/build_datetime.h
echo "#define BUILD_DATETIME \"$build_datetime\"" >> src/include/build_datetime.h
echo "#endif" >> src/include/build_datetime.h

# Build
scons -f SConstruct_linux_$ARCH

# Check if build was successful
if [[ $? -eq 0 ]]; then
	# Copy executable
	cp build/linux/$ARCH/release/xmem ./xmem
	cp build/linux/$ARCH/release/xmem bin/xmem-linux-$ARCH
	echo Done! The executable xmem is at the top of the project tree, and at bin/xmem-linux-$ARCH
	exit 0
else
	echo X-Mem for GNU/Linux on $ARCH build FAILED.
	exit 1
fi
