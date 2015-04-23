#!/bin/bash
#
# Author: Mark Gottscho
# mgottscho@ucla.edu

LIBHUGETLBFS=http://superb-dca2.dl.sourceforge.net/project/libhugetlbfs/libhugetlbfs/2.19/libhugetlbfs-2.19.tar.gz
PYTHON=https://www.python.org/ftp/python/2.7.9/Python-2.7.9.tgz
SCONS=http://sourceforge.net/projects/scons/files/scons/2.3.4/scons-2.3.4.tar.gz
GCC=http://www.netgull.com/gcc/releases/gcc-4.8.2/gcc-4.8.2.tar.gz
DOXYGEN=http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.9.1.linux.bin.tar.gz
LIBNUMA=ftp://oss.sgi.com/www/projects/libnuma/download/numactl-2.0.10.tar.gz
wget $LIBHUGETLBFS
wget $PYTHON
wget $SCONS
wget $GCC
wget $DOXYGEN
wget $LIBNUMA
