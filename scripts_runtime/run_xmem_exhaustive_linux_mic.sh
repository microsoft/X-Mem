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
#
# Run the entire suite of standard X-Mem microbenchmarks across entire memory hierarchy.
# Working set sizes are iterated in multiples of 2.
# This could take a long time depending on the system.


ARGC=$# # Get number of arguments excluding arg0 (script name itself)

if [[ "$ARGC" != 5 ]]; then # Bad number of arguments
	echo "Author: Mark Gottscho"
	echo "mgottscho@ucla.edu"
	echo ""
	echo "USAGE: run_xmem_exhaustive_mic.sh <XMEM_DIR> <ARCH> <MAX_NUMBER_THREADS> <MAX_WORKING_SET_SIZE_KB> <OUTPUT_DIR>"
	exit 1
fi

xmem_dir=$1
arch=$2
max_num_threads=$3
max_working_set_size_KB=$4
output_dir=$5

mkdir -p $output_dir

cat /proc/cpuinfo > $output_dir/cpuinfo.txt
uname -r > $output_dir/uname.txt
uname -m >> $output_dir/uname.txt
uname -n >> $output_dir/uname.txt

#for (( num_threads=1; num_threads<=$max_num_threads; num_threads++ )); do
num_threads=1
for (( working_set_size_KB=4; working_set_size_KB<=$max_working_set_size_KB; )); do
    echo $working_set_size_KB KB, $num_threads thread\(s\)...
    # Large pages are not used by default. If you want them, specify -L.
    $xmem_dir/bin/xmem-linux-$arch -t -l -c64 -c512 -R -W -s -S1 -S2 -S4 -S8 -S16 -v -w$working_set_size_KB -j$num_threads -f $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.csv > $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.txt 2>&1

    let "working_set_size_KB=$working_set_size_KB*2"
done

num_threads=4
for (( working_set_size_KB=4; working_set_size_KB<=$max_working_set_size_KB; )); do
    echo $working_set_size_KB KB, $num_threads thread\(s\)...
    # Large pages are not used by default. If you want them, specify -L.
    $xmem_dir/bin/xmem-linux-$arch -t -l -c64 -c512 -R -W -s -S1 -S2 -S4 -S8 -S16 -v -w$working_set_size_KB -j$num_threads -f $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.csv > $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.txt 2>&1

    let "working_set_size_KB=$working_set_size_KB*2"
done

num_threads=60
for (( working_set_size_KB=4; working_set_size_KB<=$max_working_set_size_KB; )); do
    echo $working_set_size_KB KB, $num_threads thread\(s\)...
    # Large pages are not used by default. If you want them, specify -L.
    $xmem_dir/bin/xmem-linux-$arch -t -l -c64 -c512 -R -W -s -S1 -S2 -S4 -S8 -S16 -v -w$working_set_size_KB -j$num_threads -f $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.csv > $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.txt 2>&1

    let "working_set_size_KB=$working_set_size_KB*2"
done

num_threads=120
for (( working_set_size_KB=4; working_set_size_KB<=$max_working_set_size_KB; )); do
    echo $working_set_size_KB KB, $num_threads thread\(s\)...
    # Large pages are not used by default. If you want them, specify -L.
    $xmem_dir/bin/xmem-linux-$arch -t -l -c64 -c512 -R -W -s -S1 -S2 -S4 -S8 -S16 -v -w$working_set_size_KB -j$num_threads -f $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.csv > $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.txt 2>&1

    let "working_set_size_KB=$working_set_size_KB*2"
done

num_threads=240
for (( working_set_size_KB=4; working_set_size_KB<=$max_working_set_size_KB; )); do
    echo $working_set_size_KB KB, $num_threads thread\(s\)...
    # Large pages are not used by default. If you want them, specify -L.
    $xmem_dir/bin/xmem-linux-$arch -t -l -c64 -c512 -R -W -s -S1 -S2 -S4 -S8 -S16 -v -w$working_set_size_KB -j$num_threads -f $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.csv > $output_dir/xmem_w$working_set_size_KB\_j$num_threads\.txt 2>&1

    let "working_set_size_KB=$working_set_size_KB*2"
done
#done

echo DONE
