#!/bin/bash
#
# The MIT License (MIT)
# 
# Copyright (c) 2014 Microsoft
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
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
    mkdir bin
    cp build/linux/$ARCH/release/xmem ./xmem
    cp build/linux/$ARCH/release/xmem bin/xmem-linux-$ARCH
    echo Done! The executable xmem is at the top of the project tree, and at bin/xmem-linux-$ARCH
    exit 0
else
    echo X-Mem for GNU/Linux on $ARCH build FAILED.
    exit 1
fi
