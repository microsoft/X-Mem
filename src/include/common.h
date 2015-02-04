/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Microsoft
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * 
 * @brief Header file for common preprocessor definitions, macros, functions, and global constants.
 */

#ifndef __COMMON_H
#define __COMMON_H

//Libraries
#include <cstdint>
#include <cstddef>

#ifdef _WIN32
#include <intrin.h>
#endif

#ifdef __gnu_linux__
#include <immintrin.h>
#endif

namespace xmem {
	namespace common {

#define VERSION "1.2.10"

#if !defined(_WIN32) && !defined(__gnu_linux__)
#error Neither Windows/GNULinux build environments were detected!
#endif

//Windows: convert platform-specific preprocessor flags for architecture to xmem-specific constants
#ifdef _WIN32

#ifdef _M_IX86 //Intel x86
#define ARCH_INTEL_X86
#endif

#ifdef _M_X64 //Intel x86-64
#define ARCH_INTEL_X86_64
#endif

#ifdef _M_IX86_FP //Intel x86-64 SSE2 extensions
#define ARCH_INTEL_X86_64_SSE2
#endif

#ifdef __AVX__ //Intel x86-64 AVX extensions
#define ARCH_INTEL_X86_64_AVX
#endif

#ifdef __AVX2__ //Intel x86-64 AVX2 extensions
#define ARCH_INTEL_X86_64_AVX2
#endif

#ifdef _AMD64 //AMD64
#define ARCH_AMD64
#endif

#ifdef _M_ARM //ARM architecture
#define ARCH_ARM
#endif

#endif

//Linux: convert platform-specific preprocessor flags for architecture to xmem-specific constants
#ifdef __gnu_linux__

#ifdef __i386__ //Intel x86
#define ARCH_INTEL_X86
#endif

#ifdef __x86_64__ //Intel x86-64
#define ARCH_INTEL_X86_64
#endif

#ifdef __SSE2__ //Intel x86-64 SSE2 extensions
#define ARCH_INTEL_X86_64_SSE2
#endif

#ifdef __AVX__ //Intel x86-64 AVX extensions
#define ARCH_INTEL_X86_64_AVX
#endif

#ifdef __AVX2__ //Intel x86-64 AVX2 extensions
#define ARCH_INTEL_X86_64_AVX2
#endif

#ifdef __amd64__ //AMD64
#define ARCH_AMD64
#endif

#ifdef __arm__ //ARM architecture
#define ARCH_ARM
#endif

#endif

//Common size constants in bytes
#define KB 1024
#define MB 1048576
#define MB_4 4194304
#define MB_16 16777216
#define MB_64 67108864
#define MB_256 268435456
#define MB_512 536870912
#define GB 1073741824
#define GB_4 4294967296

//Default compile-time constants
#define DEFAULT_PAGE_SIZE 4*KB /**< Default platform page size in bytes. This generally should not be relied on, but is a failsafe. */
#define DEFAULT_LARGE_PAGE_SIZE 2*MB /**< Default platform large page size in bytes. This generally should not be relied on, but is a failsafe. */
#define DEFAULT_WORKING_SET_SIZE_PER_THREAD DEFAULT_PAGE_SIZE /**< Default working set size in bytes. */
#define DEFAULT_NUM_WORKER_THREADS 1 /**< Default number of worker threads to use. */
#define DEFAULT_NUM_NODES 0 /**< Default number of NUMA nodes. */
#define DEFAULT_NUM_PHYSICAL_PACKAGES 0 /**< Default number of physical packages. */
#define DEFAULT_NUM_PHYSICAL_CPUS 0 /**< Default number of physical CPU cores. */
#define DEFAULT_NUM_LOGICAL_CPUS 0 /**< Default number of logical CPU cores. */
#define DEFAULT_NUM_L1_CACHES 0 /**< Default number of L1 caches. */
#define DEFAULT_NUM_L2_CACHES 0 /**< Default number of L2 caches. */
#define DEFAULT_NUM_L3_CACHES 0 /**< Default number of L3 caches. */
#define DEFAULT_NUM_L4_CACHES 0 /**< Default number of L4 caches. */
#define MIN_ELAPSED_TICKS 10000 /**< If any routine measured fewer than this number of ticks its results should be viewed with suspicion. This is because the latency of the timer itself will matter. */


//Loop unrolling tricks. There are a bunch so that we can use the length needed for each situation. Unrolling too much hurts code size and instruction reuse. Yes, an unroll of 65536 is probably unnecessary. :)
//Note that I don't provide a default "UNROLL" macro, because I believe the programmer should know exactly how many times they are unrolling to make sure loop bounds are not violated.
#define UNROLL2(x) x x
#define UNROLL4(x) UNROLL2(x) UNROLL2(x)
#define UNROLL8(x) UNROLL4(x) UNROLL4(x)
#define UNROLL16(x) UNROLL8(x) UNROLL8(x)
#define UNROLL32(x) UNROLL16(x) UNROLL16(x)
#define UNROLL64(x) UNROLL32(x) UNROLL32(x)
#define UNROLL128(x) UNROLL64(x) UNROLL64(x)
#define UNROLL256(x) UNROLL128(x) UNROLL128(x)
#define UNROLL512(x) UNROLL256(x) UNROLL256(x)
#define UNROLL1024(x) UNROLL512(x) UNROLL512(x)
#define UNROLL2048(x) UNROLL1024(x) UNROLL1024(x)
#define UNROLL4096(x) UNROLL2048(x) UNROLL2048(x)
#define UNROLL8192(x) UNROLL4096(x) UNROLL4096(x)
#define UNROLL16384(x) UNROLL8192(x) UNROLL8192(x)
#define UNROLL32768(x) UNROLL16384(x) UNROLL16384(x)
#define UNROLL65536(x) UNROLL32768(x) UNROLL32768(x)

#define LATENCY_BENCHMARK_UNROLL_LENGTH 512 /**< Number of unrolls in the latency benchmark pointer chasing core function. */

/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/*
 *	User-configurable compilation configuration
 *
 *  Feel free to change these as needed. To disable an option, simply comment out its #define statement. To enable an option, ensure it is not commented out.
 *	In some cases, such as chunk size, stride size, etc. for throughput benchmarks, all combinations of the options will be used! This might dramatically increase runtime.
 */

//Which timer to use in the benchmarks. Only one may be selected!
//#define USE_QPC_TIMER /**< RECOMMENDED ENABLED. WINDOWS ONLY. Use the Windows QueryPerformanceCounter timer API. This is a safe bet as it is more hardware-agnostic and has fewer quirks, but it has lower resolution than the TSC timer. */
#define USE_TSC_TIMER /**< RECOMMENDED DISABLED. Use the Intel Time Stamp Counter native hardware timer. Only use this if you know what you are doing. */

#ifdef _WIN32 //DO NOT COMMENT THIS OUT
//#define USE_LARGE_PAGES /**< RECOMMENDED ENABLED. TODO: Currently only implemented correctly for Windows because of lackluster support for large pages in GNU/Linux, and the fact that libhugetlbfs is not NUMA-aware in the way we need. Allocate memory using large pages rather than small normal pages. In general, this is highly recommended, as the TLB can skew benchmark results for DRAM. */
#endif

//Benchmarking methodology. Only one may be selected!
#define USE_TIME_BASED_BENCHMARKS /**< RECOMMENDED ENABLED. All benchmarks run for an estimated amount of time, and the figures of merit are computed based on the amount of memory accesses completed in the time limit. This mode has more consistent runtime across different machines, memory performance, and working set sizes, but may have more conservative measurements for differing levels of cache hierarchy (overestimating latency and underestimating throughput). */
//#define USE_SIZE_BASED_BENCHMARKS /**< RECOMMENDED DISABLED. All benchmarks run for an estimated amount of memory accesses, and the figures of merit are computed based on the length of time required to run the benchmark. This mode may have highly varying runtime across different machines, memory performance, and working set sizes, but may have more optimistic measurements across differing levels of cache hierarchy (underestimating latency and overestimating throughput). */

#ifdef USE_TIME_BASED_BENCHMARKS //DO NOT COMMENT THIS OUT!
#define BENCHMARK_DURATION_SEC 4 /**< RECOMMENDED VALUE: At least 2. Number of seconds to run in each benchmark. */
#define THROUGHPUT_BENCHMARK_BYTES_PER_PASS 4096 /**< RECOMMENDED VALUE: 4096. Number of bytes read or written per pass of any ThroughputBenchmark. This must be less than or equal to the minimum working set size, which is currently 4 KB. */
#endif //DO NOT COMMENT THIS OUT

#ifdef USE_SIZE_BASED_BENCHMARKS //DO NOT COMMENT THIS OUT
//#define USE_PASSES_CURVE_1 /**< RECOMMENDED DISABLED. The passes per iteration of a benchmark will be given by y = 65536 / working_set_size_KB */
#define USE_PASSES_CURVE_2 /**< RECOMMENDED ENABLED. The passes per iteration of a benchmark will be given by y = 4*2097152 / working_set_size_KB^2 */
#endif //DO NOT COMMENT THIS OUT

//Throughput benchmark access patterns
#define USE_THROUGHPUT_SEQUENTIAL_PATTERN /**< RECOMMENDED ENABLED. Run the sequential family pattern of ThroughputBenchmarks. */
//#define USE_THROUGHPUT_RANDOM_PATTERN /**< RECOMMENDED DISABLED. Run the random-access family pattern of ThroughputBenchmarks. TODO: Not yet implemented. */

#ifdef USE_THROUGHPUT_SEQUENTIAL_PATTERN //DO NOT COMMENT THIS OUT
//Throughput benchmark forward strides
#define USE_THROUGHPUT_FORW_STRIDE_1 /**< RECOMMENDED ENABLED. In throughput benchmarks with sequential pattern, do forward strides of 1 chunk (forward sequential). */
//#define USE_THROUGHPUT_FORW_STRIDE_2 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do forward strides of 2 chunks. */
//#define USE_THROUGHPUT_FORW_STRIDE_4 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do forward strides of 4 chunks. */
//#define USE_THROUGHPUT_FORW_STRIDE_8 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do forward strides of 8 chunks. */
//#define USE_THROUGHPUT_FORW_STRIDE_16 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do forward strides of 16 chunks. */

//Throughput benchmark reverse strides
#define USE_THROUGHPUT_REV_STRIDE_1 /**< RECOMMENDED ENABLED. In throughput benchmarks with sequential pattern, do reverse strides of 1 chunk (reverse sequential). */
//#define USE_THROUGHPUT_REV_STRIDE_2 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do reverse strides of 2 chunks. */
//#define USE_THROUGHPUT_REV_STRIDE_4 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do reverse strides of 4 chunks. */
//#define USE_THROUGHPUT_REV_STRIDE_8 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do reverse strides of 8 chunks. */
//#define USE_THROUGHPUT_REV_STRIDE_16 /**< RECOMMENDED DISABLED. In throughput benchmarks with sequential pattern, do reverse strides of 16 chunks. */
#endif //DO NOT COMMENT THIS OUT

//Throughput benchmark reads and writes
#define USE_THROUGHPUT_READS /**< RECOMMENDED ENABLED. In throughput benchmarks, read from memory. */
#define USE_THROUGHPUT_WRITES /**< RECOMMENDED ENABLED. In throughput benchmarks, write to memory. */

//Latency benchmark pointer chasing construction method
#define USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN /**< RECOMMENDED ENABLED. In latency benchmarks, generate the pointer chasing pattern using a random shuffle, which has a chance of creating small cycles. Much faster to run but strictly less correct. O(N) */
//#define USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN /**< RECOMMENDED DISABLED. In latency benchmarks, generate the pointer chasing pattern using a random directed Hamiltonian Cycle across the entire memory space under test. Slow to compute as it is O(N^2), but strictly more correct. */

#define POWER_SAMPLING_PERIOD_SEC 1 /**< RECOMMENDED VALUE: 1. Sampling period in seconds for all power measurement mechanisms. */
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/


//Compile-time options checks
#if defined(USE_QPC_TIMER) && !defined(_WIN32)
#error USE_QPC_TIMER may only be defined for a Windows system!
#endif 

#if !defined(USE_QPC_TIMER) && !defined(USE_TSC_TIMER)
#error One type of timer must be selected!
#endif

#if defined(USE_QPC_TIMER) && defined(USE_TSC_TIMER)
#error Only one type of timer may be specified!
#endif

#if (defined(USE_TIME_BASED_BENCHMARKS) && defined(USE_SIZE_BASED_BENCHMARKS)) || (!defined(USE_TIME_BASED_BENCHMARKS) && !defined(USE_SIZE_BASED_BENCHMARKS))
#error Exactly one of USE_TIME_BASED_BENCHMARKS and USE_SIZE_BASED_BENCHMARKS must be defined!
#endif

#ifdef USE_TIME_BASED_BENCHMARKS
#ifndef BENCHMARK_DURATION_SEC
#error BENCHMARK_DURATION_SEC must be defined!
#endif
#if BENCHMARK_DURATION_SEC <= 0
#error BENCHMARK_DURATION_SEC must be a positive integer!
#endif
#ifndef THROUGHPUT_BENCHMARK_BYTES_PER_PASS
#error THROUGHPUT_BENCHMARK_BYTES_PER_PASS must be defined!
#else
#if THROUGHPUT_BENCHMARK_BYTES_PER_PASS > DEFAULT_PAGE_SIZE || THROUGHPUT_BENCHMARK_BYTES_PER_PASS <= 0
#error THROUGHPUT_BENCHMARK_BYTES_PER_PASS must be less than or equal to the minimum possible working set size. It also must be a positive integer.
#endif
#endif
#endif

#ifdef USE_SIZE_BASED_BENCHMARKS
#if (defined(USE_PASSES_CURVE_1) && defined(USE_PASSES_CURVE_2)) || (!defined(USE_PASSES_CURVE_1) && !defined(USE_PASSES_CURVE_2))
#error Exactly one passes curve must be defined.
#endif
#endif

#if !defined (USE_THROUGHPUT_SEQUENTIAL_PATTERN) && !defined(USE_THROUGHPUT_RANDOM_PATTERN)
#error At least one throughput pattern compile-time option must be set!
#endif

#if defined(USE_THROUGHPUT_SEQUENTIAL_PATTERN)
#if !defined(USE_THROUGHPUT_FORW_STRIDE_1) && !defined(USE_THROUGHPUT_FORW_STRIDE_2) && !defined(USE_THROUGHPUT_FORW_STRIDE_4) && !defined(USE_THROUGHPUT_FORW_STRIDE_8) && !defined(USE_THROUGHPUT_FORW_STRIDE_16) && !defined(USE_THROUGHPUT_REV_STRIDE_1) && !defined(USE_THROUGHPUT_REV_STRIDE_2) && !defined(USE_THROUGHPUT_REV_STRIDE_4) && !defined(USE_THROUGHPUT_REV_STRIDE_8) && !defined(USE_THROUGHPUT_REV_STRIDE_16) 
#error Throughput benchmark sequential pattern compile-time option was selected, but no stride options were set! At least one must be enabled.
#endif
#endif

#if !defined(USE_THROUGHPUT_READS) && !defined(USE_THROUGHPUT_WRITES)
#error At least one read or write mode compile-time option must be selected for throughput benchmarks!
#endif

#if !defined(USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN) && !defined(USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN)
#error One latency benchmark pattern compile-time option must be selected!
#endif

#if defined(USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN) && defined(USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN)
#error Only one latency benchmark pattern compile-time option must be selected!
#endif

#if !defined(POWER_SAMPLING_PERIOD_SEC) || POWER_SAMPLING_PERIOD_SEC <= 0
#error POWER_SAMPLING_PERIOD_SEC must be defined and greater than 0!
#endif

		extern bool g_verbose;
		extern size_t g_page_size;
		extern size_t g_large_page_size;
		extern uint32_t g_num_nodes;
		extern uint32_t g_num_logical_cpus;
		extern uint32_t g_num_physical_packages;
		extern uint32_t g_starting_test_index;
		extern uint32_t g_test_index;

		//Typedef the platform specific stuff to word sizes to match 4 different chunk options
		typedef uint32_t Word32_t;
#ifdef ARCH_INTEL_X86_64
		typedef uint64_t Word64_t;
#endif
#ifdef ARCH_INTEL_X86_64_AVX
		typedef __m128i Word128_t;
		typedef __m256i Word256_t;
#endif
		/**
		 * @brief Memory access patterns are broadly categorized by sequential or random-access.
		 */
		typedef enum {
#ifdef USE_THROUGHPUT_SEQUENTIAL_PATTERN
			SEQUENTIAL,
#endif
#ifdef USE_THROUGHPUT_RANDOM_PATTERN
			RANDOM,
#endif
			NUM_PATTERN_MODES
		} pattern_mode_t;

		/**
		 * @brief Memory access batterns are broadly categorized by reads and writes.
		 */
		typedef enum {
#ifdef USE_THROUGHPUT_READS
			READ,
#endif
#ifdef USE_THROUGHPUT_WRITES
			WRITE,
#endif
			NUM_RW_MODES
		} rw_mode_t;

		/**
		 * @brief Legal memory read/write chunk sizes in bits.
		 */
		typedef enum {
			CHUNK_32b,
			CHUNK_64b,
			CHUNK_128b,
			CHUNK_256b,
			NUM_CHUNK_SIZES
		} chunk_size_t;

		/**
		* @brief Prints a basic welcome message to the console with useful information.
		*/
		void print_welcome_message();

		/**
		 * @brief Prints the various C/C++ types to the console for this machine.
		 */
		void print_types_report();

		/**
		 * @brief Prints compile-time option information to the console.
		 */
		void print_compile_time_options();

		/**
		 * @brief Tests any enabled timers and outputs results to the console for sanity checking.
		 */
		void test_timers();

		/**
		 * @brief Checks to see if the calling thread can be locked to all logical CPUs in the system, and reports to the console the progress.
		 */
		void test_thread_affinities();

		/**
		 * @brief Sets the affinity of the calling thread to the lowest numbered logical CPU in the given NUMA node.
		 * TODO: Improve this functionality, it is quite limiting.
		 * @param numa_node The NUMA node number to select a CPU from.
		 * @returns True on success.
		 */
		bool lock_thread_to_numa_node(uint32_t numa_node);

		/**
		 * @brief Clears the affinity of the calling thread to any given NUMA node.
		 * @returns True on success.
		 */
		bool unlock_thread_to_numa_node();

		/**
		 * @brief Sets the affinity of the calling thread to a given logical CPU.
		 * @param cpu_id The logical CPU identifier to lock the thread to.
		 * @returns True on success.
		 */
		bool lock_thread_to_cpu(uint32_t cpu_id);

		/**
		 * @brief Clears the affinity of the calling thread to any given logical CPU.
		 * @returns True on success.
		 */
		bool unlock_thread_to_cpu();
		
		/**
		 * @brief Gets the CPU ID for a logical CPU of interest in a particular NUMA node.
		 * For example, if numa_node is 1 and cpu_in_node is 2, and there are 4 logical CPUs per node, then this will give the answer 6 (6th CPU), assuming CPU IDs start at 0.
		 * @param numa_node The NUMA node of interest.
		 * @param cpu_in_node The Nth logical CPU in the node.
		 * @returns The Nth logical CPU ID in the specified NUMA node. If none is found, returns -1.
		 */
		int32_t cpu_id_in_numa_node(uint32_t numa_node, uint32_t cpu_in_node);

		/**
		 * @brief Computes the number of passes to use for a given working set size in KB, when size-based benchmarking mode is enabled at compile-time.
		 * You may want to change this implementation to suit your needs. See the compile-time options in common.h.
		 * @param working_set_size_KB The working set size of the memory in KB.
		 * @returns The number of passes to use.
		 */
		size_t compute_number_of_passes(size_t working_set_size_KB);

		/**
		 * @brief Queries the page sizes from the system and sets relevant global variables.
		 * @returns False if the default value has to be used because the appropriate values could not be queried successfully from the OS.
		 */
		bool config_page_size();

		/**
		 * @brief Initializes useful global variables.
		 */
		void init_globals();

		/**
		 * @brief Sets up global variables based on system information at runtime.
		 * @returns 0 on success.
		 */
		int32_t query_sys_info();
	};
};

#endif
