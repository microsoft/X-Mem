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

# Update repository information
RUN apt-get update

# Install runtime library to support huge/large pages.
RUN apt-get install -y libhugetlbfs0

# Install runtime library to support NUMA.
RUN apt-get install -y libnuma1

# Temporarily add X-Mem Linux binaries to /xmem_v2.4.1/ in the container
#RUN mkdir /xmem_v${xmem_version}
ADD releases/tarball/xmem_v${xmem_version}.tar.gz /

# Set up only what is needed for x86-64 AVX at /xmem
RUN mkdir /xmem
RUN mv /xmem_v${xmem_version}/xmem-linux-x64_avx /xmem/xmem
RUN mv /xmem_v${xmem_version}/ATTRIBUTION /xmem/
RUN mv /xmem_v${xmem_version}/CHANGELOG /xmem/
RUN mv /xmem_v${xmem_version}/LICENSE /xmem/
RUN mv /xmem_v${xmem_version}/README.md /xmem/
RUN rm -rf /xmem_v${xmem_version}

# Entrypoint
ENTRYPOINT ["/xmem/xmem"]

# Set default argument to X-Mem
CMD ["--help"]
