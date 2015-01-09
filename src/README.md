README
------------------------------------------------------------------------------------------------------------

X-Mem: Extensible Memory Benchmarking Tool v1.02
------------------------------------------------------------------------------------------------------------

The flexible open-source research tool for characterizing memory hierarchy throughput, latency, and power. 

Originally authored by Mark Gottscho (Email: <mgottscho@ucla.edu>) as a Summer 2014 intern at Microsoft Research, Redmond, WA.

------------------------------------------------------------------------------------------------------------
LICENSE
------------------------------------------------------------------------------------------------------------

The MIT License (MIT)

Copyright (c) 2014 Microsoft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

------------------------------------------------------------------------------------------------------------
FEATURES
------------------------------------------------------------------------------------------------------------

This tool is provided as open source with the hope of being useful to the broader research and development community. Here are some of X-Mem's features.

Flexibility: Easy reconfiguration for different sets of tests
	- Cache levels
	- NUMA
	- Multi-threading
	- Forward and reverse strides
	- Sequential and random access
	- Read and write
	- 32, 64, 128, 256-bit width memory instructions

Extensibility: C++ object-oriented principles
	- Supports rapid addition of new benchmark kernel routines by others
	- Example: stream triad algorithm, impact of false sharing, etc. are possible with minor additions

Cross-platform: Currently implemented for Windows on x86-64 CPUs with AVX extensions
	- Designed to allow straightforward porting to other operating systems and ISAs
	- Planning to implement Unix port

Memory throughput:
	- Accurate measurement of sustained memory throughput to all levels of cache
	- Burst mode benchmark kernels possible (extensible!)

Memory latency: 
	- Accurate measurement of round-trip unloaded memory latency to all levels of cache
	- Loaded latency measurements planned

Memory power:
	- Currently collecting DRAM power via custom driver exposed in Windows performance counter API
	- Supports custom power instrumentation without much modification
	- Thorough Documentation: extensive Doxygen source comments, HTML, PDF docs

For feature requests, please refer to the contact information at the end of this README.

------------------------------------------------------------------------------------------------------------
RUNTIME PREREQUISITES
------------------------------------------------------------------------------------------------------------

There are certain runtime prerequisites in order for the software to run correctly: 

- Intel x86-64 CPU. RECOMMENDED: Recent CPU with SSE2 and AVX extended instruction sets for improved throughput benchmarking capabilities.
- Microsoft Windows 64-bit. Tested with Windows Server 2012 R2 and Windows 8.1.
- Microsoft Visual C++ 2013 Redistributables (64-bit)

These prerequisites may be eased with future versions of the software.

------------------------------------------------------------------------------------------------------------
INSTALLATION
------------------------------------------------------------------------------------------------------------

The only file that is needed to run is xmem.exe. It has no other dependencies aside from the system prerequisites listed above.

------------------------------------------------------------------------------------------------------------
USAGE
------------------------------------------------------------------------------------------------------------

NOTE: On Windows, make sure you run X-Mem with Administrator privileges. This is needed in order to:
	- Allocate "large pages" for improved performance as well as query 
	- Read performance counter data from the OS for reporting power (when applicable)
	- Elevate thread priority and pin threads to CPUs for improved performance and benchmarking consistency

xmem [options]

Options:

    -h, --help                Print usage and exit.

    -l, --latency             Measure memory latency

    -t, --throughput          Measure memory throughput

    -w, --working_set_size    Working set size in KB. This must be a multiple of
                              4KB.

    -n, --iterations          Iterations per benchmark test

    -i, --base_test_index     Numerical index of the first benchmark, for
                              tracking unique test IDs.

    -f, --output_file         Output filename to use. If not specified, no
                              output file generated.

Examples:

    xmem --help

    xmem -h

    xmem -t

    xmem -t --latency -n10 -w524288 -f results.csv -i 101

------------------------------------------------------------------------------------------------------------
BUILDING FROM SOURCE
------------------------------------------------------------------------------------------------------------

Before building the source, enable and disable the relevant compile-time options in src/common/common.h, under the section "User-configurable compilation configuration". Please read the comments by each #define statement to understand the context of each option.

After you have set the desired compile-time options, build the source. On Windows, running build-win.bat should suffice. On Unix, run build-unix.sh. The source can also be built using your favored IDE of choice. Visual C++ 2013 project and solution files have been provided to allow out-of-the-box builds with Visual Studio 2013 on Windows platforms.

Make sure you build the code in "Release" mode. Do not include debug capabilities as it can dramatically affect performance of the benchmarks, leading to pessimistic results.

------------------------------------------------------------------------------------------------------------
SOURCE CODE DOCUMENTATION
------------------------------------------------------------------------------------------------------------

The tool comes with built-in Doxygen comments in the source code, which can be used to generate both HTML and LaTeX --> PDF documentation. Documentation is maintained under the doc/ subdirectory. To build documentation after modifying the source, run build-docs-win.bat on Windows, or build-docs-unix.sh on Unix systems. Note that Doxygen and a LaTeX distribution must be installed on the system.

------------------------------------------------------------------------------------------------------------
VERSION CONTROL
------------------------------------------------------------------------------------------------------------

This project is under version control using git. Its master repository is hosted at <https://github.com/Microsoft/X-Mem.git>.

------------------------------------------------------------------------------------------------------------
CONTACT AND FEEDBACK
------------------------------------------------------------------------------------------------------------

For questions, comments, criticism, bug reports, and other feedback for this software, please contact Mark Gottscho via email at <mgottscho@ucla.edu> or via web at <http://www.seas.ucla.edu/~gottscho>.

For inquiries about this work while conducted at Microsoft, please contact Dr. Mohammed Shoaib at <mohammed.shoaib@microsoft.com> or Dr. Sriram Govindan at <srgovin@microsoft.com>.

------------------------------------------------------------------------------------------------------------
ACKNOWLEDGMENT
------------------------------------------------------------------------------------------------------------

The author would like to thank Dr. Mohammed Shoaib of Microsoft Research and Dr. Sriram Govindan of Microsoft for their mentorship in the creation of this software. Further thanks to Dr. Bikash Sharma, Mark Santaniello, Mike Andrewartha, and Laura Caulfield of Microsoft for their contributions, feedback, and assistance. Finally, thank you to Dr. Jie Liu of Microsoft Research, Dr. Badriddine Khessib and Dr. Kushagra Vaid of Microsoft, and Prof. Puneet Gupta of UCLA for giving me the opportunity to create this work.
