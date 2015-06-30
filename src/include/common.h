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
 *
 * Author: Mark Gottscho <mgottscho@ucla.edu>
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
#include <windows.h>
#endif

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h> //for Intel __m128i datatype
#include <immintrin.h> //for Intel __m256i datatype
#endif

#if defined(__ARM_NEON)
#include <arm_neon.h> //For ARM uint64x2_t datatype
#endif

namespace xmem {

#define VERSION "2.2.6"

#if !defined(_WIN32) && !defined(__gnu_linux__)
#error Neither Windows/GNULinux build environments were detected!
#endif

//Windows: convert platform-specific preprocessor flags for architecture to xmem-specific constants
#ifdef _WIN32

#ifdef _M_IX86 //Intel x86
#define ARCH_INTEL
#define ARCH_INTEL_X86
#endif

#ifdef _M_X64 //Intel x86-64
#define ARCH_INTEL
#define ARCH_INTEL_X86_64
#define ARCH_64BIT
#define HAS_NUMA
#define HAS_LARGE_PAGES
#endif

#ifdef _M_IX86_FP //Intel SSE/SSE2 extensions
#define ARCH_INTEL
#if _M_IX86_FP == 1
#define ARCH_INTEL_SSE
#endif
#if _M_IX86_FP == 2
#define ARCH_INTEL_SSE
#define ARCH_INTEL_SSE2
#endif
#endif

#ifdef __AVX__ //Intel AVX extensions
#define ARCH_INTEL
#define ARCH_INTEL_AVX
#endif

#ifdef __AVX2__ //Intel AVX2 extensions
#define ARCH_INTEL
#define ARCH_INTEL_AVX2
#endif

#ifdef _M_AMD64 //AMD64
#define ARCH_INTEL
#define ARCH_AMD64
#define ARCH_64BIT
#endif

#ifdef _M_ARM //ARM architecture
#define ARCH_ARM
#endif

#ifdef __ARM_NEON
#define ARCH_ARM_NEON //FIXME: I don't think there is a way to explicitly enable/disable NEON support on Windows, so I stole the gcc preprocessor definition __ARM_NEON for this.
#endif

#ifdef _M_ARM_FP //ARM extensions
#if _M_ARM_FP >= 30 && _M_ARM_FP <= 39
#define ARCH_ARM_VFP_V3
#endif
#if _M_ARM_FP >= 40 && _M_ARM_FP <= 49
#define ARCH_ARM_VFP_V3
#define ARCH_ARM_VFP_V4
#endif
#endif

#endif

//Linux: convert platform-specific preprocessor flags for architecture to xmem-specific constants
#ifdef __gnu_linux__

#ifdef __i386__ //Intel x86
#define ARCH_INTEL
#define ARCH_INTEL_X86
#endif

#ifdef __x86_64__ //Intel x86-64
#define ARCH_INTEL
#define ARCH_INTEL_X86_64
#define ARCH_64BIT
#define HAS_NUMA
#define HAS_LARGE_PAGES
#endif

#ifdef __SSE__ //Intel SSE extensions
#define ARCH_INTEL
#define ARCH_INTEL_SSE
#endif

#ifdef __SSE2__ //Intel SSE2 extensions
#define ARCH_INTEL
#define ARCH_INTEL_SSE2
#endif

#ifdef __SSE3__ //Intel SSE3 extensions
#define ARCH_INTEL
#define ARCH_INTEL_SSE3
#endif

#ifdef __AVX__ //Intel AVX extensions
#define ARCH_INTEL
#define ARCH_INTEL_AVX
#endif

#ifdef __AVX2__ //Intel AVX2 extensions
#define ARCH_INTEL
#define ARCH_INTEL_AVX2
#endif

#ifdef __amd64__ //AMD64
#define ARCH_INTEL
#define ARCH_AMD64
#define ARCH_64BIT
#endif

#ifdef __arm__ //ARM architecture
#define ARCH_ARM
#define ARCH_ARM_VFP_V3 //FIXME: this is assumed, as I don't know how to check directly
#endif

#ifdef __aarch64__ //ARM 64-bit
#define ARCH_ARM
#define ARCH_ARM_64
#define ARCH_ARM_VFP_V4 //FIXME: this is assumed, as I don't know how to check directly
#define ARCH_64BIT
#endif

#ifdef __ARM_ARCH_7__ //ARMv7
#define ARCH_ARM
#define ARCH_ARM_V7
#endif

#ifdef __ARM_ARCH_8__ //ARMv8
#define ARCH_ARM
#define ARCH_ARM_V8
#define ARCH_ARM_64
#define ARCH_ARM_VFP_V4 //FIXME: this is assumed, as I don't know how to check directly
#define ARCH_64BIT
#endif

#ifdef __ARM_NEON //ARM NEON extensions
#define ARCH_ARM
#define ARCH_ARM_NEON
#define ARCH_ARM_VFP_V4 //FIXME: this is assumed, as I don't know how to check directly
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

#ifdef ARCH_64BIT
#define GB_4 4294967296
#endif

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
*   User-configurable compilation configuration
*
*  Feel free to change these as needed. To disable an option, simply comment out its #define statement. To enable an option, ensure it is not commented out.
*   In some cases, such as chunk size, stride size, etc. for throughput benchmarks, all combinations of the options will be used! This might dramatically increase runtime.
*/

//Which timer to use in the benchmarks. Only one may be selected!
#define USE_OS_TIMER /**< RECOMMENDED ENABLED. If enabled, uses the QPC timer on Windows and the POSIX clock_gettime() on GNU/Linux for all timing purposes. */
//#define USE_HW_TIMER /**< RECOMMENDED DISABLED. If enabled, uses the platform-specific hardware timer (e.g., TSC on Intel x86-64). This may be less portable or have other implementation-specific quirks but for most purposes should work fine. */

#define BENCHMARK_DURATION_MS 250 /**< RECOMMENDED VALUE: At least 250. Number of milliseconds to run in each benchmark. */
#define THROUGHPUT_BENCHMARK_BYTES_PER_PASS 4096 /**< RECOMMENDED VALUE: 4096. Number of bytes read or written per pass of any ThroughputBenchmark. This must be less than or equal to the minimum working set size, which is currently 4 KB. */

#define POWER_SAMPLING_PERIOD_MS 1000 /**< RECOMMENDED VALUE: 1000. Sampling period in milliseconds for all power measurement mechanisms. */

//++++++++++++++++++ User-implemented extensions configuration here +++++++++++++++++++++
//Only one extension may be enabled at a time.
#define EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK /**< RECOMMENDED ENABLED. This allows for a custom extension to X-Mem that performs latency benchmarking with forward sequential 64-bit and 256-bit read-based load threads with variable delays injected in between memory accesses. */
//#define EXT_STREAM_BENCHMARK /**< RECOMMENDED DISABLED. This allows for a custom extension to X-Mem that performs stream copy, add, and triad kernels similar to those of the well-known STREAM throughput benchmark. */

/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/


//Compile-time options checks: timers
#ifdef USE_OS_TIMER
#ifdef _WIN32
#define USE_QPC_TIMER 
#endif
#ifdef __gnu_linux__
#define USE_POSIX_TIMER
#endif
#endif

#ifdef USE_HW_TIMER
#ifdef ARCH_INTEL
#define USE_TSC_TIMER
#endif
#ifdef ARCH_ARM
#error TODO: Implement ARM hardware timer.
#endif
#endif

#if defined(USE_OS_TIMER) && defined(USE_HW_TIMER)
#error Only one type of timer may be defined!
#endif 

#if BENCHMARK_DURATION_MS <= 0
#error BENCHMARK_DURATION_MS must be positive!
#endif

#if THROUGHPUT_BENCHMARK_BYTES_PER_PASS > DEFAULT_PAGE_SIZE || THROUGHPUT_BENCHMARK_BYTES_PER_PASS <= 0
#error THROUGHPUT_BENCHMARK_BYTES_PER_PASS must be less than or equal to the minimum possible working set size. It also must be a positive integer.
#endif

//Compile-time options checks: power sampling frequency. TODO: this should probably be a runtime option
#if !defined(POWER_SAMPLING_PERIOD_MS) || POWER_SAMPLING_PERIOD_MS <= 0
#error POWER_SAMPLING_PERIOD_MS must be defined and greater than 0!
#endif

//#ifdef ARCH_64BIT
    typedef uint64_t tick_t;
//#else
    ///*typedef union {
        //uint32_t upper;
        //uint32_t lower;
    //} tick_t;*/
    //typedef uint32_t tick_t; //FIXME: this will easily roll over on a ~GHz machine over a 4-second benchmark! On 32-bit systems, we either need to reduce benchmark duration to about 250 ms or find a way to hack 64-bit timer with rollover. So far the 250 ms approach seems to work OK.
//#endif

    extern bool g_verbose;
    extern size_t g_page_size;
    extern size_t g_large_page_size;
    extern uint32_t g_num_nodes;
    extern uint32_t g_num_logical_cpus;
    extern uint32_t g_num_physical_packages;
    extern uint32_t g_starting_test_index;
    extern uint32_t g_test_index;
    extern tick_t g_ticks_per_ms;
    extern float g_ns_per_tick;

    //Typedef the platform specific stuff to word sizes to match 4 different chunk options
#if defined(ARCH_64BIT) || defined(ARCH_ARM_NEON)
#define HAS_WORD_64 
#endif
#if defined(ARCH_INTEL_AVX) || defined(ARCH_ARM_NEON)
#define HAS_WORD_128
#endif
#ifdef ARCH_INTEL_AVX
#define HAS_WORD_256
#endif

    typedef uint32_t Word32_t; 
#ifdef HAS_WORD_64
    typedef uint64_t Word64_t;
#endif
#ifdef HAS_WORD_128
#ifdef ARCH_INTEL
    typedef __m128i Word128_t;
#endif
#if defined(ARCH_ARM) && defined(ARCH_ARM_NEON)
    typedef uint64x2_t Word128_t;
#endif
#endif
#ifdef HAS_WORD_256
#ifdef ARCH_INTEL
    typedef __m256i Word256_t; //Not possible on current ARM systems.
#endif
#ifdef ARCH_ARM
    #error ARM does not support 256-bit memory operations, this should not have happened.
#endif
#endif

    /**
     * @brief Memory access patterns are broadly categorized by sequential or random-access.
     */
    typedef enum {
        SEQUENTIAL,
        RANDOM,
        NUM_PATTERN_MODES
    } pattern_mode_t;

    /**
     * @brief Memory access batterns are broadly categorized by reads and writes.
     */
    typedef enum {
        READ,
        WRITE,
        NUM_RW_MODES
    } rw_mode_t;

    /**
     * @brief Legal memory read/write chunk sizes in bits.
     */
    typedef enum {
        CHUNK_32b,
#ifdef HAS_WORD_64
        CHUNK_64b,
#endif
#ifdef HAS_WORD_128
        CHUNK_128b,
#endif
#ifdef HAS_WORD_256
        CHUNK_256b,
#endif
        NUM_CHUNK_SIZES
    } chunk_size_t;

    typedef enum {
#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK
        EXT_NUM_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK,
#endif
#ifdef EXT_STREAM_BENCHMARK
        EXT_NUM_STREAM_BENCHMARK,
#endif
        NUM_EXTENSIONS
    } ext_t;

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
     * @brief Initializes the timer and outputs results to the console for sanity checking.
     */
    void setup_timer();

    /**
     * @brief Reports timer info to the console.
     */
    void report_timer();

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

    /**
     * @brief Reports the system configuration to the console as indicated by global variables.
     */
    void report_sys_info();

    /**
     * @brief Query the timer for the start of a timed section of code.
     * @returns The starting tick for some timed section of code using the timer.
     */
    tick_t start_timer();

    /**
     * @brief Query the timer for the end of a timed section of code.
     * @returns The ending tick for some timed section of code using the timer.
     */
    tick_t stop_timer();

#ifdef _WIN32
    /**
     * @brief Increases the scheduling priority of the calling thread.
     * @param originalPriorityClass The Windows priority class before priority was increased.
     * @param originalPriority The Windows priority before priority was increased.
     * @returns True on success.
     */
    bool boostSchedulingPriority(DWORD& originalPriorityClass, DWORD& originalPriority);
#endif

#ifdef __gnu_linux__
    /**
     * @brief Increases the scheduling priority of the calling thread.
     * @returns True on success.
     */
    bool boostSchedulingPriority();
#endif

#ifdef _WIN32
    /**
     * @brief Reverts the scheduling priority of the calling thread.
     * @param originalPriorityClass The Windows priority class before priority was increased.
     * @param originalPriority The Windows priority before priority was increased.
     * @returns True on success.
     */
    bool revertSchedulingPriority(DWORD originalPriorityClass, DWORD originalPriority);
#endif

#ifdef __gnu_linux__
    /**
     * @brief Reverts the scheduling priority of the calling thread.
     * @returns True on success.
     */
    bool revertSchedulingPriority();
#endif
};

#endif
