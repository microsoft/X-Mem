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



# Dockerfile to containerize the ability to run (but not build) X-Mem on Linux

# We prefer to base X-Mem on Ubuntu distribution
FROM ubuntu:14.04

# Set maintainer
MAINTAINER "Mark Gottscho, Email: mgottscho@ucla.edu"

# IMPORTANT: set X-Mem version information
ENV xmem_version 2.4.1
LABEL version=${xmem_version}
LABEL description="X-Mem: The E>X<tensible >Mem<ory Characterization Tool"

# Update repository information
RUN apt-get update

# Install vim text editor
RUN apt-get install -y vim

# Install doxygen for generating documentation on Linux
RUN apt-get install -y doxygen doxygen-latex

# Install development library to support huge/large pages.
RUN apt-get install -y libhugetlbfs-dev

# Install development library to support NUMA.
RUN apt-get install -y libnuma-dev

# Install Python 2.7
RUN apt-get install -y python2.7

# Install SCons
RUN apt-get install -y scons

# Install g++ compiler
RUN apt-get install -y g++

# Install g++ 4.7 and multilibs to build for 32-bit x86 on 64-bit x86-64 host. We cannot use same g++ version as above because the ARM cross-compiler stuff is incompatible while supporting 32-bit x86 for some reason. Ubuntu package managers will only allow one or the other.
RUN apt-get install -y g++-4.7 g++-4.7-multilib gcc-4.7 gcc-4.7-multilib

# Install gcc cross-compiler for ARM targets
RUN apt-get install -y g++-4.8-arm-linux-gnueabihf gcc-4.8-arm-linux-gnueabihf

# For some reason when cross-compiling on x86-64 host for 32-bit x86, /usr/include/asm/errno.h does not exist. There is, however, /usr/include/asm-generic/errno.h. The following band-aid fix works.
RUN cp /usr/include/asm-generic/errno.h /usr/include/asm/

# Make top-level directory in container for X-Mem source tree
RUN mkdir /X-Mem

# Add X-Mem source tree to container
ADD . /X-Mem/

# Set working directory for container
WORKDIR /X-Mem

# Entrypoint
ENTRYPOINT ["/bin/bash"]
