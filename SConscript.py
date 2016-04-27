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

import os
import platform

# Initialize build environment
env = Environment()

# Get arguments
arch = ARGUMENTS.get('arch', 'x64_avx')
hostos = platform.system()

# Architecture and OS-independent settings
env.Append(CPPPATH = ['src/include'])
env.Append(CPPPATH = ['src/include/ext/DelayInjectedLoadedLatencyBenchmark']) # Extension: Delay-injected loaded latency benchmark
env.Append(CPPPATH = ['src/include/ext/StreamBenchmark']) # Extension: Stream benchmark

# Customize build settings based on architecture and OS
if hostos == 'Linux': # gcc
    env.Append(CPPFLAGS = ' -Wall -Wno-unused-but-set-variable -Wno-unused-variable -g -O3 -std=c++11 -fabi-version=6')
    env.Append(LIBS = ['pthread'])

    if arch == 'x64_avx': 
        env.Append(CPPFLAGS = ' -mavx')
        env.Append(LIBS = ['numa', 'hugetlbfs'])
    elif arch == 'x64':
        env.Append(LIBS = ['numa', 'hugetlbfs'])
    if arch == 'mic': 
        env.PATH = os.environ['PATH']
        env.CXX = 'icc' # Use Intel compiler
        env.Append(CPPFLAGS = ' -mmic')
        env.Append(LINKFLAGS = ['-mmic'])
        # IGNORE THESE FOR NOW
        #env = Environment(ENV = {'PATH' : os.environ['PATH']}, CXX = "icc", LINK = "k1om-mpss-linux-ld")
        #env = Environment(ENV = {'PATH' : os.environ['PATH']}, CXX = "k1om-mpss-linux-g++")
        #env.Append(CPPPATH = ['/opt/mpss/3.6/sysroots/k1om-mpss-linux/usr/include'])
        #env.Append(CPPPATH = ['/opt/mpss/3.6/sysroots/k1om-mpss-linux/usr/include/c++'])
        #env['LIBPATH'] = '/opt/mpss/3.6/sysroots/k1om-mpss-linux/usr/lib64'
        #env.Append(LIBPATH = '/opt/mpss/3.6/sysroots/x86_64-mpsssdk-linux/lib')
        #env.Append(LIBPATH = '/opt/mpss/3.6/sysroots/k1om-mpss-linux/lib64')
    elif arch == 'x86': # Cross-compile for 32-bit x86 on 64-bit x86-64 platform
        env.Append(CPPFLAGS = ' -m32') # GNU binutils support x86 cross-compile using this flag
        env.Append(LIBPATH = ['/usr/lib32'])
        env.Append(LINKFLAGS = ['-m32'])
    elif arch == 'arm': # Cross-compile for 32-bit ARM on 64-bit x86-64 platform
        env.CXX = 'arm-linux-gnueabihf-g++-4.8' # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard')
    elif arch == 'arm_neon': # Cross-compile for 32-bit ARM w/ NEON vector extensions on 64-bit x86-64 platform
        env.CXX = 'arm-linux-gnueabihf-g++-4.8' # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard -mfpu=vfpv3 -mfpu=neon')
    elif arch == 'arm64': # Cross-compile for 64-bit ARM on 64-bit x86-64 platform
        env.CXX = 'arm-linux-gnueabihf-g++-4.8' # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard -mfpu=vfpv3 -mfpu=neon -march=armv8-a')


    # List all C++ source files
    sources = [
        Glob('src/*.cpp'), 
        Glob('src/ext/*/*.cpp'), # All extensions
    ]
elif hostos == 'Windows':
    print 'todo'

# Do the build!
defaultBuild = env.Program(target = 'xmem', source = sources)
Default(defaultBuild)
