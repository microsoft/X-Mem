README
------------------------------------------------------------------------------------------------------------

X-Mem: A Cross-Platform and Extensible Memory Characterization Tool for the Cloud v2.3.1
------------------------------------------------------------------------------------------------------------

X-Mem is a flexible open-source research tool for characterizing memory hierarchy throughput, latency, power, and more. The tool was developed jointly by Microsoft and the UCLA NanoCAD Lab. This project was started by Mark Gottscho (Email: mgottscho@ucla.edu) as a Summer 2014 PhD intern at Microsoft Research. X-Mem is released freely and open-source under the MIT License. The project is under active development. Stay tuned for more updates.

PROJECT REVISION DATE: August 3, 2015.

------------------------------------------------------------------------------------------------------------
VERSION CONTROL AND OBTAINING SOURCE CODE
------------------------------------------------------------------------------------------------------------

This project is under version control using git. Its master repository is hosted at <https://github.com/Microsoft/X-Mem.git>. There is also another mirrored fork at <https://github.com/nanocad-lab/X-Mem.git>.

------------------------------------------------------------------------------------------------------------
OBTAINING PRE-BUILT BINARIES
------------------------------------------------------------------------------------------------------------

If you don't want to build X-Mem from source, you can get regular releases of pre-built binaries for Windows and GNU/Linux from the project homepage, located at <https://nanocad-lab.github.io/X-Mem>.

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
DESIGN PHILOSOPHY
------------------------------------------------------------------------------------------------------------

Previous memory benchmarking tools such as STREAM/STREAM2, lmbench3, and mlc all had various shortcomings that made them inadequate for emerging needs in the cloud. In particular, cloud systems present four key challenges that we addressed with X-Mem. Stay tuned for updates -- more information will be coming soon.

------------------------------------------------------------------------------------------------------------
FEATURES
------------------------------------------------------------------------------------------------------------

This tool is provided as open source with the hope of being useful to the broader research and development community. Here are some of X-Mem's features.

Flexibility: Easy reconfiguration for different combinations of tests
- Working sets in increments of 4KB, allowing cache up to main memory-level benchmarking
- NUMA support
- Multi-threading support
- Large page support

Extensibility: Modularity via C++ object-oriented principles
- Supports rapid addition of new benchmark kernel routines
- Example: stream triad algorithm, impact of false sharing, etc. are possible with minor changes

Cross-platform: Currently implemented for two OSes and architecture families
- Windows: Intel x86 (32-bit), x86-64, and x86-64 with AVX extensions
- GNU/Linux: Intel x86 (32-bit), x86-64, and x86-64 with AVX extensions, ARM (32-bit), ARM (32-bit) with NEON, ARMv8 (64-bit)
- Designed to allow straightforward porting to other operating systems and ISAs
- ARM on Windows currently not possible due to some incompatibilites and/or lack of support. This may be resolved in the future.

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
	- This is done for 32, 64, 128, and 256-bit load chunk sizes where applicable using the forward sequential read pattern.

For feature requests, please refer to the contact information at the end of this README.

------------------------------------------------------------------------------------------------------------
RUNTIME PREREQUISITES
------------------------------------------------------------------------------------------------------------

There are a few runtime prerequisites in order for the software to run correctly.

HARDWARE:

- Intel x86, x86-64, or x86-64 with AVX CPU. AMD CPUs should also work although this has not been tested!
- ARM Cortex-A series processors with VFP and NEON extensions. Specifically tested on ARM Cortex A9 (32-bit) which is ARMv7. 64-bit builds for ARMv8-A should also work but have not been tested. GNU/Linux builds only. You can attempt Windows, but I have issues linking the 32-bit version and compiling the NEON version. Also, as far as I can see there is no Windows support for ARMv8 at all.

WINDOWS:

- Microsoft Windows 8.1 64-bit or later, Server 2012 R2 or later.
- Microsoft Visual C++ 2013 Redistributables (32-bit) -- for x86 (32-bit) builds
- Microsoft Visual C++ 2013 Redistributables (64-bit) -- for x86-64 and x86-64 with AVX builds
- You MAY need Administrator privileges, in order to:
    - Use large pages, if the --large_pages option is selected (see USAGE, below)
    - The first time you use --large_pages on a given Windows machine, you may need to ensure that your Windows user account has the necessary rights to allow lockable memory pages. To do this on Windows 8, run gpedit.msc --> Local Computer Policy --> Computer Configuration --> Windows Settings --> Security Settings --> Local Policies --> User Rights Assignment --> Add your username to "Lock pages in memory". Then log out and then log back in.
    - Use the PowerReader interface, depending on end-user implementation
    - Elevate thread priority and pin threads to logical CPUs for improved performance and benchmarking consistency

GNU/LINUX:

- GNU utilities with support for C++11. Tested with gcc 4.8.2 on Ubuntu 14.04 LTS for x86 (32-bit), x86-64, and x86-64 with AVX on Intel Sandy Bridge, Ivy Bridge, and Haswell families.
- libhugetlbfs. You can obtain it at <http://libhugetlbfs.sourceforge.net>. On Ubuntu systems, you can install using "sudo apt-get install libhugetlbfs0".
- Potentially, administrator privileges, if you plan to use the --large_pages option.
    - During runtime, if the --large_pages option is selected, you may need to first manually ensure that large pages are available from the OS. This can be done by running "hugeadm --pool-list". It is recommended to set minimum pool to 1GB (in order to measure DRAM effectively). If needed, this can be done by running "hugeadm --pool-pages-min 2MB:512". Alternatively, run the linux_setup_runtime_hugetlbfs.sh script that is provided with X-Mem. 

------------------------------------------------------------------------------------------------------------
INSTALLATION
------------------------------------------------------------------------------------------------------------

The only file that is needed to run on Windows is the respective executable xmem-win-<ARCH>.exe on Windows, and xmem-linux-<ARCH> on GNU/Linux. It has no other dependencies aside from the pre-installed system prerequisites listed above.

------------------------------------------------------------------------------------------------------------
USAGE
------------------------------------------------------------------------------------------------------------

To get help using the tool, simply run it with the "-h" or "--help" option. It includes a list of all available options and provides usage examples.

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

- bash shell. Other shells could work but are untested.
- gcc with support for the C++11 standard. Tested with gcc version 4.8.2 on Ubuntu 14.04 LTS for x86 (32-bit), x86-64, x86-64 with AVX builds.
- gcc cross-compiler for ARM targets (assumed build on x86-64 Ubuntu host).
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
CONTACT, FEEDBACK, AND BUG REPORTS
------------------------------------------------------------------------------------------------------------

For questions, comments, criticism, bug reports, and other feedback for this software, please contact Mark Gottscho via email at <mgottscho@ucla.edu> or via web at <http://www.seas.ucla.edu/~gottscho>.

------------------------------------------------------------------------------------------------------------
ACKNOWLEDGMENT
------------------------------------------------------------------------------------------------------------

Mark Gottscho would like to thank Dr. Mohammed Shoaib of Microsoft Research and Dr. Sriram Govindan of Microsoft for their mentorship in the creation of this software. Further thanks to Dr. Bikash Sharma, Mark Santaniello, Mike Andrewartha, and Laura Caulfield of Microsoft for their contributions, feedback, and assistance. Thank you as well to Dr. Jie Liu of Microsoft Research, Dr. Badriddine Khessib and Dr. Kushagra Vaid of Microsoft, and Prof. Puneet Gupta of UCLA for giving Mark the opportunity to create this work. Finally, Mark would like to thank Dr. Fedor Pikus of Mentor Graphics for teaching him some useful HPC programming techniques.
