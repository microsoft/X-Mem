README
------------------------------------------------------------------------------------------------------------

X-Mem: Extensible Memory Benchmarking Tool v1.3.2
------------------------------------------------------------------------------------------------------------

The flexible open-source research tool for characterizing memory hierarchy throughput, latency, and power. 

Originally authored by Mark Gottscho (Email: <mgottscho@ucla.edu>) as a Summer 2014 intern at Microsoft Research, Redmond, WA.

This project is under active development. Stay tuned for more updates.

PROJECT REVISION DATE: February 8, 2015.

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

Flexibility: Easy reconfiguration for different combinations of tests
	- Working sets in increments of 4KB, allowing cache up to main memory-level benchmarking
	- NUMA support
	- Multi-threading support
	- Large page support

Extensibility: modularity via C++ object-oriented principles
	- Supports rapid addition of new benchmark kernel routines
	- Example: stream triad algorithm, impact of false sharing, etc. are possible with minor changes

Cross-platform: Currently implemented for Windows and GNU/Linux on x86-64 CPUs
	- Designed to allow straightforward porting to other operating systems and ISAs
	- GNU/Linux on ARM port planned

Memory throughput:
	- Accurate measurement of sustained memory throughput to all levels of cache and memory
	- Regular access patterns: forward & reverse sequential as well as strides of 2, 4, 8, and 16 words
	- Random access patterns
	- Read and write
	- 32, 64, 128, 256-bit width memory instructions

Memory latency: 
	- Accurate measurement of round-trip memory latency to all levels of cache and memory
	- Loaded and unloaded latency via use of multithreaded load generation

Memory power:
	- Currently collecting DRAM power via custom driver exposed in Windows performance counter API
	- Support custom power instrumentation through a simple interface that end-users can implement

Documentation:
	- Extensive Doxygen source code comments, PDF manual, HTML

For feature requests, please refer to the contact information at the end of this README.

------------------------------------------------------------------------------------------------------------
RUNTIME PREREQUISITES
------------------------------------------------------------------------------------------------------------

There are a few runtime prerequisites in order for the software to run correctly.

HARDWARE:

- Intel x86-64 CPU supporting AVX extensions (Sandy Bridge or later).

WINDOWS:

- Microsoft Windows 64-bit, 8.0 or later, Server 2012 or later.
- Microsoft Visual C++ 2013 Redistributables (64-bit)
- Potentially, Administrator privileges, in order to:
	- use large pages, if the --large_pages option is selected (see USAGE, below)
	- use the PowerReader interface, depending on end-user implementation
	- elevate thread priority and pin threads to logical CPUs for improved performance and benchmarking consistency

GNU/LINUX:

- GNU utilities with support for C++11. Tested with gcc 4.8.2 on Ubuntu 14.04 LTS.
- libhugetlbfs. You can obtain it at <http://libhugetlbfs.sourceforge.net>. On Ubuntu systems, you can install using "sudo apt-get install libhugetlbfs0".
- Potentially, administrator privileges, if you plan to use the --large_pages option.
	- During runtime, if the --large_pages option is selected, you may need to first manually ensure that large pages are available from the OS. This can be done by running "hugeadm --pool-list". It is recommended to set minimum pool to 1GB (in order to measure DRAM effectively). If needed, this can be done by running "hugeadm --pool-pages-min 2MB:512". Alternatively, run the linux_setup_runtime_hugetlbfs.sh script that is provided with X-Mem. 

------------------------------------------------------------------------------------------------------------
INSTALLATION
------------------------------------------------------------------------------------------------------------

The only file that is needed to run on Windows is xmem-win.exe, and xmem-linux on GNU/Linux. It has no other dependencies aside from the system prerequisites listed above.

------------------------------------------------------------------------------------------------------------
USAGE
------------------------------------------------------------------------------------------------------------

USAGE: xmem [options]

Options:
    -c, --chunk_size            A chunk size to use for throughput benchmarks,
                                specified in bits. Allowed values: 32, 64, 128,
                                and 256. If no chunk sizes specified, use 64-bit
                                chunks by default. NOTE: Some chunk sizes may
                                not be supported on your hardware.
    -f, --output_file           Output filename to use. If not specified, no
                                output file generated.
    -h, --help                  Print usage and exit.
    -i, --base_test_index       Numerical index of the first benchmark, for
                                tracking unique test IDs.
    -j, --num_worker_threads    Number of worker threads to use in relevant
                                benchmarks. This may not exceed the number of
                                logical CPUs in the system. For throughput
                                benchmarks, this is the number of independent
                                load-generating threads. For latency benchmarks,
                                this is the number of independent
                                load-generating threads plus one latency
                                measurement thread.
    -l, --latency               Measure memory latency
    -n, --iterations            Iterations per benchmark test
    -r, --random_access         Use a random access pattern on throughput
                                benchmarks. WARNING: not yet implemented,
                                results are not correct.
    -s, --sequential_access     Use a sequential access pattern on throughput
                                benchmarks
    -t, --throughput            Measure memory throughput
    -u, --force_uma             Test only CPU/memory NUMA node 0 instead of all
                                combinations.
    -v, --verbose               Verbose mode, increase detail in X-Mem console
                                reporting.
    -w, --working_set_size      Working set size per worker thread in KB. This
                                must be a multiple of 4KB.
    -L, --large_pages           Use large pages if possible. This may enable
                                better memory performance, particularly for
                                random-access patterns, but may not be supported
                                on your system.
    -R, --reads                 Use memory reads in throughput benchmarks.
    -W, --writes                Use memory writes in throughput benchmarks.
    -S, --stride_size           A stride size to use for sequential throughput
                                benchmarks, specified in powers-of-two multiples
                                of the chunk size(s). Allowed values: 1, -1, 2,
                                -2, 4, -4, 8, -8, 16, -16. Positive indicates
                                the forward direction (increasing addresses),
                                while negative indicates the reverse direction.

If a given option is not specified, X-Mem defaults will be used where
appropriate.

Examples:
    xmem --help
    xmem -h
    xmem -l --verbose -n5 --chunk_size=32 -s
    xmem -t --latency -w524288 -f results.csv -c32 -c256 -i 101 -u -j2

------------------------------------------------------------------------------------------------------------
BUILDING FROM SOURCE
------------------------------------------------------------------------------------------------------------

Before building the source, enable and disable the relevant compile-time options in src/include/common.h, under the section "User-configurable compilation configuration". Please read the comments by each #define statement to understand the context of each option.

After you have set the desired compile-time options, build the source. On Windows, running build-win.bat should suffice. On GNU/Linux, run build-linux.sh.

If you customize your build, make sure you use the "Release" mode for your OS. Do not include debug capabilities as it can dramatically affect performance of the benchmarks, leading to pessimistic results.

------------------------------------------------------------------------------------------------------------
BUILD PREREQUISITES
------------------------------------------------------------------------------------------------------------

There are a few software build prerequisites, depending on your platform.

WINDOWS:

- Any version of Visual Studio 2013 64-bit (also known as version 12.0).
- Python 2.7. You can obtain it at <http://www.python.org>.
- SCons build system. You can obtain it at <http://www.scons.org>. Build tested with SCons 2.3.4.

GNU/LINUX:

- gcc with support for the C++11 standard. Tested with gcc version 4.8.2 on Ubuntu 14.04 LTS for x86-64.
- Python 2.7. You can obtain it at <http://www.python.org>. On Ubuntu systems, you can install using "sudo apt-get install python2.7". You may need some other Python 2.7 packages as well.
- SCons build system. You can obtain it at <http://www.scons.org>. On Ubuntu systems, you can install using "sudo apt-get install scons". Build tested with SCons 2.3.4.
- Kernel support for large (huge) pages. This support can be verified on your Linux installation by running "grep hugetlbfs /proc/filesystems". If you do not have huge page support in your kernel, you can build a kernel with the appropriate options switched on: "CONFIG_HUGETLB_PAGE" and "CONFIG_HUGETLBFS".
- libhugetlbfs. This is used for allocating large (huge) pages if the --large_pages runtime option is selected. You can obtain it at <http://libhugetlbfs.sourceforge.net>. On Ubuntu systems, you can install using "sudo apt-get install libhugetlbfs-dev".

------------------------------------------------------------------------------------------------------------
DOCUMENTATION BUILD PREREQUISITES
------------------------------------------------------------------------------------------------------------

The following tools are only needed for automatically regenerating source code documentation with HTML and PDF.

WINDOWS:

- doxygen tool. You can obtain it at <http://www.stack.nl/~dimitri/doxygen>.
- LaTeX distribution. You can get a Windows distribution at <http://www.miktex.org>.
- make for Windows. You can obtain it at <http://gnuwin32.sourceforge.net/packages/make.htm>. You will have to manually add it to your Windows path.

GNU/LINUX:

- doxygen tool. You can obtain it at <http://www.stack.nl/~dimitri/doxygen>. On Ubuntu systems, you can install with "sudo apt-get install doxygen".
- LaTeX distribution. On Ubuntu systems, LaTeX distributed with doxygen should actually be sufficient. You can install with "sudo apt-get install doxygen-latex".
- make. This should be included on any GNU/Linux system.

------------------------------------------------------------------------------------------------------------
SOURCE CODE DOCUMENTATION
------------------------------------------------------------------------------------------------------------

The tool comes with built-in Doxygen comments in the source code, which can be used to generate both HTML and LaTeX --> PDF documentation. Documentation is maintained under the doc/ subdirectory. To build documentation after modifying the source, run build-docs-win.bat on Windows, or build-docs-linux.sh on GNU/Linux systems. Note that Doxygen and a LaTeX distribution must be installed on the system.

------------------------------------------------------------------------------------------------------------
VERSION CONTROL
------------------------------------------------------------------------------------------------------------

This project is under version control using git. Its master repository is hosted at <https://github.com/Microsoft/X-Mem.git>. There is also another fork at <https://github.com/nanocad-lab/X-Mem.git>, which generally mirrors Microsoft's repository.

------------------------------------------------------------------------------------------------------------
CONTACT, FEEDBACK, AND BUG REPORTS
------------------------------------------------------------------------------------------------------------

For questions, comments, criticism, bug reports, and other feedback for this software, please contact Mark Gottscho via email at <mgottscho@ucla.edu> or via web at <http://www.seas.ucla.edu/~gottscho>.

For inquiries about this work while conducted at Microsoft, please contact Dr. Mohammed Shoaib at <mohammed.shoaib@microsoft.com> or Dr. Sriram Govindan at <srgovin@microsoft.com>.

------------------------------------------------------------------------------------------------------------
ACKNOWLEDGMENT
------------------------------------------------------------------------------------------------------------

Mark Gottscho would like to thank Dr. Mohammed Shoaib of Microsoft Research and Dr. Sriram Govindan of Microsoft for their mentorship in the creation of this software. Further thanks to Dr. Bikash Sharma, Mark Santaniello, Mike Andrewartha, and Laura Caulfield of Microsoft for their contributions, feedback, and assistance. Thank you as well to Dr. Jie Liu of Microsoft Research, Dr. Badriddine Khessib and Dr. Kushagra Vaid of Microsoft, and Prof. Puneet Gupta of UCLA for giving me the opportunity to create this work. Finally, Mark would like to thank Dr. Fedor Pikus of Mentor Graphics for teaching him some useful HPC programming techniques.
