#!/bin/bash
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

min_pool_size=512 # in multiples of huge page size
echo "Setting up hugetlbfs on GNU/Linux, so that we can allocate large pages using X-Mem."
echo "Current huge pages status:"
hugeadm --pool-list
echo "Setting minimum pool size to $min_pool_size large pages."
sudo hugeadm --pool-pages-min 2MB:$min_pool_size
echo "New huge pages status:"
hugeadm --pool-list
