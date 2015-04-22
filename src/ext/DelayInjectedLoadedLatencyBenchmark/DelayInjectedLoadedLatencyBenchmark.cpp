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
 * @brief Implementation file for the DelayInjectedLatencyBenchmark class.
 */


//Headers
#include <common.h>

#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK

#include <DelayInjectedLoadedLatencyBenchmark.h>
#include <delay_injected_benchmark_kernels.h>
#include <MemoryWorker.h>
#include <LatencyWorker.h>
#include <LoadWorker.h>

//Libraries
#include <iostream>
#include <random>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

using namespace xmem;
		
DelayInjectedLoadedLatencyBenchmark::DelayInjectedLoadedLatencyBenchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint32_t passes_per_iteration,
#endif
		uint32_t num_worker_threads,
		uint32_t mem_node,
		uint32_t cpu_node,
		chunk_size_t chunk_size,
		std::vector<PowerReader*> dram_power_readers,
		std::string name,
		uint32_t delay
	) :
		LatencyBenchmark(
			mem_array,
			len,
			iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
			passes_per_iteration,
#endif
			num_worker_threads,
			mem_node,
			cpu_node,
			SEQUENTIAL,
			READ,
			chunk_size,
			1,
			dram_power_readers,
			name
		),
		__delay(delay)
	{ 
}

void DelayInjectedLoadedLatencyBenchmark::report_benchmark_info() const {
	LatencyBenchmark::report_benchmark_info();
	std::cout << "Load worker kernel delay value: " << __delay << std::endl;
}

uint32_t DelayInjectedLoadedLatencyBenchmark::getDelay() const {
	return __delay;
}

bool DelayInjectedLoadedLatencyBenchmark::_run_core() {
	size_t len_per_thread = _len / _num_worker_threads; //Carve up memory space so each worker has its own area to play in

	//Set up latency measurement kernel function pointers
	RandomFunction lat_kernel_fptr = &chasePointers;
	RandomFunction lat_kernel_dummy_fptr = &dummy_chasePointers;

	//Initialize memory regions for all threads by writing to them, causing the memory to be physically resident.
	forwSequentialWrite_Word64(_mem_array,
							   reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array)+_len)); //static casts to silence compiler warnings

	//Build pointer indices for random-access latency thread. We assume that latency thread is the first one, so we use beginning of memory region.
	if (!buildRandomPointerPermutation(_mem_array,
									   reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array)+len_per_thread), //static casts to silence compiler warnings
									   CHUNK_64b)) { 
		std::cerr << "ERROR: Failed to build a random pointer permutation for the latency measurement thread!" << std::endl;
		return false;
	}

	//Set up load generation kernel function pointers
	SequentialFunction load_kernel_fptr = NULL;
	SequentialFunction load_kernel_dummy_fptr = NULL; 
	if (_num_worker_threads > 1) { //If we only have one worker thread, it is used for latency measurement only, and no load threads will be used.
		switch (_chunk_size) {
#ifdef HAS_WORD_64
			case CHUNK_64b:
				switch (__delay) {
					case 0:
						load_kernel_fptr = &forwSequentialRead_Word64; //not an extended kernel
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64; //not an extended kernel
						break;
					case 1:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay1;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay1;
						break;
					case 2:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay2;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay2;
						break;
					case 4:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay4;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay4;
						break;
					case 8:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay8;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay8;
						break;
					case 16:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay16;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay16;
						break;
					case 32:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay32;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay32;
						break;
					case 64:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay64;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay64;
						break;
					case 128:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay128;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay128;
						break;
					case 256:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay256;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay256plus;
						break;
					case 512:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay512;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay256plus;
						break;
					case 1024:
						load_kernel_fptr = &forwSequentialRead_Word64_Delay1024;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word64_Delay256plus;
						break;
					default:
						std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
						return false;
				}
				break;
#endif
#ifdef HAS_WORD_256
			case CHUNK_256b:
				switch (__delay) {
					case 0:
						load_kernel_fptr = &forwSequentialRead_Word256; //not an extended kernel
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256; //not an extended kernel
						break;
					case 1:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay1;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay1;
						break;
					case 2:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay2;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay2;
						break;
					case 4:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay4;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay4;
						break;
					case 8:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay8;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay8;
						break;
					case 16:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay16;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay16;
						break;
					case 32:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay32;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay32;
						break;
					case 64:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay64;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay64plus;
						break;
					case 128:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay128;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay64plus;
						break;
					case 256:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay256;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay64plus;
						break;
					case 512:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay512;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay64plus;
						break;
					case 1024:
						load_kernel_fptr = &forwSequentialRead_Word256_Delay1024;
						load_kernel_dummy_fptr = &dummy_forwSequentialLoop_Word256_Delay64plus;
						break;
					default:
						std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
						return false;
				}
				break;
#endif
			default:
				std::cerr << "ERROR: Chunk size must be 64-bit or 256-bit to run the delay-injected latency benchmark extension!" << std::endl;
				return false;
		}
	}

	//Set up some stuff for worker threads
	std::vector<MemoryWorker*> workers;
	std::vector<Thread*> worker_threads;
	
	//Start power measurement
	if (g_verbose)
		std::cout << "Starting power measurement threads...";
	
	if (!_start_power_threads()) {
		if (g_verbose)
			std::cout << "FAIL" << std::endl;
		std::cerr << "WARNING: Failed to start power threads." << std::endl;
	} else if (g_verbose)
		std::cout << "done" << std::endl;
	
	//Run benchmark
	if (g_verbose)
		std::cout << "Running benchmark." << std::endl << std::endl;

	//Do a bunch of iterations of the core benchmark routine
	for (uint32_t i = 0; i < _iterations; i++) {

		//Create load workers and load worker threads
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			void* thread_mem_array = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + t*len_per_thread);
			int32_t cpu_id = cpu_id_in_numa_node(_cpu_node, t);
			if (cpu_id < 0)
				std::cerr << "WARNING: Failed to find logical CPU " << t << " in NUMA node " << _cpu_node << std::endl;
			if (t == 0) { //special case: thread 0 is always latency thread
				workers.push_back(new LatencyWorker(thread_mem_array,
												    len_per_thread,
#ifdef USE_SIZE_BASED_BENCHMARKS
												    _passes_per_iteration,
#endif
												    lat_kernel_fptr,
												    lat_kernel_dummy_fptr,
												    cpu_id));
			} else {
				workers.push_back(new LoadWorker(thread_mem_array,
												 len_per_thread,
#ifdef USE_SIZE_BASED_BENCHMARKS
												 _passes_per_iteration,
#endif
												 load_kernel_fptr,
												 load_kernel_dummy_fptr,
												 cpu_id));
			}
			worker_threads.push_back(new Thread(workers[t]));
		}

		//Start worker threads! gogogo
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			worker_threads[t]->create_and_start();

		//Wait for all threads to complete
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			if (!worker_threads[t]->join())
				std::cerr << "WARNING: A worker thread failed to complete correctly!" << std::endl;
		
		//Compute metrics for this iteration
		bool iter_warning = false;

		//Compute latency metric
		uint32_t lat_passes = workers[0]->getPasses();	
		tick_t lat_adjusted_ticks = workers[0]->getAdjustedTicks();
		tick_t lat_elapsed_dummy_ticks = workers[0]->getElapsedDummyTicks();
		uint32_t lat_bytes_per_pass = workers[0]->getBytesPerPass();
		uint32_t lat_accesses_per_pass = lat_bytes_per_pass / 8;
		iter_warning |= workers[0]->hadWarning();
		
		//Compute throughput generated by load threads
		uint32_t load_total_passes = 0;
		tick_t load_total_adjusted_ticks = 0;
		tick_t load_total_elapsed_dummy_ticks = 0;
		uint32_t load_bytes_per_pass = 0;
		double load_avg_adjusted_ticks = 0;
		for (uint32_t t = 1; t < _num_worker_threads; t++) {
			load_total_passes += workers[t]->getPasses();
			load_total_adjusted_ticks += workers[t]->getAdjustedTicks();
			load_total_elapsed_dummy_ticks += workers[t]->getElapsedDummyTicks();
			load_bytes_per_pass = workers[t]->getBytesPerPass(); //all should be the same.
			iter_warning |= workers[t]->hadWarning();
		}

		//Compute load metrics for this iteration
		load_avg_adjusted_ticks = static_cast<double>(load_total_adjusted_ticks) / (_num_worker_threads-1);
		if (_num_worker_threads > 1)
			_loadMetricOnIter[i] = (((static_cast<double>(load_total_passes) * static_cast<double>(load_bytes_per_pass)) / static_cast<double>(MB)))   /  ((load_avg_adjusted_ticks * g_ns_per_tick) / 1e9);

		if (iter_warning)
			_warning = true;
	
		if (g_verbose) { //Report metrics for this iteration
			//Latency thread
			std::cout << "Iter " << i+1 << " had " << lat_passes << " latency measurement passes, with " << lat_accesses_per_pass << " accesses per pass:";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...lat clock ticks == " << lat_adjusted_ticks << " (adjusted by -" << lat_elapsed_dummy_ticks << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...lat ns == " << lat_adjusted_ticks * g_ns_per_tick << " (adjusted by -" << lat_elapsed_dummy_ticks * g_ns_per_tick << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...lat sec == " << lat_adjusted_ticks * g_ns_per_tick / 1e9 << " (adjusted by -" << lat_elapsed_dummy_ticks * g_ns_per_tick / 1e9 << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			//Load threads
			if (_num_worker_threads > 1) {
				std::cout << "Iter " << i+1 << " had " << load_total_passes << " total load generation passes, with " << load_bytes_per_pass << " bytes per pass:";
				if (iter_warning) std::cout << " -- WARNING";
				std::cout << std::endl;

				std::cout << "...load total clock ticks across " << _num_worker_threads-1 << " threads == " << load_total_adjusted_ticks << " (adjusted by -" << load_total_elapsed_dummy_ticks << ")";
				if (iter_warning) std::cout << " -- WARNING";
				std::cout << std::endl;

				std::cout << "...load total ns across " << _num_worker_threads-1 << " threads == " << load_total_adjusted_ticks * g_ns_per_tick << " (adjusted by -" << load_total_elapsed_dummy_ticks * g_ns_per_tick << ")";
				if (iter_warning) std::cout << " -- WARNING";
				std::cout << std::endl;

				std::cout << "...load total sec across " << _num_worker_threads-1 << " threads == " << load_total_adjusted_ticks * g_ns_per_tick / 1e9 << " (adjusted by -" << load_total_elapsed_dummy_ticks * g_ns_per_tick / 1e9 << ")";
				if (iter_warning) std::cout << " -- WARNING";
				std::cout << std::endl;
			}
		}
		
		//Compute overall metrics for this iteration
		_metricOnIter[i] = static_cast<double>(lat_adjusted_ticks * g_ns_per_tick)  /  static_cast<double>(lat_accesses_per_pass * lat_passes);
		_averageMetric += _metricOnIter[i];
		_averageLoadMetric += _loadMetricOnIter[i];
		
		//Clean up workers and threads for this iteration
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			delete worker_threads[t];
			delete workers[t];
		}
		worker_threads.clear();
		workers.clear();
	}

	//Stop power measurement
	if (g_verbose) {
		std::cout << std::endl;
		std::cout << "Stopping power measurement threads...";
	}
	
	if (!_stop_power_threads()) {
		if (g_verbose)
			std::cout << "FAIL" << std::endl;
		std::cerr << "WARNING: Failed to stop power measurement threads." << std::endl;
	} else if (g_verbose)
		std::cout << "done" << std::endl;
	
	//Run metadata
	_averageMetric /= static_cast<double>(_iterations);
	_averageLoadMetric /= static_cast<double>(_iterations);
	_hasRun = true;

	return true;
}

#endif
