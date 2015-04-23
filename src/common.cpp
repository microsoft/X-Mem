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
#include <Timer.h>

//Libraries
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#ifdef __gnu_linux__
#include <unistd.h>
#include <pthread.h>
#ifdef HAS_NUMA
#include <numa.h>
#endif
#include <fstream> //for std::ifstream
#include <vector> //for std::vector
#include <algorithm> //for std::find

#ifdef ARCH_INTEL
#include <immintrin.h> //for timer
#include <cpuid.h> //for timer
#include <x86intrin.h> //for timer
#endif

#ifdef USE_POSIX_TIMER
#include <time.h>
#endif

extern "C" {
#include <hugetlbfs.h> //for getting huge page size
}
#endif

namespace xmem {
	bool g_verbose = false; /**< If true, be more verbose with console reporting. */
	size_t g_page_size; /**< Default page size on the system, in bytes. */
	size_t g_large_page_size; /**< Large page size on the system, in bytes. */
	uint32_t g_num_nodes; /**< Number of NUMA nodes in the system. */
	uint32_t g_num_logical_cpus; /**< Number of logical CPU cores in the system. This may be different than physical CPUs, e.g. simultaneous multithreading. */
	uint32_t g_num_physical_cpus; /**< Number of physical CPU cores in the system. */
	uint32_t g_num_physical_packages; /**< Number of physical CPU packages in the system. Generally this is the same as number of NUMA nodes, unless UMA emulation is done in hardware. */
	uint32_t g_total_l1_caches; /**< Total number of L1 caches in the system. */
	uint32_t g_total_l2_caches; /**< Total number of L2 caches in the system. */
	uint32_t g_total_l3_caches; /**< Total number of L3 caches in the system. */
	uint32_t g_total_l4_caches; /**< Total number of L4 caches in the system. */
	uint32_t g_starting_test_index; /**< Numeric identifier for the first benchmark test. */
	uint32_t g_test_index; /**< Numeric identifier for the current benchmark test. */
	tick_t g_ticks_per_ms; /**< Timer ticks per ms. */
	double g_ns_per_tick; /**< Nanoseconds per timer tick. */
};

using namespace xmem;

void xmem::print_types_report() {
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
#ifdef ARCH_64BIT
	std::cout << "int64_t:  \t\t\t" << sizeof(int64_t) << std::endl;
#endif
	std::cout << std::endl;
	std::cout << "uint8_t:  \t\t\t" << sizeof(uint8_t) << std::endl;
	std::cout << "uint16_t:  \t\t\t" << sizeof(uint16_t) << std::endl;
	std::cout << "uint32_t:  \t\t\t" << sizeof(uint32_t) << std::endl;
#ifdef ARCH_64BIT
	std::cout << "uint64_t:  \t\t\t" << sizeof(uint64_t) << std::endl;
#endif
	std::cout << std::endl;
	std::cout << "Word32_t:  \t\t\t" << sizeof(Word32_t) << std::endl;
#ifdef HAS_WORD_64
	std::cout << "Word64_t:  \t\t\t" << sizeof(Word64_t) << std::endl;
#endif
#ifdef HAS_WORD_128
	std::cout << "Word128_t: \t\t\t" << sizeof(Word128_t) << std::endl;
#endif
#ifdef HAS_WORD_256
	std::cout << "Word256_t: \t\t\t" << sizeof(Word256_t) << std::endl;
#endif
	std::cout << std::endl;
	std::cout << "void*:  \t\t\t" << sizeof(void*) << std::endl;
	std::cout << "uintptr_t:  \t\t\t" << sizeof(uintptr_t) << std::endl;
	std::cout << "size_t:  \t\t\t" << sizeof(size_t) << std::endl;
}

void xmem::print_compile_time_options() {
	std::cout << std::endl;
	std::cout << "This binary was built for the following OS and architecture capabilities: " << std::endl;
#ifdef _WIN32
	std::cout << "Win32" << std::endl;
#endif
#ifdef _WIN64
	std::cout << "Win64" << std::endl;
#endif
#ifdef __gnu_linux__
	std::cout <<  "GNU/Linux" << std::endl;
#endif
#ifdef ARCH_INTEL
	std::cout << "ARCH_INTEL" << std::endl;
#endif
#ifdef ARCH_INTEL_X86
	std::cout << "ARCH_INTEL_X86" << std::endl;
#endif
#ifdef ARCH_INTEL_X86_64
	std::cout << "ARCH_INTEL_X86_64" << std::endl;
#endif
#ifdef ARCH_INTEL_SSE
	std::cout << "ARCH_INTEL_SSE" << std::endl;
#endif
#ifdef ARCH_INTEL_SSE2
	std::cout << "ARCH_INTEL_SSE2" << std::endl;
#endif
#ifdef ARCH_INTEL_SSE3
	std::cout << "ARCH_INTEL_SSE3" << std::endl;
#endif
#ifdef ARCH_INTEL_AVX
	std::cout << "ARCH_INTEL_AVX" << std::endl;
#endif
#ifdef ARCH_INTEL_AVX2
	std::cout << "ARCH_INTEL_AVX2" << std::endl;
#endif
#ifdef ARCH_AMD64
	std::cout << "ARCH_AMD64" << std::endl;
#endif
#ifdef ARCH_ARM
	std::cout << "ARCH_ARM" << std::endl;
#endif
#ifdef ARCH_ARM_64
	std::cout << "ARCH_ARM_64" << std::endl;
#endif
#ifdef ARCH_ARM_V7
	std::cout << "ARCH_ARM_V7" << std::endl;
#endif
#ifdef ARCH_ARM_V8
	std::cout << "ARCH_ARM_V8" << std::endl;
#endif
#ifdef ARCH_ARM_VFP_V3
	std::cout << "ARCH_ARM_VFP_V3" << std::endl;
#endif
#ifdef ARCH_ARM_VFP_V4
	std::cout << "ARCH_ARM_VFP_V4" << std::endl;
#endif
#ifdef ARCH_ARM_NEON
	std::cout << "ARCH_ARM_NEON" << std::endl;
#endif
#ifdef ARCH_64BIT
	std::cout << "ARCH_64BIT" << std::endl;
#endif
#ifdef HAS_NUMA
	std::cout << "HAS_NUMA" << std::endl;
#endif
#ifdef HAS_WORD_64
	std::cout << "HAS_WORD_64" << std::endl;
#endif
#ifdef HAS_WORD_128
	std::cout << "HAS_WORD_128" << std::endl;
#endif
#ifdef HAS_WORD_256
	std::cout << "HAS_WORD_256" << std::endl;
#endif
	std::cout << std::endl;
	std::cout << "This binary was built with the following compile-time options:" << std::endl;
#ifdef NDEBUG
	std::cout << "NDEBUG" << std::endl;
#endif
#ifdef USE_OS_TIMER
	std::cout << "USE_OS_TIMER" << std::endl;
#endif
#ifdef USE_HW_TIMER
	std::cout << "USE_HW_TIMER" << std::endl;
#endif
#ifdef USE_QPC_TIMER
	std::cout << "USE_QPC_TIMER" << std::endl;
#endif
#ifdef USE_POSIX_TIMER
	std::cout << "USE_POSIX_TIMER" << std::endl;
#endif
#ifdef USE_TSC_TIMER
	std::cout << "USE_TSC_TIMER" << std::endl;
#endif
	//TODO: ARM timer
#ifdef BENCHMARK_DURATION_SEC
	std::cout << "BENCHMARK_DURATION_SEC = " << BENCHMARK_DURATION_SEC << std::endl; //This must be defined
#endif
#ifdef THROUGHPUT_BENCHMARK_BYTES_PER_PASS
	std::cout << "THROUGHPUT_BENCHMARK_BYTES_PER_PASS == " << THROUGHPUT_BENCHMARK_BYTES_PER_PASS << std::endl;
#endif
#ifdef POWER_SAMPLING_PERIOD_SEC
	std::cout << "POWER_SAMPLING_PERIOD_MS == " << POWER_SAMPLING_PERIOD_MS << std::endl;
#endif
#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK
	std::cout << "EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK" << std::endl;
#endif
#ifdef EXT_STREAM_BENCHMARK
	std::cout << "EXT_STREAM_BENCHMARK" << std::endl;
#endif
	std::cout << std::endl;
}

void xmem::setup_timer() {
	std::cout << "Initializing timer...";

	Timer timer;
	g_ticks_per_ms = timer.get_ticks_per_ms();
	g_ns_per_tick = timer.get_ns_per_tick();

	std::cout << "done" << std::endl;
}

void xmem::report_timer() {
	std::cout << "Calculated timer frequency: " << g_ticks_per_ms * 1000 << " Hz == " << (double)(g_ticks_per_ms*1000) / (1e6) << " MHz" << std::endl;
	std::cout << "Derived timer ns per tick: " << g_ns_per_tick << std::endl;
	std::cout << std::endl;
}
	
void xmem::test_thread_affinities() {
	std::cout << std::endl << "Testing thread affinities..." << std::endl;
	bool success = false;
	for (uint32_t cpu = 0; cpu < g_num_logical_cpus; cpu++) {
		std::cout << "Locking to logical CPU " << cpu << "...";
		success = lock_thread_to_cpu(cpu);
		std::cout << (success ? "Pass" : "FAIL");
		std::cout << "      Unlocking" << "...";
		success = unlock_thread_to_cpu();
		std::cout << (success ? "Pass" : "FAIL");
		std::cout << std::endl;
	}
}

bool xmem::lock_thread_to_numa_node(uint32_t numa_node) {
	return lock_thread_to_cpu(cpu_id_in_numa_node(numa_node, 0)); //get the first CPU in the node if of interest
}

bool xmem::unlock_thread_to_numa_node() {
	return unlock_thread_to_cpu();
}

bool xmem::lock_thread_to_cpu(uint32_t cpu_id) {
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
#ifdef __gnu_linux__
	cpu_set_t cpus;
	CPU_ZERO(&cpus);
	CPU_SET(static_cast<int32_t>(cpu_id), &cpus);

	pthread_t tid = pthread_self();
	return (!pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpus));
#endif
}

bool xmem::unlock_thread_to_cpu() {
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
#ifdef __gnu_linux__
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
	
int32_t xmem::cpu_id_in_numa_node(uint32_t numa_node, uint32_t cpu_in_node) {
#ifndef HAS_NUMA
	if (numa_node != 0) {
		std::cerr << "WARNING: NUMA is not supported on this X-Mem build." << std::endl;
		return -1;
	}

	return cpu_in_node;
#endif

#ifdef HAS_NUMA
	int32_t cpu_id = -1;
	uint32_t rank_in_node = 0;
#ifdef _WIN32
	ULONGLONG processorMask = 0;
	GetNumaNodeProcessorMask(numa_node, &processorMask);
	//Select Nth CPU in the node
	uint32_t shifts = 0;
	ULONGLONG shiftmask = processorMask;
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
#ifdef __gnu_linux__
	struct bitmask *bm_ptr = numa_allocate_cpumask();
	if (!bm_ptr) {
		std::cerr << "WARNING: Failed to allocate a bitmask for loading NUMA information." << std::endl;
		return -1;
	}
	if (numa_node_to_cpus(static_cast<int32_t>(numa_node), bm_ptr)) //error
		return -1;

	//Select Nth CPU in the node
	for (uint32_t i = 0; i < bm_ptr->size; i++) {
		if (numa_bitmask_isbitset(bm_ptr, i) == 1) {
			if (cpu_in_node == rank_in_node) { //found the CPU of interest in the NUMA node
				cpu_id = i;
				if (bm_ptr)
					free(bm_ptr);
				return cpu_id;
			}
			rank_in_node++;
		}
	}
	
	if (bm_ptr)
		free(bm_ptr);
#endif
	return cpu_id;
#endif
}
	
void xmem::init_globals() {
	//Initialize global variables to defaults.
	g_verbose = false;
	g_num_nodes = DEFAULT_NUM_NODES;
	g_num_physical_packages = DEFAULT_NUM_PHYSICAL_PACKAGES;
	g_num_physical_cpus = DEFAULT_NUM_PHYSICAL_CPUS;
	g_num_logical_cpus = DEFAULT_NUM_LOGICAL_CPUS;
	g_total_l1_caches = DEFAULT_NUM_L1_CACHES;
	g_total_l2_caches = DEFAULT_NUM_L2_CACHES;
	g_total_l3_caches = DEFAULT_NUM_L3_CACHES;
	g_total_l4_caches = DEFAULT_NUM_L4_CACHES;
	g_page_size = DEFAULT_PAGE_SIZE;
	g_large_page_size = DEFAULT_LARGE_PAGE_SIZE; 

	g_ticks_per_ms = 0;
	g_ns_per_tick = 0;
}

int32_t xmem::query_sys_info() {
	//Windows only: get logical processor information data structures from OS
#ifdef _WIN32
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION curr = NULL;
	DWORD len = 0;
	DWORD offset = 0;
	DWORD retval = 0;
	bool done = false;
	retval = GetLogicalProcessorInformation(buffer, &len); //this will fail because buffer is not yet allocated.
	buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(len));
	if (!buffer) {
		std::cerr << "WARNING: Failed to allocate memory for querying system information." << std::endl;
		return -1;
	}
	retval = GetLogicalProcessorInformation(buffer, &len); //try again
#endif

#ifdef __gnu_linux__
	std::ifstream in;
	in.open("/proc/cpuinfo");
	char line[512];
	uint32_t id = 0;
#endif

	//Get NUMA info
#ifdef _WIN32
	curr = buffer;
	offset = 0;
	while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= len) {
		if (curr->Relationship == RelationNumaNode)
			g_num_nodes++;
		else if (curr->Relationship == RelationProcessorPackage)
			g_num_physical_packages++;
		curr++;
		offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	}
#endif
#ifdef __gnu_linux__
#ifdef HAS_NUMA
	if (numa_available() == -1) { //Check that NUMA is available.
		std::cerr << "WARNING: NUMA API is not available on this system." << std::endl;
		return -1;
	}

	//Get number of nodes. This is easy.
	g_num_nodes = numa_max_node()+1;
#endif
#ifndef HAS_NUMA //special case
	g_num_nodes = 1;
#endif

	//Get number of physical packages. This is somewhat convoluted, but not sure of a better way on Linux. Technically there could be on-chip NUMA, so...
	std::vector<uint32_t> phys_package_ids;
	while (!in.eof()) {
		in.getline(line, 512, '\n');

		// the line with CPU feature flags may exceed 512 B, leaving the stream in an error state and the remainder of the line unread
		if(in.fail() && in.gcount() == 511) {
			// clean up the stream
			in.clear();
			// eat the remaining line
			in.ignore(std::numeric_limits<std::streamsize>::max(),
				'\n');
		}

		std::string line_string(line);
		if (line_string.find("physical id") != std::string::npos) {
			sscanf(line, "physical id\t\t\t: %u", &id);
			if (std::find(phys_package_ids.begin(), phys_package_ids.end(), id) == phys_package_ids.end()) //have not seen this physical processor yet
				phys_package_ids.push_back(id); //add to list
		}
	}
	g_num_physical_packages = phys_package_ids.size();
#endif

	//Get number of CPUs
#ifdef _WIN32
	curr = buffer;
	offset = 0;
	while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= len) {
		if (curr->Relationship == RelationProcessorCore) {
			DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
			DWORD bitSetCount = 0;
			ULONG_PTR bitMask = curr->ProcessorMask;
			ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
			DWORD i;
			
			for (i = 0; i <= LSHIFT; ++i)
			{
				bitSetCount += ((bitMask & bitTest)?1:0);
				bitTest/=2;
			}

			g_num_logical_cpus += bitSetCount;
			g_num_physical_cpus++;
		}
		curr++;
		offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	}
#endif
#ifdef __gnu_linux__
	//Get number of logical CPUs
	g_num_logical_cpus = sysconf(_SC_NPROCESSORS_ONLN); //This isn't really portable -- requires glibc extensions to sysconf()

	//Get number of physical CPUs. This is somewhat convoluted, but not sure of a better way on Linux. I don't want to assume anything about HyperThreading-like things.
	std::vector<uint32_t> core_ids;
	while (!in.eof()) {
		in.getline(line, 512, '\n');
		
		// the line with CPU feature flags may exceed 512 B, leaving the stream in an error state and the remainder of the line unread
		if(in.fail() && in.gcount() == 511) {
			// clean up the stream
			in.clear();
			// eat the remaining line
			in.ignore(std::numeric_limits<std::streamsize>::max(),
				'\n');
		}

		std::string line_string(line);
		if (line_string.find("core id") != std::string::npos) {
			sscanf(line, "core id\t\t\t: %u", &id);
			if (std::find(core_ids.begin(), core_ids.end(), id) == core_ids.end()) //have not seen this physical core yet
				core_ids.push_back(id); //add to list
		}
	}
	g_num_physical_cpus = core_ids.size() * g_num_physical_packages; //FIXME: currently this assumes each processor package has an equal number of cores. This may not be true in general! Need more complicated /proc/cpuinfo parsing.
#endif

	//Get number of caches
#ifdef _WIN32
	curr = buffer;
	offset = 0;
	while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= len) {
		if (curr->Relationship == RelationCache) {
			switch (curr->Cache.Level) {
				case 1:
					g_total_l1_caches++;
					break;
				case 2:
					g_total_l2_caches++;
					break;
				case 3:
					g_total_l3_caches++;
					break;
				case 4:
					g_total_l4_caches++;
					break;
				default:
					std::cerr << "WARNING: Unknown cache level detected in system information." << std::endl;
					break;
			}
		}
		curr++;
		offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	}
#endif
#ifdef __gnu_linux__
	//FIXME: guessing number of caches on GNU/Linux, not sure how to get correct answer. This does not affect X-Mem functionality, however.
	g_total_l1_caches = g_num_physical_cpus; 
	g_total_l2_caches = g_num_physical_cpus; 
	g_total_l3_caches = g_num_physical_packages; 
	g_total_l4_caches = 0; 
#endif

	//Get page size
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	DWORD pgsz = sysinfo.dwPageSize;
	g_page_size = pgsz;
	g_large_page_size = GetLargePageMinimum();
#endif
#ifdef __gnu_linux__
	g_page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
	g_large_page_size = gethugepagesize(); 
	in.close();
#endif

#ifdef _WIN32
	if (buffer)
		free(buffer);
#endif

	return 0;
}

void xmem::report_sys_info() {
	
	std::cout << "done" << std::endl;
	std::cout << "Number of NUMA nodes: " << g_num_nodes << std::endl;
	std::cout << "Number of physical processor packages: " << g_num_physical_packages << std::endl;
	std::cout << "Number of physical processor cores: " << g_num_physical_cpus << std::endl;
	std::cout << "Number of logical processor cores: " << g_num_logical_cpus << std::endl;
	std::cout << "Number of processor L1/L2/L3/L4 caches: " 
		<< g_total_l1_caches
		<< "/"
		<< g_total_l2_caches
		<< "/" 
		<< g_total_l3_caches
		<< "/"
		<< g_total_l4_caches
#ifdef __gnu_linux__
		<< " (guesses)"
#endif
		<< std::endl; 
	std::cout << "Regular page size: " << g_page_size << " B" << std::endl;
	std::cout << "Large page size: " << g_large_page_size << " B" << std::endl;
}

tick_t xmem::start_timer() {
#ifdef USE_TSC_TIMER
#ifdef _WIN32
	int32_t dontcare[4];
	__cpuid(dontcare, 0); //Serializing instruction. This forces all previous instructions to finish
	return __rdtsc(); //Get clock tick
#endif
#ifdef __gnu_linux__
	volatile int32_t dc0 = 0;
	volatile int32_t dc1, dc2, dc3, dc4;
	__cpuid(dc0, dc1, dc2, dc3, dc4); //Serializing instruction. This forces all previous instructions to finish
	return __rdtsc(); //Get clock tick

	/*
	uint32_t low, high;
	__asm__ __volatile__ (
		"cpuid\n\t"
		"rdtsc\n\t"
		"mov %%eax, %0\n\t"
		"mov %%edx, %1\n\n"
		: "=r" (low), "=r" (high)
		: : "%rax", "%rbx", "%rcx", "%rdx");

	return ((static_cast<uint64_t>(high) << 32) | low);
	*/
#endif
#endif
	
	//TODO: ARM hardware timer

#ifdef USE_QPC_TIMER
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return static_cast<tick_t>(tmp.QuadPart);
#endif

#ifdef USE_POSIX_TIMER
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return static_cast<tick_t>(tp.tv_sec*1e9+tp.tv_nsec); //Return time in nanoseconds
#endif
}

tick_t xmem::stop_timer() {
	//TODO: ARM hardware timer
#ifdef USE_TSC_TIMER
#ifdef _WIN32
	tick_t tick;
	uint32_t filler;
	int32_t dontcare[4];
	tick = __rdtscp(&filler); //Get clock tick. This is a partially serializing instruction. All previous instructions must finish
	__cpuid(dontcare, 0); //Fully serializing instruction. We do this to prevent later instructions from being moved inside the timed section
	return tick;
#endif
#ifdef __gnu_linux__
	tick_t tick;
	uint32_t filler;
	volatile int32_t dc0 = 0;
	volatile int32_t dc1, dc2, dc3, dc4;
	tick = __rdtscp(&filler); //Get clock tick. This is a partially serializing instruction. All previous instructions must finish
	__cpuid(dc0, dc1, dc2, dc3, dc4); //Serializing instruction. This forces all previous instructions to finish
	return tick;
	
	/*
	uint32_t low, high;
	__asm__ __volatile__ (
		"rdtscp\n\t"
		"mov %%eax, %0\n\t"
		"mov %%edx, %1\n\t"
		"cpuid\n\t"
		: "=r" (low), "=r" (high)
		: : "%rax", "%rbx", "%rcx", "%rdx");
	return ((static_cast<uint64_t>(high) << 32) | low);
	*/
#endif
#endif
	
	//TODO: ARM hardware timer

#ifdef USE_QPC_TIMER
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return static_cast<tick_t>(tmp.QuadPart);
#endif
	
#ifdef USE_POSIX_TIMER
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return static_cast<tick_t>(tp.tv_sec*1e9+tp.tv_nsec); //Return time in nanoseconds
#endif
}

#ifdef _WIN32
bool xmem::boostSchedulingPriority(DWORD& originalPriorityClass, DWORD& originalPriority) {
	originalPriorityClass = GetPriorityClass(GetCurrentProcess()); 	
	originalPriority = GetThreadPriority(GetCurrentThread());
	SetPriorityClass(GetCurrentProcess(), 0x80); //HIGH_PRIORITY_CLASS
	SetThreadPriority(GetCurrentThread(), 15); //THREAD_PRIORITY_TIME_CRITICAL

	return true;
}
#endif

#ifdef __gnu_linux__
bool xmem::boostSchedulingPriority() {
	if (nice(-20) == EPERM)
		return false;
	return true;
}
#endif

#ifdef _WIN32
bool xmem::revertSchedulingPriority(DWORD originalPriorityClass, DWORD originalPriority) {
	SetPriorityClass(GetCurrentProcess(), originalPriorityClass);
	SetThreadPriority(GetCurrentThread(), originalPriority);
	return true;
}
#endif

#ifdef __gnu_linux__
bool xmem::revertSchedulingPriority() {
	if (nice(0) == EPERM)
		return false;
	return true;
}
#endif
