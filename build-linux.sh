#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

echo Building X-Mem for GNU/Linux...

# Do a little trick to ensure build datetime are correct
# DO NOT remove this code -- otherwise X-Mem will fail to build.
build_datetime=`date`
echo "#ifndef __BUILD_DATETIME_H" > src/include/build_datetime.h
echo "#define __BUILD_DATETIME_H" >> src/include/build_datetime.h
echo "#define BUILD_DATETIME \"$build_datetime\"" >> src/include/build_datetime.h
echo "#endif" >> src/include/build_datetime.h

# Build
scons -f SConstruct_linux

# Check if build was successful
if [[ $? -eq 0 ]]; then
	# Copy executable
	cp build/linux/release/xmem ./xmem-linux
	echo Done! The executable is at the top of the project tree: xmem-linux
	exit 0
else
	echo X-Mem for GNU/Linux build FAILED.	
	exit 1
fi
