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
 * @brief Implementation file for the LatencyBenchmark class.
 */

//Headers
#include <include/LatencyBenchmark.h>
#include <include/common.h>
#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
#include <include/x86_64/TSCTimer.h>
#endif
#include <include/benchmark_kernels.h>

//Libraries
#include <iostream>
#include <algorithm>
#include <random>
#include <assert.h>
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
		
LatencyBenchmark::LatencyBenchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t passes_per_iteration,
#endif
		uint32_t cpu_node,
		uint32_t mem_node,
		uint32_t num_worker_threads,
		std::string name,
		xmem::timers::Timer *timer,
		std::vector<xmem::power::PowerReader*> dram_power_readers
	) :
	Benchmark(
		mem_array,
		len,
		iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		passes_per_iteration,
#endif
		NUM_CHUNK_SIZES,
		cpu_node,
		mem_node,
		num_worker_threads,
		name,
		timer,
		dram_power_readers
	),
	__bench_fptr(nullptr),
	__dummy_fptr(nullptr)
	{ 
	__bench_fptr = &chasePointers;
	__dummy_fptr = &dummy_chasePointers;
}

void LatencyBenchmark::report_benchmark_info() {
	std::cout << "CPU NUMA Node: " << _cpu_node << std::endl;
	std::cout << "Memory NUMA Node: " << _mem_node << std::endl;
	std::cout << "Access Pattern: random (pointer-chasing) -- read-only";
	std::cout << std::endl;
	std::cout << std::endl;
}

void LatencyBenchmark::report_results() {
	__internal_report_results();
}

bool LatencyBenchmark::__run_core() {
	if (_hasRun) //A benchmark should only be run once per object
		return false;

	//Spit out useful info
	std::cout << std::endl;
	std::cout << "-------- Running Benchmark: " << _name;
	std::cout << " ----------" << std::endl;
	report_benchmark_info(); 
	
	//Build indices for random workload
	__buildRandomPointerPermutation();

	//Set processor affinity
	bool locked = lock_thread_to_cpu(cpu_id_in_numa_node(_cpu_node,0)); //1st CPU in the node
	if (!locked)
		std::cout << "WARNING: Failed to lock thread to NUMA CPU Node " << _cpu_node << "! Results may not be correct." << std::endl;

	//Increase scheduling priority
#ifdef _WIN32
	DWORD originalPriorityClass = GetPriorityClass(GetCurrentProcess()); 	
	DWORD originalPriority = GetThreadPriority(GetCurrentThread());
	SetPriorityClass(GetCurrentProcess(), 0x80); //HIGH_PRIORITY_CLASS
	SetThreadPriority(GetCurrentThread(), 15); //THREAD_PRIORITY_TIME_CRITICAL
#else
	#error Windows is the only supported OS at this time.
#endif

	//Number of pointers followed per call of __chasePointers()
#ifdef USE_TIME_BASED_BENCHMARKS
	size_t num_pointers = LATENCY_BENCHMARK_UNROLL_LENGTH;
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
	size_t num_pointers = _len / sizeof(uintptr_t);
#endif
	
	//Prime memory
#ifdef VERBOSE
	std::cout << "Priming benchmark...";
#endif
	__primeMemory(4);
#ifdef VERBOSE
	std::cout << "done" << std::endl;

	//Start power measurement
	std::cout << "Starting power measurement threads...";
#endif
	if (!_start_power_threads())
		std::cout << "FAIL" << std::endl;
#ifdef VERBOSE
	else
		std::cout << "done" << std::endl;
#endif

	//Run benchmark
#ifdef VERBOSE
	std::cout << "Running benchmark." << std::endl << std::endl;
#endif

	//Do a bunch of iterations of the core benchmark routine
	uintptr_t* next_address = static_cast<uintptr_t*>(_mem_array);
	for (uint32_t i = 0; i < _iterations; i++) {
		uint64_t elapsed_ticks = 0, elapsed_dummy_ticks = 0, adjusted_ticks = 0, passes = 0;
		bool iter_warning = false;
#ifdef USE_TIME_BASED_BENCHMARKS
		uint64_t accesses_per_pass = LATENCY_BENCHMARK_UNROLL_LENGTH;
		uint64_t ticks = 0, target_ticks = 0, p = 0, p2 = 0;
		
		target_ticks = _timer->get_ticks_per_sec() * BENCHMARK_DURATION_SEC; //Rough target run duration in seconds 

		//Run actual version of function and loop overhead
		while (elapsed_ticks < target_ticks) {
			_timer->start();
			UNROLL256((*__bench_fptr)(next_address, &next_address, 0);)
			ticks = _timer->stop();
			elapsed_ticks += ticks;
			passes+=256;
		}

		//Run dummy version of function and loop overhead
		p = 0;
		while (p < passes) {
			_timer->start();
			UNROLL256((*__dummy_fptr)(NULL, NULL, 0);)
			ticks = _timer->stop();
			elapsed_dummy_ticks += ticks;
			p+=256;
		}
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t accesses_per_pass = _len / sizeof(uintptr_t);
		passes = _passes_per_iteration;

		//Time actual version of function and loop overhead
		_timer->start();
		for (uint64_t p = 0; p < _passes_per_iteration; p++)
			(*__bench_fptr)(next_address, &next_address, _len);
		elapsed_ticks = _timer->stop();

		//Time dummy version of function and loop overhead
		next_address = static_cast<uintptr_t*>(_mem_array); 
		_timer->start();
		for (uint64_t p = 0; p < _passes_per_iteration; p++)
			(*__dummy_fptr)(NULL, NULL, _len);
		elapsed_dummy_ticks = _timer->stop();
#endif

		adjusted_ticks = elapsed_ticks - elapsed_dummy_ticks; //In some odd circumstance where dummy is slower than real function, this will go to extremely high value. This will generate a WARNING in the console, but otherwise the benchmark will not be invalidated.
		
		//Warn if something looks fishy
		if (elapsed_dummy_ticks >= elapsed_ticks || elapsed_ticks < MIN_ELAPSED_TICKS || adjusted_ticks < 0.5 * elapsed_ticks) {
			iter_warning = true;
			_warning = true;
		}
	
#ifdef VERBOSE
		//Report metrics for this iteration
		std::cout << "Iter " << i+1 << " had " << passes << " passes, with " << accesses_per_pass << " accesses per pass:";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;

		std::cout << "...clock ticks == " << adjusted_ticks << " (adjusted by -" << elapsed_dummy_ticks << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;

		std::cout << "...ns == " << adjusted_ticks * _timer->get_ns_per_tick() << " (adjusted by -" << elapsed_dummy_ticks * _timer->get_ns_per_tick() << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;

		std::cout << "...sec == " << adjusted_ticks * _timer->get_ns_per_tick() / 1e9 << " (adjusted by -" << elapsed_dummy_ticks * _timer->get_ns_per_tick() / 1e9 << ")";
		if (iter_warning) std::cout << " -- WARNING";
		std::cout << std::endl;
#endif
		
		//Compute metric for this iteration
		_metricOnIter[i] = static_cast<double>(adjusted_ticks * _timer->get_ns_per_tick())  /  static_cast<double>(passes * num_pointers);
		_averageMetric += _metricOnIter[i];
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
	
	//Unset processor affinity
	if (locked)
		unlock_thread_to_numa_node();

	//Revert thread priority
#ifdef _WIN32
	SetPriorityClass(GetCurrentProcess(), originalPriorityClass);
	SetThreadPriority(GetCurrentThread(), originalPriority);
#else
#error Windows is the only supported OS at this time.
#endif

	//Run metadata
	_averageMetric /= static_cast<double>(_iterations);
	_hasRun = true;

	return true;
}

bool LatencyBenchmark::run() {
	return __run_core();
}

bool LatencyBenchmark::__buildRandomPointerPermutation() {
#ifdef VERBOSE
	std::cout << "Preparing memory region under test. This might take a while...";
#endif
	//std::fstream myfile;
	//myfile.open("test.txt", std::fstream::out);
	
	size_t num_pointers = _len / sizeof(uintptr_t); //Number of pointers that fit into the memory region
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

void LatencyBenchmark::__primeMemory(uint64_t passes) {
	void* start_address = _mem_array; 
	void* end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + _len);

	for (uint64_t p = 0; p < passes; p++)
		forwSequentialRead_Word64(start_address, end_address); //dependent reads on the memory, make sure caches are ready, coherence, etc...
}

void LatencyBenchmark::__internal_report_results() {
	std::cout << std::endl;
	std::cout << "*** RESULTS";
	std::cout << "***" << std::endl;
	std::cout << std::endl;
 
	if (_hasRun) {
		for (uint32_t i = 0; i < _iterations; i++)
			std::cout << "Iter #" << i + 1 << ": " << _metricOnIter[i] << " ns/access" << std::endl;
		std::cout << "Average: " << _averageMetric << " ns/access" << std::endl;
		if (_warning) std::cout << " (WARNING)";
		std::cout << std::endl;
		
		report_power_results();
	}
	else
		std::cout << "WARNING: Benchmark has not run yet. No reported results." << std::endl;
}
