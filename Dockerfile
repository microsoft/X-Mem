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

# Install development library to support huge/large pages.
RUN apt-get install -y libhugetlbfs-dev

# Install development library to support NUMA.
RUN apt-get install -y libnuma-dev

# Install Python 2.7
RUN apt-get install -y python2.7

# Install SCons
RUN apt-get install -y scons

# Install gcc cross-compiler for ARM targets
RUN apt-get install -y g++-4.8-arm-linux-gnueabihf

# Install doxygen for generating documentation on Linux
RUN apt-get install -y doxygen doxygen-latex

# Install g++ compiler
RUN apt-get install -y g++

# Install vim text editor
RUN apt-get install -y vim

# Make top-level directory in container for X-Mem source tree
RUN mkdir /X-Mem

# Add X-Mem source tree to container
ADD . /X-Mem/

# Set working directory for container
WORKDIR /X-Mem

# Entrypoint
ENTRYPOINT ["/bin/bash"]
