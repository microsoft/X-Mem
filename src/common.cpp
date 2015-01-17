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
 * @brief Implementation file for common preprocessor definitions, macros, functions, and global constants.
 */

//Headers
#include <common.h>
#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
#include <x86_64/TSCTimer.h>
#endif
#if defined(_WIN32) && defined(USE_QPC_TIMER)
#include <win/QPCTimer.h>
#endif

//Libraries
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <unistd.h>
#include <pthread.h>
#include <numa.h>
#endif

namespace xmem {
	namespace common {
		size_t g_page_size; /**< Default page size on the system, in bytes. */
		size_t g_large_page_size; /**< Large page size on the system, in bytes. */
		uint32_t g_num_nodes; /**< Number of NUMA nodes in the system. */
		uint32_t g_num_logical_cpus; /**< Number of logical CPU cores in the system. */
		uint32_t g_num_physical_packages; /**< Number of physical CPU packages in the system. Generally this is the same as number of NUMA nodes, unless UMA emulation is done in hardware. */
		uint32_t g_starting_test_index; /**< Numeric identifier for the first benchmark test. */
		uint32_t g_test_index; /**< Numeric identifier for the current benchmark test. */
	};
};

void xmem::common::print_welcome_message() {
	//Greetings!
	std::cout << "--------------------------------------------------------------------" << std::endl;
	std::cout << "Extensible Memory Benchmarking Tool (X-Mem) v" << VERSION << std::endl;
	std::cout << "(C) Microsoft Corporation 2014" << std::endl;
	std::cout << "Originally authored by Mark Gottscho <mgottscho@ucla.edu>" << std::endl;
	std::cout << "--------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}

void xmem::common::print_types_report() {
	std::cout << std::endl << "These are the system type sizes:" << std::endl;
	std::cout << "short:  \t\t\t" << sizeof(short) << std::endl;
	std::cout << "int:  \t\t\t\t" << sizeof(int) << std::endl;
	std::cout << "long:  \t\t\t\t" << sizeof(long) << std::endl;
	std::cout << "long long:  \t\t\t" << sizeof(long long) << std::endl;
	std::cout << std::endl;
	std::cout << "unsigned short:  \t\t" << sizeof(unsigned short) << std::endl;
	std::cout << "unsigned:  \t\t\t" << sizeof(unsigned) << std::endl;
	std::cout << "unsigned long:  \t\t" << sizeof(unsigned long) << std::endl;
	std::cout << "unsigned long long:  \t\t" << sizeof(unsigned long long) << std::endl;
	std::cout << std::endl;
	std::cout << "int8_t:  \t\t\t" << sizeof(int8_t) << std::endl;
	std::cout << "int16_t:  \t\t\t" << sizeof(int16_t) << std::endl;
	std::cout << "int32_t:  \t\t\t" << sizeof(int32_t) << std::endl;
	std::cout << "int64_t:  \t\t\t" << sizeof(int64_t) << std::endl;
	std::cout << std::endl;
	std::cout << "uint8_t:  \t\t\t" << sizeof(uint8_t) << std::endl;
	std::cout << "uint16_t:  \t\t\t" << sizeof(uint16_t) << std::endl;
	std::cout << "uint32_t:  \t\t\t" << sizeof(uint32_t) << std::endl;
	std::cout << "uint64_t:  \t\t\t" << sizeof(uint64_t) << std::endl;
	std::cout << std::endl;
	std::cout << "__m128i:  \t\t\t" << sizeof(__m128i) << std::endl;
	std::cout << "__m256i:  \t\t\t" << sizeof(__m256i) << std::endl;
	std::cout << std::endl;
	std::cout << "void*:  \t\t\t" << sizeof(void*) << std::endl;
	std::cout << "uintptr_t:  \t\t\t" << sizeof(uintptr_t) << std::endl;
	std::cout << "size_t:  \t\t\t" << sizeof(size_t) << std::endl;
}

void xmem::common::print_compile_time_options() {
	std::cout << std::endl;
	std::cout << "This binary was built for the following OS and architecture capabilities: " << std::endl;
#ifdef _WIN32
	std::cout << "Win32" << std::endl;
#endif
#ifdef _WIN64
	std::cout << "Win64" << std::endl;
#endif
#ifdef ARCH_INTEL_X86
	std::cout << "ARCH_INTEL_X86" << std::endl;
#endif
#ifdef ARCH_INTEL_X86_64
	std::cout << "ARCH_INTEL_X86_64" << std::endl;
#endif
#ifdef ARCH_INTEL_X86_64_SSE2
	std::cout << "ARCH_INTEL_X86_64_SSE2" << std::endl;
#endif
#ifdef ARCH_INTEL_X86_64_AVX
	std::cout << "ARCH_INTEL_X86_64_AVX" << std::endl;
#endif
#ifdef ARCH_INTEL_X86_64_AVX2
	std::cout << "ARCH_INTEL_X86_64_AVX2" << std::endl;
#endif
#ifdef ARCH_AMD64
	std::cout << "ARCH_AMD64" << std::endl;
#endif
#ifdef ARCH_ARM
	std::cout << "ARCH_ARM" << std::endl;
#endif
	std::cout << std::endl;
	std::cout << "This binary was built with the following compile-time options:" << std::endl;
#ifdef VERBOSE
	std::cout << "VERBOSE" << std::endl;
#endif
#ifdef NDEBUG
	std::cout << "NDEBUG" << std::endl;
#endif
#ifdef USE_ALL_NUMA_NODES
	std::cout << "USE_ALL_NUMA_NODES" << std::endl;
#endif
#ifdef MULTITHREADING_ENABLE
	std::cout << "MULTITHREADING_ENABLE" << std::endl;
#endif
#ifdef USE_QPC_TIMER
	std::cout << "USE_QPC_TIMER" << std::endl;
#endif
#ifdef USE_TSC_TIMER
	std::cout << "USE_TSC_TIMER" << std::endl;
#endif
#ifdef USE_LARGE_PAGES
	std::cout << "USE_LARGE_PAGES" << std::endl;
#endif
#ifdef USE_TIME_BASED_BENCHMARKS
	std::cout << "USE_TIME_BASED_BENCHMARKS" << std::endl;
#endif
#ifdef BENCHMARK_DURATION_SEC
	std::cout << "BENCHMARK_DURATION_SEC = " << BENCHMARK_DURATION_SEC << std::endl; //This must be defined
#endif
#ifdef THROUGHPUT_BENCHMARK_BYTES_PER_PASS
	std::cout << "THROUGHPUT_BENCHMARK_BYTES_PER_PASS == " << THROUGHPUT_BENCHMARK_BYTES_PER_PASS << std::endl;
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
	std::cout << "USE_SIZE_BASED_BENCHMARKS" << std::endl;
#endif
#ifdef USE_PASSES_CURVE_1
	std::cout << "USE_PASSES_CURVE_1" << std::endl;
#endif
#ifdef USE_PASSES_CURVE_2
	std::cout << "USE_PASSES_CURVE_2" << std::endl;
#endif
#ifdef USE_CHUNK_32b
	std::cout << "USE_CHUNK_32b" << std::endl;
#endif
#ifdef USE_CHUNK_64b
	std::cout << "USE_CHUNK_64b" << std::endl;
#endif
#ifdef USE_CHUNK_128b
	std::cout << "USE_CHUNK_128b" << std::endl;
#endif
#ifdef USE_CHUNK_256b
	std::cout << "USE_CHUNK_256b" << std::endl;
#endif
#ifdef USE_THROUGHPUT_SEQUENTIAL_PATTERN
	std::cout << "USE_THROUGHPUT_SEQUENTIAL_PATTERN" << std::endl;
#endif
#ifdef USE_THROUGHPUT_RANDOM_PATTERN
	std::cout << "USE_THROUGHPUT_RANDOM_PATTERN" << std::endl;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
	std::cout << "USE_THROUGHPUT_FORW_STRIDE_1" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
	std::cout << "USE_THROUGHPUT_REV_STRIDE_1" << std::endl;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
	std::cout << "USE_THROUGHPUT_FORW_STRIDE_2" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
	std::cout << "USE_THROUGHPUT_REV_STRIDE_2" << std::endl;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
	std::cout << "USE_THROUGHPUT_FORW_STRIDE_4" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
	std::cout << "USE_THROUGHPUT_REV_STRIDE_4" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
	std::cout << "USE_THROUGHPUT_FORW_STRIDE_8" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
	std::cout << "USE_THROUGHPUT_REV_STRIDE_8" << std::endl;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
	std::cout << "USE_THROUGHPUT_FORW_STRIDE_16" << std::endl;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
	std::cout << "USE_THROUGHPUT_REV_STRIDE_16" << std::endl;
#endif
#ifdef USE_THROUGHPUT_READS
	std::cout << "USE_THROUGHPUT_READS" << std::endl;
#endif
#ifdef USE_THROUGHPUT_WRITES
	std::cout << "USE_THROUGHPUT_WRITES" << std::endl;
#endif
#ifdef USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN
	std::cout << "USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN" << std::endl;
#endif
#ifdef USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN
	std::cout << "USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN" << std::endl;
#endif
#ifdef POWER_SAMPLING_PERIOD_SEC
	std::cout << "POWER_SAMPLING_PERIOD_SEC == " << POWER_SAMPLING_PERIOD_SEC << std::endl;
#endif
	std::cout << std::endl;
}

void xmem::common::test_timers() {
	std::cout << std::endl << "Testing timers..." << std::endl;
#ifdef USE_QPC_TIMER
	xmem::timers::win::QPCTimer qpc;
	std::cout << "Reported QPC timer frequency: " << qpc.get_ticks_per_sec() << " Hz == " << (double)(qpc.get_ticks_per_sec()) / (1e6) << " MHz" << std::endl;
	std::cout << "Derived QPC timer ns per tick: " << qpc.get_ns_per_tick() << std::endl;
#endif
#ifdef USE_TSC_TIMER
	xmem::timers::x86_64::TSCTimer tsc;
	std::cout << "Calculated TSC timer frequency: " << tsc.get_ticks_per_sec() << " Hz == " << (double)(tsc.get_ticks_per_sec()) / (1e6) << " MHz" << std::endl;
	std::cout << "Derived TSC timer ns per tick: " << tsc.get_ns_per_tick() << std::endl;
#endif
	std::cout << std::endl;
}
	
void xmem::common::test_thread_affinities() {
	std::cout << std::endl << "Testing thread affinities..." << std::endl;
	bool success = false;
	for (uint32_t cpu = 0; cpu < xmem::common::g_num_logical_cpus; cpu++) {
		std::cout << "Locking to logical CPU " << cpu << "...";
		success = lock_thread_to_cpu(cpu);
		std::cout << (success ? "Pass" : "FAIL");
		std::cout << "      Unlocking" << "...";
		success = unlock_thread_to_cpu();
		std::cout << (success ? "Pass" : "FAIL");
		std::cout << std::endl;
	}
}

bool xmem::common::lock_thread_to_numa_node(uint32_t numa_node) {
	return lock_thread_to_cpu(cpu_id_in_numa_node(numa_node, 0)); //get the first CPU in the node if of interest
}

bool xmem::common::unlock_thread_to_numa_node() {
	return unlock_thread_to_cpu();
}

bool xmem::common::lock_thread_to_cpu(uint32_t cpu_id) {
#ifdef _WIN32
	HANDLE tid = GetCurrentThread();
	if (tid == 0)
		return false;
	else {
		DWORD_PTR threadAffinityMask = 1 << cpu_id;
		DWORD_PTR prev_mask = SetThreadAffinityMask(tid, threadAffinityMask); //enable only 1 CPU
		if (prev_mask == 0)
			return false;
	}
	return true;
#endif
#ifdef __unix__
	cpu_set_t cpus;
	CPU_ZERO(&cpus);
	CPU_SET(static_cast<int32_t>(cpu_id), &cpus);

	pthread_t tid = pthread_self();
	return (!pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpus));
#endif
}

bool xmem::common::unlock_thread_to_cpu() {
#ifdef _WIN32
	HANDLE tid = GetCurrentThread();
	if (tid == 0)
		return false;
	else {
		DWORD_PTR threadAffinityMask = static_cast<DWORD_PTR>(-1);
		DWORD_PTR prev_mask = SetThreadAffinityMask(tid, threadAffinityMask); //enable all CPUs
		if (prev_mask == 0)
			return false;
	}
	return true;
#endif
#ifdef __unix__
	pthread_t tid = pthread_self();
	cpu_set_t cpus;
	CPU_ZERO(&cpus);

	if (pthread_getaffinity_np(tid, sizeof(cpu_set_t), &cpus)) //failure
		return false;
	
	int32_t total_num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	for (int32_t c = 0; c < total_num_cpus; c++)
		CPU_SET(c, &cpus);

	return (!pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpus));
#endif
}
		
int32_t xmem::common::cpu_id_in_numa_node(uint32_t numa_node, uint32_t cpu_in_node) {
	int32_t cpu_id = -1;
	uint32_t rank_in_node = 0;
#ifdef _WIN32
	uint64_t processorMask = 0;
	GetNumaNodeProcessorMask(numa_node, &processorMask);
	//Select Nth CPU in the node
	uint32_t shifts = 0;
	uint64_t shiftmask = processorMask;
	bool done = false;
	while (!done && shifts < sizeof(shiftmask)*8) {
		if ((shiftmask & 0x1) == 0x1) { //current CPU is in the NUMA node
			if (cpu_in_node == rank_in_node) { //found the CPU of interest in the NUMA node
				cpu_id = static_cast<int32_t>(shifts);
				done = true;
			}
			rank_in_node++;
		}
		shiftmask = shiftmask >> 1; //shift right by one to examine next CPU
		shifts++;
	}
#endif
#ifdef __unix__
	struct bitmask bm;
	if (numa_node_to_cpus(static_cast<int32_t>(numa_node), &bm)) //error
		return -1;
	
	//Select Nth CPU in the node
	for (uint64_t i = 0; i < bm.size; i++) {
		if (numa_bitmask_isbitset(&bm, i)) {
			if (cpu_in_node == rank_in_node) { //found the CPU of interest in the NUMA node
				cpu_id = i;
				return cpu_id;
			}
			rank_in_node++;
		}
	}

	return cpu_id;
#endif
}
	
size_t xmem::common::compute_number_of_passes(size_t working_set_size_KB) {
	size_t passes = 0;
#ifdef USE_PASSES_CURVE_1
	passes = 65536 / working_set_size_KB;
#else
#ifdef USE_PASSES_CURVE_2
	passes = (4*2097152) / working_set_size_KB^2;
#endif
#endif
	if (passes < 1)
		passes = 1;
	return passes;
}
