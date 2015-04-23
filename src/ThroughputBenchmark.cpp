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
#include <ThroughputBenchmark.h>
#include <common.h>
#include <LoadWorker.h>
#include <Thread.h>

//Libraries
#include <iostream>
#include <assert.h>
#include <time.h>

using namespace xmem;

ThroughputBenchmark::ThroughputBenchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
		uint32_t num_worker_threads,
		uint32_t mem_node,
		uint32_t cpu_node,
		pattern_mode_t pattern_mode,
		rw_mode_t rw_mode,
		chunk_size_t chunk_size,
		int32_t stride_size,
		std::vector<PowerReader*> dram_power_readers,
		std::string name
	) :
	Benchmark(
		mem_array,
		len,
		iterations,
		num_worker_threads,
		mem_node,
		cpu_node,
		pattern_mode,
		rw_mode,
		chunk_size,
		stride_size,
		dram_power_readers,
		"MB/s",
		name
	)
	{
}

bool ThroughputBenchmark::_run_core() {
	size_t len_per_thread = _len / _num_worker_threads; //Carve up memory space so each worker has its own area to play in

	//Set up kernel function pointers
	SequentialFunction kernel_fptr_seq = NULL;
	SequentialFunction kernel_dummy_fptr_seq = NULL; 
	RandomFunction kernel_fptr_ran = NULL;
	RandomFunction kernel_dummy_fptr_ran = NULL; 

	if (_pattern_mode == SEQUENTIAL) {
		if (!determineSequentialKernel(_rw_mode, _chunk_size, _stride_size, &kernel_fptr_seq, &kernel_dummy_fptr_seq)) {
			std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
			return false;
		}
	} else if (_pattern_mode == RANDOM) {
		if (!determineRandomKernel(_rw_mode, _chunk_size, &kernel_fptr_ran, &kernel_dummy_fptr_ran)) {
			std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
			return false;
		}
			
		//Build pointer indices. Note that the pointers for each thread must stay within its respective region, otherwise sharing may occur. 
		for (uint32_t i = 0; i < _num_worker_threads; i++) {
			if (!buildRandomPointerPermutation(reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + i*len_per_thread), //casts to silence compiler warnings
											   reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + (i+1)*len_per_thread), //casts to silence compiler warnings
											   _chunk_size)) {
				std::cerr << "ERROR: Failed to build a random pointer permutation for a worker thread!" << std::endl;
				return false;
			}
		}
	} else {
		std::cerr << "ERROR: Got an invalid pattern mode." << std::endl;
		return false;
	}

	//Set up some stuff for worker threads
	std::vector<LoadWorker*> workers;
	std::vector<Thread*> worker_threads;

	//Start power measurement
	if (g_verbose) 
		std::cout << "Starting power measurement threads...";
	if (!_start_power_threads()) {
		if (g_verbose)
			std::cout << "FAIL" << std::endl;
		std::cerr << "WARNING: Failed to start power measurement threads." << std::endl;
	} else if (g_verbose)
		std::cout << "done" << std::endl;

	//Run benchmark
	if (g_verbose)
		std::cout << "Running benchmark." << std::endl << std::endl;

	//Do a bunch of iterations of the core benchmark routines
	for (uint32_t i = 0; i < _iterations; i++) {
		//Create workers and worker threads
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			void* thread_mem_array = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + t * len_per_thread);
			int32_t cpu_id = cpu_id_in_numa_node(_cpu_node, t);
			if (cpu_id < 0)
				std::cerr << "WARNING: Failed to find logical CPU " << t << " in NUMA node " << _cpu_node << std::endl;
			if (_pattern_mode == SEQUENTIAL)
				workers.push_back(new LoadWorker(thread_mem_array,
												 len_per_thread,
												 kernel_fptr_seq,
												 kernel_dummy_fptr_seq,
												 cpu_id));
			else if (_pattern_mode == RANDOM)
				workers.push_back(new LoadWorker(thread_mem_array,
												 len_per_thread,
												 kernel_fptr_ran,
												 kernel_dummy_fptr_ran,
												 cpu_id));
			else
				std::cerr << "WARNING: Invalid benchmark pattern mode." << std::endl;
			worker_threads.push_back(new Thread(workers[t]));
		}

		//Start worker threads! gogogo
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			worker_threads[t]->create_and_start();

		//Wait for all threads to complete
		for (uint32_t t = 0; t < _num_worker_threads; t++)
			if (!worker_threads[t]->join())
				std::cerr << "WARNING: A worker thread failed to complete correctly!" << std::endl;

		//Compute throughput achieved with all workers
		uint32_t total_passes = 0;
		tick_t total_adjusted_ticks = 0;
		tick_t avg_adjusted_ticks = 0;
		tick_t total_elapsed_dummy_ticks = 0;
		uint32_t bytes_per_pass = workers[0]->getBytesPerPass(); //all should be the same.
		bool iter_warning = false;
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			total_passes += workers[t]->getPasses();
			total_adjusted_ticks += workers[t]->getAdjustedTicks();
			total_elapsed_dummy_ticks += workers[t]->getElapsedDummyTicks();
			iter_warning |= workers[t]->hadWarning();
		}

		avg_adjusted_ticks = total_adjusted_ticks / _num_worker_threads;

		if (iter_warning)
			_warning = true;
			
		if (g_verbose ) { //Report duration for this iteration
			std::cout << "Iter " << i+1 << " had " << total_passes << " passes in total across " << _num_worker_threads << " threads, with " << bytes_per_pass << " bytes touched per pass:";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...clock ticks in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks << " (adjusted by -" << total_elapsed_dummy_ticks << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;
			
			std::cout << "...ns in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks * g_ns_per_tick << " (adjusted by -" << total_elapsed_dummy_ticks * g_ns_per_tick << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...sec in total across " << _num_worker_threads << " threads == " << total_adjusted_ticks * g_ns_per_tick / 1e9 << " (adjusted by -" << total_elapsed_dummy_ticks * g_ns_per_tick / 1e9 << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;
		}
		
		//Compute metric for this iteration
		_metricOnIter[i] = ((static_cast<double>(total_passes) * static_cast<double>(bytes_per_pass)) / static_cast<double>(MB))   /   ((static_cast<double>(avg_adjusted_ticks) * g_ns_per_tick) / 1e9);
		_averageMetric += _metricOnIter[i];


		//Clean up workers and threads for this iteration
		for (uint32_t t = 0; t < _num_worker_threads; t++) {
			delete worker_threads[t];
			delete workers[t];
		}
		worker_threads.clear();
		workers.clear();
	}

	//Stopping power measurement
	if (g_verbose) 
		std::cout << "Stopping power measurement threads...";
	if (!_stop_power_threads()) {
		if (g_verbose)
			std::cout << "FAIL" << std::endl;
		std::cerr << "WARNING: Failed to stop power measurement threads." << std::endl;
	} else if (g_verbose)
		std::cout << "done" << std::endl;
	
	//Run metadata
	_averageMetric /= static_cast<double>(_iterations);
	_hasRun = true;

	return true;
}
