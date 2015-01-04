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
 * @brief Implementation file for the ThroughputBenchmark class.
 */

//Headers
#include "ThroughputBenchmark.h"
#include <common/common.h>
#include "ThroughputBenchmarkWorker.h"
#include <benchmark/benchmark_kernels/benchmark_kernels.h>
#include <thread/Thread.h>
#include <thread/Runnable.h>

//Libraries
#include <iostream>
#include <algorithm> 
#include <assert.h>
#include <random>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <processthreadsapi.h>
#else
#error Windows is the only supported OS at this time.
#endif

using namespace xmem::benchmark;
using namespace xmem::benchmark::benchmark_kernels;
using namespace xmem::common;

ThroughputBenchmark::ThroughputBenchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t passes_per_iteration,
#endif
		chunk_size_t chunk_size,
		uint32_t cpu_node,
		uint32_t mem_node,
		uint32_t num_worker_threads,
		std::string name,
		xmem::timers::Timer *timer,
		std::vector<xmem::power::PowerReader*> dram_power_readers,
		int64_t stride_size,
		pattern_mode_t pattern_mode,
		rw_mode_t rw_mode
	) :
	Benchmark(
		mem_array,
		len,
		iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		passes_per_iteration,
#endif
		chunk_size,
		cpu_node,
		mem_node,
		num_worker_threads,
		name,
		timer,
		dram_power_readers
	),
	__stride_size(stride_size),
	__pattern_mode(pattern_mode),
	__rw_mode(rw_mode),
	__bench_fptr(nullptr),
	__dummy_fptr(nullptr)
	{ 
	switch (__pattern_mode) {
#ifdef USE_THROUGHPUT_SEQUENTIAL_PATTERN
		case SEQUENTIAL:
			switch (__rw_mode) {
#ifdef USE_THROUGHPUT_READS
				case READ:
					switch (_chunk_size) {
#ifdef USE_CHUNK_32b
						case CHUNK_32b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialRead_Word32;
									__dummy_fptr = &dummy_forwSequentialLoop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialRead_Word32;
									__dummy_fptr = &dummy_revSequentialLoop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Read_Word32;
									__dummy_fptr = &dummy_forwStride2Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Read_Word32;
									__dummy_fptr = &dummy_revStride2Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Read_Word32;
									__dummy_fptr = &dummy_forwStride4Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Read_Word32;
									__dummy_fptr = &dummy_revStride4Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Read_Word32;
									__dummy_fptr = &dummy_forwStride8Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Read_Word32;
									__dummy_fptr = &dummy_revStride8Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Read_Word32;
									__dummy_fptr = &dummy_forwStride16Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Read_Word32;
									__dummy_fptr = &dummy_revStride16Loop_Word32;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_64b
						case CHUNK_64b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialRead_Word64;
									__dummy_fptr = &dummy_forwSequentialLoop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialRead_Word64;
									__dummy_fptr = &dummy_revSequentialLoop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Read_Word64;
									__dummy_fptr = &dummy_forwStride2Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Read_Word64;
									__dummy_fptr = &dummy_revStride2Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Read_Word64;
									__dummy_fptr = &dummy_forwStride4Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Read_Word64;
									__dummy_fptr = &dummy_revStride4Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Read_Word64;
									__dummy_fptr = &dummy_forwStride8Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Read_Word64;
									__dummy_fptr = &dummy_revStride8Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Read_Word64;
									__dummy_fptr = &dummy_forwStride16Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Read_Word64;
									__dummy_fptr = &dummy_revStride16Loop_Word64;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_128b
						case CHUNK_128b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialRead_Word128;
									__dummy_fptr = &dummy_forwSequentialLoop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialRead_Word128;
									__dummy_fptr = &dummy_revSequentialLoop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Read_Word128;
									__dummy_fptr = &dummy_forwStride2Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Read_Word128;
									__dummy_fptr = &dummy_revStride2Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Read_Word128;
									__dummy_fptr = &dummy_forwStride4Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Read_Word128;
									__dummy_fptr = &dummy_revStride4Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Read_Word128;
									__dummy_fptr = &dummy_forwStride8Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Read_Word128;
									__dummy_fptr = &dummy_revStride8Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Read_Word128;
									__dummy_fptr = &dummy_forwStride16Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Read_Word128;
									__dummy_fptr = &dummy_revStride16Loop_Word128;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_256b
						case CHUNK_256b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialRead_Word256;
									__dummy_fptr = &dummy_forwSequentialLoop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialRead_Word256;
									__dummy_fptr = &dummy_revSequentialLoop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Read_Word256;
									__dummy_fptr = &dummy_forwStride2Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Read_Word256;
									__dummy_fptr = &dummy_revStride2Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Read_Word256;
									__dummy_fptr = &dummy_forwStride4Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Read_Word256;
									__dummy_fptr = &dummy_revStride4Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Read_Word256;
									__dummy_fptr = &dummy_forwStride8Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Read_Word256;
									__dummy_fptr = &dummy_revStride8Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Read_Word256;
									__dummy_fptr = &dummy_forwStride16Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Read_Word256;
									__dummy_fptr = &dummy_revStride16Loop_Word256;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif

						default:
							std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
							_obj_valid = false;
							break;
					}
					break;
#endif
#ifdef USE_THROUGHPUT_WRITES
				case WRITE:
					switch (_chunk_size) {
#ifdef USE_CHUNK_32b
						case CHUNK_32b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialWrite_Word32;
									__dummy_fptr = &dummy_forwSequentialLoop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialWrite_Word32;
									__dummy_fptr = &dummy_revSequentialLoop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Write_Word32;
									__dummy_fptr = &dummy_forwStride2Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Write_Word32;
									__dummy_fptr = &dummy_revStride2Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Write_Word32;
									__dummy_fptr = &dummy_forwStride4Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Write_Word32;
									__dummy_fptr = &dummy_revStride4Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Write_Word32;
									__dummy_fptr = &dummy_forwStride8Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Write_Word32;
									__dummy_fptr = &dummy_revStride8Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Write_Word32;
									__dummy_fptr = &dummy_forwStride16Loop_Word32;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Write_Word32;
									__dummy_fptr = &dummy_revStride16Loop_Word32;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_64b
						case CHUNK_64b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialWrite_Word64;
									__dummy_fptr = &dummy_forwSequentialLoop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialWrite_Word64;
									__dummy_fptr = &dummy_revSequentialLoop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Write_Word64;
									__dummy_fptr = &dummy_forwStride2Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Write_Word64;
									__dummy_fptr = &dummy_revStride2Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Write_Word64;
									__dummy_fptr = &dummy_forwStride4Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Write_Word64;
									__dummy_fptr = &dummy_revStride4Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Write_Word64;
									__dummy_fptr = &dummy_forwStride8Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Write_Word64;
									__dummy_fptr = &dummy_revStride8Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Write_Word64;
									__dummy_fptr = &dummy_forwStride16Loop_Word64;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Write_Word64;
									__dummy_fptr = &dummy_revStride16Loop_Word64;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_128b
						case CHUNK_128b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialWrite_Word128;
									__dummy_fptr = &dummy_forwSequentialLoop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialWrite_Word128;
									__dummy_fptr = &dummy_revSequentialLoop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Write_Word128;
									__dummy_fptr = &dummy_forwStride2Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Write_Word128;
									__dummy_fptr = &dummy_revStride2Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Write_Word128;
									__dummy_fptr = &dummy_forwStride4Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Write_Word128;
									__dummy_fptr = &dummy_revStride4Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Write_Word128;
									__dummy_fptr = &dummy_forwStride8Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Write_Word128;
									__dummy_fptr = &dummy_revStride8Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Write_Word128;
									__dummy_fptr = &dummy_forwStride16Loop_Word128;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Write_Word128;
									__dummy_fptr = &dummy_revStride16Loop_Word128;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif
#ifdef USE_CHUNK_256b
						case CHUNK_256b:
							switch (__stride_size) {
#ifdef USE_THROUGHPUT_FORW_STRIDE_1
								case 1:
									__bench_fptr = &forwSequentialWrite_Word256;
									__dummy_fptr = &dummy_forwSequentialLoop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_1
								case -1:
									__bench_fptr = &revSequentialWrite_Word256;
									__dummy_fptr = &dummy_revSequentialLoop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_2
								case 2:
									__bench_fptr = &forwStride2Write_Word256;
									__dummy_fptr = &dummy_forwStride2Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_2
								case -2:
									__bench_fptr = &revStride2Write_Word256;
									__dummy_fptr = &dummy_revStride2Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_4
								case 4:
									__bench_fptr = &forwStride4Write_Word256;
									__dummy_fptr = &dummy_forwStride4Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_4
								case -4:
									__bench_fptr = &revStride4Write_Word256;
									__dummy_fptr = &dummy_revStride4Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_8
								case 8:
									__bench_fptr = &forwStride8Write_Word256;
									__dummy_fptr = &dummy_forwStride8Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_8
								case -8:
									__bench_fptr = &revStride8Write_Word256;
									__dummy_fptr = &dummy_revStride8Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_FORW_STRIDE_16
								case 16:
									__bench_fptr = &forwStride16Write_Word256;
									__dummy_fptr = &dummy_forwStride16Loop_Word256;
									break;
#endif
#ifdef USE_THROUGHPUT_REV_STRIDE_16
								case -16:
									__bench_fptr = &revStride16Write_Word256;
									__dummy_fptr = &dummy_revStride16Loop_Word256;
									break;
#endif
								default:
									std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
									_obj_valid = false;
									break;
							}
							break;
#endif

						default:
							std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
							_obj_valid = false;
							break;
					}
					break;

				default:
					std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
					_obj_valid = false;
					break;
			}
			break;
#endif
#endif
#ifdef USE_THROUGHPUT_RANDOM_PATTERN
		case RANDOM:
			switch (__rw_mode) {
#ifdef USE_THROUGHPUT_READS
				case READ:
					switch (_chunk_size) {
#ifdef USE_CHUNK_32b
						case CHUNK_32b: 
							__bench_fptr = &randomRead_Word32;
							__dummy_fptr = &dummy_randomLoop_Word32;
							break;
#endif
#ifdef USE_CHUNK_64b
						case CHUNK_64b:
							__bench_fptr = &randomRead_Word64;
							__dummy_fptr = &dummy_randomLoop_Word64;
							break;
#endif
#ifdef USE_CHUNK_128b
						case CHUNK_128b:
							__bench_fptr = &randomRead_Word128;
							__dummy_fptr = &dummy_randomLoop_Word128;
							break;
#endif
#ifdef USE_CHUNK_256b
						case CHUNK_256b:
							__bench_fptr = &randomRead_Word256;
							__dummy_fptr = &dummy_randomLoop_Word256;
							break;
#endif

						default:
							std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
							__obj_valid = false;
							break;
					}
					break;
#endif
#ifdef USE_THROUGHPUT_WRITES				
				case WRITE:
					switch (_chunk_size) {
#ifdef USE_CHUNK_32b
						case CHUNK_32b:
							__bench_fptr = &randomWrite_Word32;
							__dummy_fptr = &dummy_randomLoop_Word32;
							break;
#endif
#ifdef USE_CHUNK_64b
						case CHUNK_64b:
							__bench_fptr = &randomWrite_Word64;
							__dummy_fptr = &dummy_randomLoop_Word64;
							break;
#endif
#ifdef USE_CHUNK_128b
						case CHUNK_128b:
							__bench_fptr = &randomWrite_Word128;
							__dummy_fptr = &dummy_randomLoop_Word128;
							break;
#endif
#ifdef USE_CHUNK_256b
						case CHUNK_256b:
							__bench_fptr = &randomWrite_Word256;
							__dummy_fptr = &dummy_randomLoop_Word256;
							break;
#endif
						default:
							std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
							__obj_valid = false;
							break;
					}
					break;
#endif

				default:
					std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
					__obj_valid = false;
					break;
			}
			break;
#endif

		default:
			std::cerr << "Got an invalid ThroughputBenchmark configuration." << std::endl;
			_obj_valid = false;
			break;
	}
}

void ThroughputBenchmark::report_benchmark_info() {
	std::cout << "CPU NUMA Node: " << _cpu_node << std::endl;
	std::cout << "Memory NUMA Node: " << _mem_node << std::endl;
	std::cout << "Chunk Size: ";
	switch (_chunk_size) {
#ifdef USE_CHUNK_32b
		case CHUNK_32b:
			std::cout << "32-bit";
			break;
#endif
#ifdef USE_CHUNK_64b
		case CHUNK_64b:
			std::cout << "64-bit";
			break;
#endif
#ifdef USE_CHUNK_128b
		case CHUNK_128b:
			std::cout << "128-bit";
			break;
#endif
#ifdef USE_CHUNK_256b
		case CHUNK_256b:
			std::cout << "256-bit";
			break;
#endif
		default:
			std::cout << "UNKNOWN";
			break;
	}
	std::cout << std::endl;

	std::cout << "Access Pattern: ";
	switch (__pattern_mode) {
#ifdef USE_THROUGHPUT_SEQUENTIAL_PATTERN
		case SEQUENTIAL:
			if (__stride_size > 0)
				std::cout << "forward ";
			else if (__stride_size < 0)
				std::cout << "reverse ";
			else 
				std::cout << "UNKNOWN ";

			if (__stride_size == 1 || __stride_size == -1)
				std::cout << "sequential";
			else 
				std::cout << "strides of " << __stride_size << " chunks";
			break;
#endif
#ifdef USE_THROUGHPUT_RANDOM_PATTERN
		case RANDOM:
			std::cout << "random";
			break;
#endif
		default:
			std::cout << "UNKNOWN";
			break;
	}
	std::cout << std::endl;


	std::cout << "Read/Write Mode: ";
	switch (__rw_mode) {
#ifdef USE_THROUGHPUT_READS
		case READ:
			std::cout << "read";
			break;
#endif
#ifdef USE_THROUGHPUT_WRITES
		case WRITE:
			std::cout << "write";
			break;
#endif
		default:
			std::cout << "UNKNOWN";
			break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

void ThroughputBenchmark::report_results() {
	__internal_report_results();
}

bool ThroughputBenchmark::__run_core() {
	if (_hasRun) //A benchmark should only be run once per object
		return false;

	//Spit out useful info
	std::cout << std::endl;
	std::cout << "-------- Running Benchmark: " << _name;
	std::cout << " ----------" << std::endl;
	report_benchmark_info(); 
	
	//Build indices for random workload
#ifdef USE_THROUGHPUT_RANDOM_PATTERN
	if (__pattern_mode == RANDOM) 
		__buildRandomPointerPermutation();
#endif

	//Start power measurement
#ifdef VERBOSE
	std::cout << "Starting power measurement threads...";
#endif
	if (!_start_power_threads())
		std::cout << "FAIL" << std::endl;
#ifdef VERBOSE
	else
		std::cout << "done" << std::endl;
#endif

	//Set up some stuff for worker threads
	size_t len_per_thread = _len / _num_worker_threads; //TODO: is this what we want?
	std::vector<ThroughputBenchmarkWorker*> workers;
	std::vector<thread::Thread*> worker_threads;

	//Do a bunch of iterations of the core benchmark routines
	#ifdef VERBOSE
		std::cout << "Running benchmark." << std::endl << std::endl;
	#endif
	for (uint32_t i = 0; i < _iterations; i++) {
		//Create workers and worker threads
		workers.reserve(_num_worker_threads);
		worker_threads.reserve(_num_worker_threads);
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			void* thread_mem_array = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + t * len_per_thread);
			workers.push_back(new ThroughputBenchmarkWorker(
												thread_mem_array,
												len_per_thread,
#ifdef USE_SIZE_BASED_BENCHMARKS
												_passes_per_iteration,
#endif
												__bench_fptr,
												__dummy_fptr,
												cpu_id_in_numa_node(_cpu_node, t)
											)
							);
			worker_threads.push_back(new thread::Thread(workers[t]));
			worker_threads[t]->create();
		}

		//Start worker threads! gogogo
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			worker_threads[t]->start();

		//Wait for all threads to complete
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			if (!worker_threads[t]->join(DEFAULT_THREAD_JOIN_TIMEOUT))
				std::cerr << "WARNING: A worker thread failed to complete by the expected time!" << std::endl;

		//Compute throughput achieved with all workers
		uint64_t total_passes = 0;
		uint64_t total_adjusted_ticks = 0;
		uint64_t avg_adjusted_ticks = 0;
		uint64_t total_elapsed_dummy_ticks = 0;
		uint64_t bytes_per_pass = workers[0]->getBytesPerPass(); //all should be the same.
		bool iter_warning = false;
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			total_passes += workers[t]->getPasses();
			total_adjusted_ticks += workers[t]->getAdjustedTicks();
			total_elapsed_dummy_ticks += workers[t]->getElapsedDummyTicks();
			iter_warning |= workers[t]->hadWarning();
		}

		avg_adjusted_ticks = total_adjusted_ticks / _num_worker_threads;
			
#ifdef VERBOSE
		//Report duration for this iteration
		std::cout << "Iter " << i+1 << " had " << total_passes << " passes in total across " << _num_worker_threads << " threads, with " << bytes_per_pass << " bytes touched per pass:";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;

		std::cout << "...clock ticks in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks << " (adjusted by -" << total_elapsed_dummy_ticks << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;
		
		std::cout << "...ns in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks * _timer->get_ns_per_tick() << " (adjusted by -" << total_elapsed_dummy_ticks * _timer->get_ns_per_tick() << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;

		std::cout << "...sec in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks * _timer->get_ns_per_tick() / 1e9 << " (adjusted by -" << total_elapsed_dummy_ticks * _timer->get_ns_per_tick() / 1e9 << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;
#endif
		
		//Compute metric for this iteration
		_metricOnIter[i] = ((static_cast<double>(total_passes) * static_cast<double>(bytes_per_pass)) / static_cast<double>(MB))   /   ((static_cast<double>(avg_adjusted_ticks) * _timer->get_ns_per_tick()) / 1e9);
		_averageMetric += _metricOnIter[i];


		//Clean up workers and threads for this iteration
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			delete worker_threads[t];
			delete workers[t];
		}
		worker_threads.clear();
		workers.clear();
	}

	//Stop power measurement
#ifdef VERBOSE
	std::cout << std::endl;
	std::cout << "Stopping power measurement threads...";
#endif
	if (!_stop_power_threads())
		std::cout << "FAIL" << std::endl;
#ifdef VERBOSE
	else
		std::cout << "done" << std::endl;
#endif
	
	//Run metadata
	_averageMetric /= static_cast<double>(_iterations);
	_hasRun = true;

	return true;
}

bool ThroughputBenchmark::run() {
	return __run_core();
}

bool ThroughputBenchmark::__buildRandomPointerPermutation() {
#ifdef VERBOSE
	std::cout << "Preparing memory region under test. This might take a while...";
#endif
	
	size_t num_pointers = 0; //Number of pointers that fit into the memory region
	
	switch (_chunk_size) {
#ifdef USE_CHUNK_32b
		case CHUNK_32b:
			num_pointers = _len / sizeof(Word32_t);
			break;
#endif
#ifdef USE_CHUNK_64b
		case CHUNK_64b:
			num_pointers = _len / sizeof(Word64_t);
			break;
#endif
#ifdef USE_CHUNK_128b
		case CHUNK_128b:
			num_pointers = _len / sizeof(Word128_t);
			break;
#endif
#ifdef USE_CHUNK_256b
		case CHUNK_256b:
			num_pointers = _len / sizeof(Word256_t);
			break;
#endif
		default:
			std::cerr << "ERROR! Got illegal chunk size when building random access pattern." << std::endl;
			return false;
	}
	
	uintptr_t* mem_base = static_cast<uintptr_t*>(_mem_array); 

	std::mt19937_64 gen(time(NULL)); //Mersenne Twister random number generator, seeded at current time

#ifdef USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN
	//Build a random directed Hamiltonian Cycle across the entire memory

	//Let W be the list of memory locations that have not been reached yet. Each entry is an index in mem_base.
	std::vector<size_t> W;
	size_t w_index = 0;

	//Initialize W to contain all memory locations, where each memory location appears exactly once in the list. The order does not strictly matter.
	W.resize(num_pointers);
	for (w_index = 0; w_index < num_pointers; w_index++) {
		W.at(w_index) = w_index;
		//std::cout << "w_index == " << w_index << ", W.at(w_index) == " << W.at(w_index) << std::endl;
	}
	
	//Build the directed Hamiltonian Cycle
	size_t v = 0; //the current memory location. Always start at the first location for the Hamiltonian Cycle construction
	size_t w = 0; //the next memory location
	w_index = 0;
	while (W.size() > 0) { //while we have not reached all memory locations
		//std::cout << "W.size() == " << W.size() << std::endl;

		W.erase(W.begin() + w_index);
		
		//Binary search for v
		/*bool found = false;
		size_t lbound = 0, ubound = W.size(); //init bounds and starting index
		while (!found) {
			//std::cout << "lbound == " << lbound << ", ubound == " << ubound << std::endl;
			w_index = (ubound + lbound) / 2; 
			if (W[w_index] == v) {//found
				//Remove v from W
				W.erase(W.begin() + w_index);
				found = true;
				//std::cout << "FOUND!" << std::endl;
			} else if (W[w_index] < v) { //v is larger, search right half
				lbound = w_index;
			} else { //v is smaller, search left half
				ubound = w_index;
			}
		}*/

		//Normal case
		if (W.size() > 0) {
			//Choose the next w_index at random from W
			w_index = gen() % W.size();
		//	std::cout << "Next w_index == " << w_index << std::endl;

			//Extract the memory location corresponding to this w_index
			w = W[w_index];
		} else { //Last element visited needs to point at head of memory to complete the cycle
			w = 0;
		}

		//std::cout << "w == " << w << std::endl;

		//std::cout << "Constructing pointer v == " << v << " --> w == " << w << std::endl;
		//Create pointer v --> w. This corresponds to a directed edge in the graph with nodes v and w.
		mem_base[v] = reinterpret_cast<uintptr_t>(mem_base + w);

		//Chase this pointer to move to next step
		v = w;
	}
#endif

#ifdef USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN
	//Do a random shuffle of the indices
	for (size_t i = 0; i < num_pointers; i++) //Initialize pointers to point at themselves (identity mapping)
		mem_base[i] = reinterpret_cast<uintptr_t>(mem_base+static_cast<uintptr_t>(i));
	std::shuffle(mem_base, mem_base + num_pointers, gen);
#endif

#ifdef VERBOSE
	std::cout << "done" << std::endl;
	std::cout << std::endl;
#endif

	return true;
}

void ThroughputBenchmark::__internal_report_results() {
	std::cout << std::endl;
	std::cout << "*** RESULTS";
	std::cout << "***" << std::endl;
	std::cout << std::endl;
 
	if (_hasRun) {
		for (uint32_t i = 0; i < _iterations; i++)
			std::cout << "Iter #" << i+1 << ": " << _metricOnIter[i] << " MB/s" << std::endl;
		std::cout << "Average: " << _averageMetric << " MB/s";
		if (_warning) std::cout << " (WARNING)";
		std::cout << std::endl;
		std::cout << std::endl;

		report_power_results();
	}
	else
		std::cout << "WARNING: Benchmark has not run yet. No reported results." << std::endl;
}

int64_t ThroughputBenchmark::getStrideSize() {
	return __stride_size;
}

pattern_mode_t ThroughputBenchmark::getPatternMode() {
	return __pattern_mode;
}

rw_mode_t ThroughputBenchmark::getRWMode() {
	return __rw_mode;
}
