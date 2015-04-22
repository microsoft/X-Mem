README
------------------------------------------------------------------------------------------------------------

X-Mem: Extensible Memory Benchmarking Tool v2.1.12
------------------------------------------------------------------------------------------------------------

The flexible open-source research tool for characterizing memory hierarchy throughput, latency, and power. 

Originally authored by Mark Gottscho (Email: <mgottscho@ucla.edu>) as a Summer 2014 intern at Microsoft Research, Redmond, WA.

This project is under active development. Stay tuned for more updates.

PROJECT REVISION DATE: April 22, 2015.

------------------------------------------------------------------------------------------------------------
LICENSE
------------------------------------------------------------------------------------------------------------

The MIT License (MIT)

Copyright (c) 2015 Microsoft

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

Cross-platform: Currently implemented for Windows and GNU/Linux on x86-64, x86-64 with AVX extensions CPUs
	- Designed to allow straightforward porting to other operating systems and ISAs
	- 32-bit x86 port under development
	- ARM port under development

Memory throughput:
	- Accurate measurement of sustained memory throughput to all levels of cache and memory
	- Regular access patterns: forward & reverse sequential as well as strides of 2, 4, 8, and 16 words
	- Random access patterns
	- Read and write
	- 32, 64, 128, 256-bit width memory instructions where applicable on each architecture

Memory latency: 
	- Accurate measurement of round-trip memory latency to all levels of cache and memory
	- Loaded and unloaded latency via use of multithreaded load generation

Memory power:
	- Support custom power instrumentation through a simple interface that end-users can implement
	- Can collect DRAM power via custom driver exposed in Windows performance counter API

Documentation:
	- Extensive Doxygen source code comments, PDF manual, HTML


INCLUDED EXTENSIONS (under src/include/ext and src/ext directories):
	- Loaded latency benchmark variant with load delays inserted as nop instructions between memory instructions.
	  This is done for 64 and 256-bit chunks on x86-64 with AVX extensions, forward sequential read load threads only at the moment.

For feature requests, please refer to the contact information at the end of this README.

------------------------------------------------------------------------------------------------------------
RUNTIME PREREQUISITES
------------------------------------------------------------------------------------------------------------

There are a few runtime prerequisites in order for the software to run correctly.

HARDWARE:

- Intel x86-64 CPU with optional support for AVX extensions. AMD CPUs should also work although this has not been tested.
- COMING SOON: Intel 32-bit x86 CPU
- COMING SOON: ARMv7 CPUs

WINDOWS:

- Microsoft Windows 8.0 64-bit or later, Server 2012 or later.
- Microsoft Visual C++ 2013 Redistributables (64-bit)
- COMING SOON: Windows 32-bit
- You MAY need Administrator privileges, in order to:
	- Use large pages, if the --large_pages option is selected (see USAGE, below)
	- The first time you use --large_pages on a given Windows machine, you may need to ensure that your Windows user account has the necessary rights to allow lockable memory pages. To do this on Windows 8, run gpedit.msc --> Local Computer Policy --> Computer Configuration --> Windows Settings --> Security Settings --> Local Policies --> User Rights Assignment --> Add your username to "Lock pages in memory". Then log out and then log back in.
	- Use the PowerReader interface, depending on end-user implementation
	- Elevate thread priority and pin threads to logical CPUs for improved performance and benchmarking consistency

GNU/LINUX:

- GNU utilities with support for C++11. Tested with gcc 4.8.2 on Ubuntu 14.04 LTS for x86-64 CPU.
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
    -a, --all                   Run all possible benchmark modes and settings
                                supported by X-Mem. This will override any other
                                relevant user inputs. Note that X-Mem may run
                                for a long time.
    -c, --chunk_size            A chunk size in bits to use for load
                                traffic-generating threads used in throughput
                                and loaded latency benchmarks. A chunk is the
                                size of each memory access in a benchmark.
                                Allowed values: 32 64 128 and 256 (platform
                                dependent). Note that some chunk sizes may not
                                be supported on all hardware. 32-bit chunks are
                                not compatible with random-access patterns on
                                64-bit machines; these combinations of settings
                                will be skipped if they occur. DEFAULT: 64 on
                                64-bit systems, 32 on 32-bit systems.
    -e, --extension             Run an X-Mem extension defined by the user at
                                build time. The integer argument specifies a
                                single unique extension. This option may be
                                included multiple times. Note that the extension
                                behavior may or may not depend on the other
                                X-Mem options as its semantics are defined by
                                the extension author.
    -f, --output_file           Generate an output file in CSV format using the
                                given filename.
    -h, --help                  Print X-Mem usage and exit.
    -i, --base_test_index       Base index for the first benchmark to run. This
                                option is provided for user convenience in
                                enumerating benchmark tests across several
                                subsequent runs of X-Mem. DEFAULT: 1
    -j, --num_worker_threads    Number of worker threads to use in benchmarks.
                                This may not exceed the number of logical CPUs
                                in the system. For throughput benchmarks, this
                                is the number of independent load-generating
                                threads. For latency benchmarks, this is the
                                number of independent load-generating threads
                                plus one latency measurement thread. In latency
                                benchmarks, 1 worker thread indicates no loading
                                is applied. DEFAULT: 1
    -l, --latency               Unloaded or loaded latency benchmarking mode. If
                                1 thread is used, unloaded latency is measured
                                using 64-bit random reads. Otherwise, 1 thread
                                is always dedicated to the 64-bit random read
                                latency measurement, and remaining threads are
                                used for load traffic generation using access
                                patterns, chunk sizes, etc. specified by other
                                arguments. See the throughput option for more
                                information on load traffic generation.
    -n, --iterations            Iterations per benchmark. Multiple independent
                                iterations may be performed on each benchmark
                                setting to ensure consistent results. DEFAULT: 1
    -r, --random_access         Use a random access pattern for load
                                traffic-generating threads used in throughput
                                and loaded latency benchmarks.
    -s, --sequential_access     Use a sequential and/or strided access pattern
                                for load traffic generating-threads used in
                                throughput and loaded latency benchmarks.
    -t, --throughput            Throughput benchmarking mode. Aggregate
                                throughput is measured across all worker
                                threads. Each load traffic-generating worker in
                                a particular benchmark runs an identical kernel.
                                Multiple distinct benchmarks may be run
                                depending on the specified benchmark settings
                                (e.g., aggregated 64-bit and 256-bit sequential
                                read throughput using strides of 1 and -8
                                chunks).
    -u, --ignore_numa           Force uniform memory access (UMA) mode. This
                                only has an effect in non-uniform memory access
                                (NUMA) systems. Limits benchmarking to CPU and
                                memory NUMA node 0 instead of all intra-node and
                                inter-node combinations. This mode can be useful
                                in situations where the user is not interested
                                in cross-node effects or node asymmetry. This
                                option may also be required if large pages are
                                desired on GNU/Linux systems due to lack of NUMA
                                support in current versions of hugetlbfs. See
                                the large_pages option.
    -v, --verbose               Verbose mode increases the level of detail in
                                X-Mem console reporting.
    -w, --working_set_size      Working set size per worker thread in KB. This
                                must be a multiple of 4KB. In all benchmarks,
                                each worker thread works on its own "private"
                                region of memory. For example, 4-thread
                                throughput benchmarking with a working set size
                                of 4 KB might result in measuring the aggregate
                                throughput of four L1 caches corresponding to
                                four physical cores, with no data sharing
                                between threads. Similarly, an 8-thread loaded
                                latency benchmark with a working set size of 64
                                MB would use 512 MB of memory in total for
                                benchmarking, with no data sharing between
                                threads. This would result in performance
                                measurement of the shared DRAM physical
                                interface, the shared L3 cache, etc.
    -L, --large_pages           Use large pages. This might enable better memory
                                performance by reducing the
                                translation-lookaside buffer (TLB) bottleneck.
                                However, this is not supported on all systems.
                                On GNU/Linux, you need hugetlbfs support with
                                pre-reserved huge pages prior to running X-Mem.
                                On GNU/Linux, you also must use the ignore_numa
                                option, as hugetlbfs is not NUMA-aware at this
                                time.
    -R, --reads                 Use memory read-based patterns in load
                                traffic-generating threads.
    -W, --writes                Use memory write-based patterns in load
                                traffic-generating threads.
    -S, --stride_size           A stride size to use for load traffic-generating
                                threads, specified in powers-of-two multiples of
                                the chunk size(s). Allowed values: 1, -1, 2, -2,
                                4, -4, 8, -8, 16, -16. Positive indicates the
                                forward direction (increasing addresses), while
                                negative indicates the reverse direction.

If a given option is not specified, X-Mem defaults will be used where
appropriate.

=======================================================================
============================ EXAMPLE USAGE ============================
=======================================================================

Print X-Mem usage message and exit. If --help or -h is specified, benchmarks
will not run regardless of other options.

        xmem --help
        xmem -h


Run unloaded latency benchmarks with 5 iterations of each distinct benchmark
setting. The chunk size of 32 bits and sequential access pattern options will be
ignored as they only apply to load traffic-generating threads, which are unused
here as the default number of worker threads is 1. Console reporting will be
verbose.

        xmem -l --verbose -n5 --chunk_size=32 -s


Run throughput and loaded latency benchmarks on a per-thread working set size of
512 MB for a grand total of 1 GB of memory space. Use chunk sizes of 32 and 256
bits for load traffic-generating threads, and ignore NUMA effects. Number the
first benchmark test starting at 101 both in console reporting and CSV file
output (results.csv).

        xmem -t --latency -w524288 -f results.csv -c32 -c256 -i 101 -u -j2


Run 3 iterations of throughput and loaded latency on a working set of 128 KB per
thread. Use 4 worker threads in total. For load traffic-generating threads, use
all combinations of read and write memory accesses, random-access patterns,
forward sequential, and strided patterns of size -4 and -16 chunks. Ignore NUMA
effects in the system and use large pages. Finally, increase verbosity of
console output. Finally, run custom user extensions with indices 0 and 1. Note
that these may not necessarily obey other input parameters as their behavior is
defined by the extension author.

        xmem -w128 -n3 -j4 -l -t --extension=0 -e1 -s -S1 -S-4 -r -S16 -R -W -u
-L -v


Run EVERYTHING and dump results to file.

        xmem -a -v -ftest.csv

Have fun! =]

------------------------------------------------------------------------------------------------------------
BUILDING FROM SOURCE
------------------------------------------------------------------------------------------------------------

After you have set the desired compile-time options, build the source. On Windows, running build-win.bat should suffice. On GNU/Linux, run build-linux.sh. Each takes a single argument specifying the target architecture.

If you customize your build, make sure you use the "Release" mode for your OS/compiler. Do not include debug capabilities as it can dramatically affect performance of the benchmarks, leading to pessimistic results.

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

------------------------------------------------------------------------------------------------------------
ACKNOWLEDGMENT
------------------------------------------------------------------------------------------------------------

Mark Gottscho would like to thank Dr. Mohammed Shoaib of Microsoft Research and Dr. Sriram Govindan of Microsoft for their mentorship in the creation of this software. Further thanks to Dr. Bikash Sharma, Mark Santaniello, Mike Andrewartha, and Laura Caulfield of Microsoft for their contributions, feedback, and assistance. Thank you as well to Dr. Jie Liu of Microsoft Research, Dr. Badriddine Khessib and Dr. Kushagra Vaid of Microsoft, and Prof. Puneet Gupta of UCLA for giving me the opportunity to create this work. Finally, Mark would like to thank Dr. Fedor Pikus of Mentor Graphics for teaching him some useful HPC programming techniques.
