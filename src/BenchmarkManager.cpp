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
 * @brief Implementation file for the BenchmarkManager class.
 */

//Headers
#include <BenchmarkManager.h>
#include <common.h>
#include <Configurator.h>

#ifdef _WIN32
#include <win/win_common_third_party.h>
#include <win/WindowsDRAMPowerReader.h>
#endif

//Libraries
#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __gnu_linux__
#include <numa.h>
extern "C" {
#include <hugetlbfs.h> //for allocating and freeing huge pages
}
#endif

using namespace xmem::benchmark;
using namespace xmem::common;
using namespace xmem::config;

BenchmarkManager::BenchmarkManager(
	Configurator &config
	) :
	__config(config),
	__num_numa_nodes(g_num_nodes),
	__benchmark_num_numa_nodes(g_num_nodes),
	__mem_arrays(),
	__mem_array_lens(),
	__tp_benchmarks(),
	__lat_benchmarks(),
	__dram_power_readers(),
	__timer(),
	__results_file(),
	__built_throughput_benchmarks(false),
	__built_latency_benchmarks(false)
	{
	//Set up DRAM power measurement
	for (uint32_t i = 0; i < g_num_physical_packages; i++) { //FIXME: this assumes that each physical package has a DRAM power measurement capability
		std::string power_obj_name = static_cast<std::ostringstream*>(&(std::ostringstream() << "Socket " << i << " DRAM"))->str();
		
		//TODO: Implement derived PowerReaders for Linux systems.
		//FIXME: this is a bandaid for when the system is -really- NUMA, but has UMA emulation in hardware. In such a case, the number of physical packages may exceed number of NUMA nodes, but there are still two physical "nodes" of DRAM power to measure. On Windows, need a way of picking a CPU core from a physical processor package rather than from a NUMA node! Maybe this exists and I need to search harder. :)
#ifdef _WIN32
		if (i >= __num_numa_nodes) 
			__dram_power_readers.push_back(new power::win::WindowsDRAMPowerReader((g_num_logical_cpus / g_num_physical_packages)*i+2, POWER_SAMPLING_PERIOD_SEC, 1, power_obj_name, (g_num_logical_cpus / g_num_physical_packages)*i+2)); //Measure using 3rd logical CPU in the node. FIXME: this is a bandaid. not always going to work if there aren't at least 3 logical CPUs in any node!!
		else //Normal condition, # NUMA nodes equals # physical processor packages which is likely in most reasonable scenarios.
			__dram_power_readers.push_back(new power::win::WindowsDRAMPowerReader(cpu_id_in_numa_node(i,2), POWER_SAMPLING_PERIOD_SEC, 1, power_obj_name, cpu_id_in_numa_node(i,2))); //Measure using 3rd logical CPU in the node. FIXME: this is a bandaid. not always going to work if there aren't at least 3 logical CPUs in any node!!
#endif
	}

	//Build working memory regions
	__setupWorkingSets(__config.getWorkingSetSizePerThread());

	//Open results file
	if (__config.useOutputFile()) {
		__results_file.open(__config.getOutputFilename().c_str(), std::fstream::out);
		if (!__results_file.is_open()) {
			__config.setUseOutputFile(false);
			std::cerr << "WARNING: Failed to open " << __config.getOutputFilename() << " for writing! No results file will be generated." << std::endl;
		}

		//Generate file headers
		__results_file << "Test Name,Iterations,Working Set Size Per Thread (KB),NUMA Memory Node,NUMA CPU Node,Access Pattern,Read/Write Mix,Chunk Size (bits),Stride Size (chunks),Average Test Result,Test Result Units,";
		for (uint32_t i = 0; i < __dram_power_readers.size(); i++)  {
			if (__dram_power_readers[i] != NULL) {
				__results_file << __dram_power_readers[i]->name() << " Average Power (W),";
				__results_file << __dram_power_readers[i]->name() << " Peak Power (W),";
			} else {
				__results_file << "?? Average Power (W),";
				__results_file << "?? Peak Power (W),";
			}
		}
		__results_file << std::endl;
	}
}

BenchmarkManager::~BenchmarkManager() {
	//Free throughput benchmarks
	for (uint32_t i = 0; i < __tp_benchmarks.size(); i++)
		if (__tp_benchmarks[i] != nullptr)
			delete __tp_benchmarks[i];
	//Free latency benchmarks
	for (uint32_t i = 0; i < __lat_benchmarks.size(); i++)
		if (__lat_benchmarks[i] != nullptr)
			delete __lat_benchmarks[i];
	//Free memory arrays
	for (uint32_t i = 0; i < __mem_arrays.size(); i++)
		if (__mem_arrays[i] != nullptr) {
#ifdef _WIN32
			VirtualFreeEx(GetCurrentProcess(), __mem_arrays[i], 0, MEM_RELEASE);
#endif
#ifdef __gnu_linux__
			if (__config.useLargePages())
				free_huge_pages(__mem_arrays[i]);
			else
				numa_free(__mem_arrays[i], __mem_array_lens[i]); 
#endif
		}
	//Close results file
	if (__results_file.is_open())
		__results_file.close();
}

bool BenchmarkManager::runAll() {
	bool success = true;

	if (__config.throughputTestSelected())
		success = success && runThroughputBenchmarks();
	if (__config.latencyTestSelected())
		success = success && runLatencyBenchmarks();

	return success;
}

bool BenchmarkManager::runThroughputBenchmarks() {
	if (!__built_throughput_benchmarks)
		__buildThroughputBenchmarks();

	for (uint32_t i = 0; i < __tp_benchmarks.size(); i++) {
		__tp_benchmarks[i]->run(); 
		__tp_benchmarks[i]->report_results(); //to console
		
		//Write to results file if necessary
		if (__config.useOutputFile()) {
			__results_file << __tp_benchmarks[i]->getName() << ",";
			__results_file << __tp_benchmarks[i]->getIterations() << ",";
			__results_file << static_cast<uint64_t>(__tp_benchmarks[i]->getLen() / __config.getNumWorkerThreads() / KB) << ",";
			__results_file << __tp_benchmarks[i]->getMemNode() << ",";
			__results_file << __tp_benchmarks[i]->getCPUNode() << ",";
			pattern_mode_t pattern = __tp_benchmarks[i]->getPatternMode();
			switch (pattern) {
				case SEQUENTIAL:
					__results_file << "SEQUENTIAL" << ",";
					break;
				case RANDOM:
					__results_file << "RANDOM" << ",";
					break;
				default:
					__results_file << "UNKNOWN" << ",";
					break;
			}

			rw_mode_t rw_mode = __tp_benchmarks[i]->getRWMode();
			switch (rw_mode) {
				case READ:
					__results_file << "READ" << ",";
					break;
				case WRITE:
					__results_file << "WRITE" << ",";
					break;
				default:
					__results_file << "UNKNOWN" << ",";
					break;
			}

			chunk_size_t chunk_size = __tp_benchmarks[i]->getChunkSize();
			switch (chunk_size) {
				case CHUNK_32b:
					__results_file << "32" << ",";
					break;
				case CHUNK_64b:
					__results_file << "64" << ",";
					break;
				case CHUNK_128b:
					__results_file << "128" << ",";
					break;
				case CHUNK_256b:
					__results_file << "256" << ",";
					break;
				default:
					__results_file << "UNKNOWN" << ",";
					break;
			}

			__results_file << __tp_benchmarks[i]->getStrideSize() << ",";
			__results_file << __tp_benchmarks[i]->getAverageMetric() << ",";
			__results_file << "MB/s" << ",";
			if (__dram_power_readers.size() > 0) { //FIXME: bandaid
				for (uint32_t j = 0; j < g_num_physical_packages; j++) {
					__results_file << __tp_benchmarks[i]->getAverageDRAMPower(j) << ",";
					__results_file << __tp_benchmarks[i]->getPeakDRAMPower(j) << ",";
				}
			}
			__results_file << std::endl;
		}
	}

	if (g_verbose)
		std::cout << std::endl << "Done running throughput benchmarks." << std::endl;

	return true;
}

bool BenchmarkManager::runLatencyBenchmarks() {
	if (!__built_latency_benchmarks)
		__buildLatencyBenchmarks();

	for (uint32_t i = 0; i < __lat_benchmarks.size(); i++) {
		__lat_benchmarks[i]->run(); 
		__lat_benchmarks[i]->report_results(); //to console
		
		//Write to results file if necessary
		if (__config.useOutputFile()) {
			__results_file << __lat_benchmarks[i]->getName() << ",";
			__results_file << __lat_benchmarks[i]->getIterations() << ",";
			__results_file << static_cast<uint64_t>(__lat_benchmarks[i]->getLen() / KB) << ",";
			__results_file << __lat_benchmarks[i]->getMemNode() << ",";
			__results_file << __lat_benchmarks[i]->getCPUNode() << ",";
			__results_file << "RANDOM POINTER CHASING" << ","; 
			__results_file << "READ ONLY" << ","; 
			__results_file << "64" << ","; 
			__results_file << "N/A" << ",";
			__results_file << __lat_benchmarks[i]->getAverageMetric() << ",";
			__results_file << "ns/access" << ",";
			for (uint32_t j = 0; j < g_num_physical_packages; j++) {
				__results_file << __lat_benchmarks[i]->getAverageDRAMPower(j) << ",";
				__results_file << __lat_benchmarks[i]->getPeakDRAMPower(j) << ",";
			}
			__results_file << std::endl;
		}
	}

	if (g_verbose)
		std::cout << std::endl << "Done running latency benchmarks." << std::endl;

	return true;
}

void BenchmarkManager::__setupWorkingSets(size_t working_set_size) {
	//Allocate memory in each NUMA node to be tested
	if (!__config.isNUMAEnabled())
		__benchmark_num_numa_nodes = 1;

	__mem_arrays.resize(__benchmark_num_numa_nodes);
	__mem_array_lens.resize(__benchmark_num_numa_nodes);

	for (uint32_t numa_node = 0; numa_node < __benchmark_num_numa_nodes; numa_node++) {
		size_t allocation_size = 0;

		if (__config.useLargePages()) {
			size_t remainder = 0;
			//For large pages, working set size could be less than a single large page. So let's allocate the right amount of memory, which is the working set size rounded up to nearest large page, which could be more than we actually use.
			if (__config.getNumWorkerThreads() * working_set_size < g_large_page_size)
				allocation_size = g_large_page_size;
			else { 
				remainder = (__config.getNumWorkerThreads() * working_set_size) % g_large_page_size;
				allocation_size = (__config.getNumWorkerThreads() * working_set_size) + remainder;
			}
			
#ifdef _WIN32
			//Make sure we have necessary privileges
			HANDLE hToken;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
				std::cerr << "ERROR: Failed to open process token to adjust privileges! Did you remember to run in Administrator mode?" << std::endl;
				exit(-1);
			}
			if (!xmem::common::win::third_party::SetPrivilege(hToken,"SeLockMemoryPrivilege", true)) {
				std::cerr << "ERROR: Failed to adjust privileges to allow locking memory pages! Did you remember to run in Administrator mode?" << std::endl;
				exit(-1);
			}
			CloseHandle(hToken);
			
			__mem_arrays[numa_node] = VirtualAllocExNuma(GetCurrentProcess(), NULL, allocation_size, MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE, numa_node); //Windows NUMA allocation. Make the allocation one page bigger than necessary so that we can do alignment.
#endif
#ifdef __gnu_linux__
			__mem_arrays[numa_node] = get_huge_pages(allocation_size, GHP_DEFAULT); //TODO: hugetlbfs does not seem to be NUMA-aware. We may require NUMA awareness and huge pages to be mutually exclusive on Linux builds =( FIXME: I get segfaults in ThroughputBenchmark if allocation_size ends up requiring more than 1 huge page.
#endif
		} else { //Non-large pages (nominal case)
			//Under normal (not large-page) operation, working set size is a multiple of regular pages.
			allocation_size = __config.getNumWorkerThreads() * working_set_size + g_page_size; 
#ifdef _WIN32
			__mem_arrays[numa_node] = VirtualAllocExNuma(GetCurrentProcess(), NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, numa_node); //Windows NUMA allocation. Make the allocation one page bigger than necessary so that we can do alignment.
#endif
#ifdef __gnu_linux__
			numa_set_strict(1); //Enforce NUMA memory allocation to land on specified node or fail otherwise. Alternative node fallback is forbidden.
			__mem_arrays[numa_node] = numa_alloc_onnode(allocation_size, numa_node);
#endif
		}
		
		if (__mem_arrays[numa_node] != nullptr)
			__mem_array_lens[numa_node] = __config.getNumWorkerThreads() * working_set_size;
		else {
			std::cerr << "ERROR: Failed to allocate " << allocation_size << " B on NUMA node " << numa_node << " for " << __config.getNumWorkerThreads() << " worker threads." << std::endl;
			exit(-1);
		}

	 	if (g_verbose) {
			std::cout << std::endl;
			std::cout << "Virtual address for memory on NUMA node " << numa_node << ": " << __mem_arrays[numa_node];
		}

		//upwards alignment to page boundary
		uintptr_t mask;
		if (__config.useLargePages())
			mask = static_cast<uintptr_t>(g_large_page_size)-1;
		else
			mask = static_cast<uintptr_t>(g_page_size)-1; //e.g. 4095 bytes
		uintptr_t tmp_ptr = reinterpret_cast<uintptr_t>(__mem_arrays[numa_node]);
		uintptr_t aligned_addr = (tmp_ptr + mask) & ~mask; //add one page to the address, then truncate least significant bits of address to be page aligned.
		__mem_arrays[numa_node] = reinterpret_cast<void*>(aligned_addr); 

		if (g_verbose) {
			std::cout << " ---- ALIGNED ----> " << __mem_arrays[numa_node] << std::endl;
			std::cout << std::endl;
		}
	}
}

void BenchmarkManager::__buildThroughputBenchmarks() {
	if (g_verbose)  {
		std::cout << std::endl;
		std::cout << "Generating throughput benchmarks." << std::endl;
	}

	//Put the enumerations into vectors to make constructing benchmarks more loopable
	std::vector<chunk_size_t> chunks;
	if (__config.useChunk32b())
		chunks.push_back(CHUNK_32b); 
	if (__config.useChunk64b())
		chunks.push_back(CHUNK_64b); 
	if (__config.useChunk128b())
		chunks.push_back(CHUNK_128b); 
	if (__config.useChunk256b())
		chunks.push_back(CHUNK_256b); 

	std::vector<rw_mode_t> rws;
	if (__config.useReads())
		rws.push_back(READ);
	if (__config.useWrites())
		rws.push_back(WRITE);
	
	std::vector<int64_t> strides;
	if (__config.useStrideP1())
		strides.push_back(1);
	if (__config.useStrideN1())
		strides.push_back(-1);
	if (__config.useStrideP2())
		strides.push_back(2);
	if (__config.useStrideN2())
		strides.push_back(-2);
	if (__config.useStrideP4())
		strides.push_back(4);
	if (__config.useStrideN4())
		strides.push_back(-4);
	if (__config.useStrideP8())
		strides.push_back(8);
	if (__config.useStrideN8())
		strides.push_back(-8);
	if (__config.useStrideP16())
		strides.push_back(16);
	if (__config.useStrideN16())
		strides.push_back(-16);

	if (g_verbose)
		std::cout << std::endl;

	std::string benchmark_name;
	uint32_t i = 0;
	//Build throughput benchmarks. This is a humongous nest of for loops, but rest assured, the range of each loop should be small enough. The problem is we have many combinations to test.
	for (uint32_t mem_node = 0; mem_node < __benchmark_num_numa_nodes; mem_node++) { //iterate each memory NUMA node
		void* mem_array = __mem_arrays[mem_node];			
		size_t mem_array_len = __mem_array_lens[mem_node];

		for (uint32_t cpu_node = 0; cpu_node < __benchmark_num_numa_nodes; cpu_node++) { //iterate each CPU node

			//DO SEQUENTIAL/STRIDED TESTS
			if (__config.useSequentialAccessPattern()) {
				for (uint32_t rw_index = 0; rw_index < rws.size(); rw_index++) { //iterate read/write access types
					rw_mode_t rw = rws[rw_index];

					for (uint32_t chunk_index = 0; chunk_index < chunks.size(); chunk_index++) { //iterate different chunk sizes
						chunk_size_t chunk = chunks[chunk_index];

						for (uint32_t stride_index = 0; stride_index < strides.size(); stride_index++) {  //iterate different stride lengths
							int64_t stride = strides[stride_index];

							benchmark_name = static_cast<std::ostringstream*>(&(std::ostringstream() << "Test #" << g_test_index++ << " (Throughput)"))->str();
							__tp_benchmarks.resize(__tp_benchmarks.size()+1);
							try {
#ifdef USE_TIME_BASED_BENCHMARKS
								__tp_benchmarks[i] = new ThroughputBenchmark(mem_array, mem_array_len, __config.getIterationsPerTest(), chunk, cpu_node, mem_node, __config.getNumWorkerThreads(), benchmark_name, &__timer, __dram_power_readers, stride, SEQUENTIAL, rw);
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
								size_t passes_per_iteration = compute_number_of_passes((mem_array_len / __config.getNumWorkerThreads()) / KB);
								__tp_benchmarks[i] = new ThroughputBenchmark(mem_array, mem_array_len, __config.getIterationsPerTest(), passes_per_iteration, chunk, cpu_node, mem_node, __config.getNumWorkerThreads(), benchmark_name, &__timer, __dram_power_readers, stride, SEQUENTIAL, rw);
#endif
							} catch (...) { 
								std::cerr << "ERROR: Failed to build a ThroughputBenchmark! Terminating." << std::endl;
								exit(-1);
							}
							if (__tp_benchmarks[i] == nullptr)
								std::cerr << "ERROR: Failed to build a ThroughputBenchmark! Terminating." << std::endl;
							i++;
						}
					}
				}
			}
			
			if (__config.useRandomAccessPattern()) {
				//DO RANDOM TESTS
				for (uint32_t rw_index = 0; rw_index < rws.size(); rw_index++) { //iterate read/write access types
					rw_mode_t rw = rws[rw_index];

					for (uint32_t chunk_index = 0; chunk_index < chunks.size(); chunk_index++) { //iterate different chunk sizes
						chunk_size_t chunk = chunks[chunk_index];

						benchmark_name = static_cast<std::ostringstream*>(&(std::ostringstream() << "Test #" << g_test_index++))->str();
						__tp_benchmarks.resize(__tp_benchmarks.size()+1);
						try {
#ifdef USE_TIME_BASED_BENCHMARKS
							__tp_benchmarks[i] = new ThroughputBenchmark(mem_array, mem_array_len, __config.getIterationsPerTest(), chunk, cpu_node, mem_node, __config.getNumWorkerThreads(), benchmark_name, &__timer, __dram_power_readers, 0, RANDOM, rw);
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
							size_t passes_per_iteration = compute_number_of_passes((mem_array_len / __config.getNumWorkerThreads) / KB);
							__tp_benchmarks[i] = new ThroughputBenchmark(mem_array, mem_array_len, __config.getIterationsPerTest(), passes_per_iteration, chunk, cpu_node, mem_node, __config.getNumWorkerThreads(), benchmark_name, &__timer, __dram_power_readers, 0, RANDOM, rw);
#endif
						} catch (...) { 
							std::cerr << "ERROR: Failed to build a ThroughputBenchmark! Terminating." << std::endl;
							exit(-1);
						}
						if (__tp_benchmarks[i] == nullptr)
							std::cerr << "ERROR: Failed to build a ThroughputBenchmark! Terminating." << std::endl;
						i++;
					}
				}
			}
		}
	}
	
	__built_throughput_benchmarks = true;
}

void BenchmarkManager::__buildLatencyBenchmarks() {
	if (g_verbose) {
		std::cout << std::endl;
		std::cout << "Generating latency benchmarks." << std::endl;
	}

	std::string benchmark_name;
	uint32_t i = 0;

	for (uint32_t mem_node = 0; mem_node < __benchmark_num_numa_nodes; mem_node++) { //iterate each memory NUMA node
		void* mem_array = __mem_arrays[mem_node];			
		size_t mem_array_len = __mem_array_lens[mem_node];

		for (uint32_t cpu_node = 0; cpu_node < __benchmark_num_numa_nodes; cpu_node++) { //iterate each CPU node
			//Determine number of passes for each benchmark. This is working set size-dependent, to ensure the timed duration of each run is sufficiently long, but not too long.

			__lat_benchmarks.resize(__lat_benchmarks.size()+1);
			benchmark_name = static_cast<std::ostringstream*>(&(std::ostringstream() << "Test #" << g_test_index++ << " (Latency)"))->str();
			try {
#ifdef USE_TIME_BASED_BENCHMARKS
				//FIXME: number of worker threads issue. loaded latency vs unloaded latency
				__lat_benchmarks[i] = new LatencyBenchmark(mem_array, mem_array_len / __config.getNumWorkerThreads(), __config.getIterationsPerTest(), cpu_node, mem_node, 1, benchmark_name, &__timer, __dram_power_readers); 
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
				size_t passes_per_iteration = compute_number_of_passes(mem_array_len / KB) / 4;
				if (passes_per_iteration < 1)
					passes_per_iteration = 1;
				__lat_benchmarks[i] = new LatencyBenchmark(mem_array, mem_array_len / __config.getNumWorkerThreads(), __config.getIterationsPerTest(), passes_per_iteration, cpu_node, mem_node, 1, benchmark_name, &__timer, __dram_power_readers); 
#endif
			} catch (...) {
				std::cerr << "ERROR: Failed to build a LatencyBenchmark! Terminating." << std::endl;
				exit(-1);
			}
			if (__lat_benchmarks[i] == nullptr)
				std::cerr << "ERROR: Failed to build a LatencyBenchmark! Terminating." << std::endl;
			i++;
		}
	}

	__built_latency_benchmarks = true;
}
