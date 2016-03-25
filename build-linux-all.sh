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
    echo "Usage: build-linux-all.sh <NUM_THREADS>"
    exit 1
fi

NUM_THREADS=$1
echo Building X-Mem for GNU/Linux on all supported architectures in parallel using $NUM_THREADS threads for each job...

./build-linux.sh x64 $NUM_THREADS &
sleep 1
./build-linux.sh x64_avx $NUM_THREADS &
sleep 1
./build-linux.sh x86 $NUM_THREADS &
sleep 1
./build-linux.sh arm $NUM_THREADS &
sleep 1
./build-linux.sh arm_neon $NUM_THREADS &
sleep 1
./build-linux.sh arm64 $NUM_THREADS &
