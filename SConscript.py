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

# Initialize build environment
env = Environment()

# Get arguments
arch = ARGUMENTS.get('arch', 'x64')
hostos = ARGUMENTS.get('hostos', 'linux')

# Architecture and OS-independent settings
env.Append(CPPPATH = ['src/include'])
env.Append(CPPPATH = ['src/include/ext/DelayInjectedLoadedLatencyBenchmark']) # Extension: Delay-injected loaded latency benchmark
env.Append(CPPPATH = ['src/include/ext/StreamBenchmark']) # Extension: Stream benchmark

# Customize build settings based on architecture and OS
if hostos == 'linux': # gcc
    env.Append(CPPFLAGS = ' -Wall -Wno-unused-but-set-variable -Wno-unused-variable -g -O3 -std=c++11 -fabi-version=6')
    env.Append(LIBS = ['pthread'])

    if arch == 'x64_avx': 
        env.Append(CPPFLAGS = ' -mavx')
        env.Append(LIBS = ['numa', 'hugetlbfs'])
    elif arch == 'x64':
        env.Append(LIBS = ['numa', 'hugetlbfs'])
    elif arch == 'mic': 
        env.Replace(PATH = os.environ['PATH'])
        env.Replace(CXX = 'icc') # Use Intel compiler
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
        env.Replace(CXX = 'arm-linux-gnueabihf-g++-4.8') # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard')
    elif arch == 'arm_neon': # Cross-compile for 32-bit ARM w/ NEON vector extensions on 64-bit x86-64 platform
        env.Replace(CXX = 'arm-linux-gnueabihf-g++-4.8') # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard -mfpu=vfpv3 -mfpu=neon')
    elif arch == 'arm64': # Cross-compile for 64-bit ARM on 64-bit x86-64 platform
        env.Replace(CXX = 'arm-linux-gnueabihf-g++-4.8') # Cross-compiler
        env.Append(CPPFLAGS = ' -mfloat-abi=hard -mfpu=vfpv3 -mfpu=neon -march=armv8-a')
    else:
        print 'Error: architecture ' + arch + ' not supported on ' + hostos + ', cannot build.'
        exit(1)

    # List all C++ source files
    sources = [
        Glob('src/*.cpp'), 
        Glob('src/ext/*/*.cpp'), # All extensions
    ]

elif hostos == 'win': # Windows OS
    # We use Visual C++ compiler
    env.Replace(MSVC_USE_SCRIPT = 'C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\bin\\amd64\\vcvars64.bat') # Path to the VC++ setup script. Assumes VS 2015
    env.Replace(AS = 'ml64') # 64-bit Visual Studio assembler
    env.Replace(MSVC_BATCH = 1) # Enable batch compilation mode (multithreaded)
    env['PDB'] = 'xmem.pdb' # Instruct Visual Studio to give us a debug/symbols file for this build of X-Mem

    # Set up compiler flags for VC++
    env.Append(ASFLAGS = ' /c /nologo /WX /Zi /W3 /errorReport:prompt')
    env.Append(CPPFLAGS = ' /c /Zi /nologo /W3 /wd4101 /WX- /MP /O2 /Oi /Ot /D WIN32 /D NDEBUG /D _CONSOLE /Gm- /EHsc /GS /fp:precise /Zc:wchar_t /Zc:forScope /Gd /TP /errorReport:queue /volatile:iso')
    env.Append(CPPFLAGS = ' /MD') # Dynamic link. Use /MT for static link

    if arch == 'x64_avx':
        env.Append(CPPFLAGS = ' /arch:AVX')
        # List all C++ source files
        sources = [
            Glob('src/*.cpp'), 
            Glob('src/ext/*/*.cpp'), # All extensions
            Glob('src/win/*.cpp'), # Windows-specific C++ code
            Glob('src/win/x86_64/*.asm'), # Have some hand-coded assembler files for Windows x86-64 AVX only
        ]

    elif arch == 'x64':
        # List all C++ source files
        sources = [
            Glob('src/*.cpp'), 
            Glob('src/ext/*/*.cpp'), # All extensions
            Glob('src/win/*.cpp'), # Windows-specific C++ code
        ]

    elif arch == 'x86':
        env.Append(CPPFLAGS = ' /arch:IA32')
        # List all C++ source files
        sources = [
            Glob('src/*.cpp'), 
            Glob('src/ext/*/*.cpp'), # All extensions
            Glob('src/win/*.cpp'), # Windows-specific C++ code
        ]

    elif arch == 'arm':
        env.Append(CPPFLAGS = ' /D _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1')
        env.Append(CPPFLAGS = ' /arch:VFPv4')
        env.Append(CPPFLAGS = ' /Gy')
        # List all C++ source files
        sources = [
            Glob('src/*.cpp'), 
            Glob('src/ext/*/*.cpp'), # All extensions
            Glob('src/win/win_common_third_party.cpp'),
        ]

    elif arch == 'arm_neon':
        env.Append(CPPFLAGS = ' /D _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1')
        env.Append(CPPFLAGS = ' /D __ARM_NEON')
        env.Append(CPPFLAGS = ' /arch:VFPv4')
        env.Append(CPPFLAGS = ' /Gy')
        # List all C++ source files
        sources = [
            Glob('src/*.cpp'), 
            Glob('src/ext/*/*.cpp'), # All extensions
            Glob('src/win/win_common_third_party.cpp'),
        ]

    else:
        print 'Error: architecture ' + arch + ' not supported on ' + hostos + ', cannot build.'
        exit(1)

# Do the build!
defaultBuild = env.Program(target = 'xmem', source = sources)
Default(defaultBuild)
