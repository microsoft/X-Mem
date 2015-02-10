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
#include <LatencyBenchmark.h>
#include <common.h>
#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
#include <x86_64/TSCTimer.h>
#endif
#include <benchmark_kernels.h>

//Libraries
#include <iostream>
#include <random>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
//#include <intrin.h>
#include <processthreadsapi.h>
#endif

#ifdef __gnu_linux__
#include <unistd.h> //for nice()
#endif

using namespace xmem;
		
LatencyBenchmark::LatencyBenchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t passes_per_iteration,
#endif
		uint32_t num_worker_threads,
		uint32_t mem_node,
		uint32_t cpu_node,
		pattern_mode_t pattern_mode,
		rw_mode_t rw_mode,
		chunk_size_t chunk_size,
		int64_t stride_size,
		Timer& timer,
		std::vector<PowerReader*> dram_power_readers,
		std::string name
	) :
		Benchmark(
			mem_array,
			len,
			iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
			passes_per_iteration,
#endif
			num_worker_threads,
			mem_node,
			cpu_node,
			pattern_mode,
			rw_mode,
			chunk_size,
			stride_size,
			timer,
			dram_power_readers,
			"ns/access",
			name
		)
	{ 
}

bool LatencyBenchmark::_run_core() {
	//Spit out useful info
	std::cout << std::endl;
	std::cout << "-------- Running Benchmark: " << _name;
	std::cout << " ----------" << std::endl;
	report_benchmark_info(); 

	//Set up benchmark kernel function pointers
	RandomFunction kernel_fptr = &chasePointers;
	RandomFunction kernel_dummy_fptr = &dummy_chasePointers;

	//Build indices for random workload
	_buildRandomPointerPermutation();

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
#endif
#ifdef __gnu_linux__
	if (nice(-20) == EPERM) //Increase priority to maximum. This will require admin privileges and thus can fail. Note that this doesn't necessarily improve performance as Linux is still using a round-robin scheduler by default.
		std::cerr << "WARNING: Failed to increase Linux thread scheduling priority using \"nice\" value of -20. This probably happened because you ran X-Mem without administrator privileges." << std::endl;
#endif

	//Number of pointers followed per call of __chasePointers()
#ifdef USE_TIME_BASED_BENCHMARKS
	size_t num_pointers = LATENCY_BENCHMARK_UNROLL_LENGTH;
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
	size_t num_pointers = _len / sizeof(uintptr_t);
#endif
	
	//Prime memory
	void* prime_start_address = _mem_array; 
	void* prime_end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + _len);
	for (uint64_t i = 0; i < 4; i++) {
		//UNROLL256((*kernel_fptr)(prime_address, &prime_address, 0);)
		forwSequentialRead_Word64(prime_start_address, prime_end_address); //dependent reads on the memory, make sure caches are ready, coherence, etc...
	}

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
	uintptr_t* next_address = static_cast<uintptr_t*>(_mem_array);
	for (uint32_t i = 0; i < _iterations; i++) {
		uint64_t elapsed_ticks = 0, elapsed_dummy_ticks = 0, adjusted_ticks = 0, passes = 0;
		bool iter_warning = false;
#ifdef USE_TIME_BASED_BENCHMARKS
		uint64_t accesses_per_pass = LATENCY_BENCHMARK_UNROLL_LENGTH;
		uint64_t ticks = 0, target_ticks = 0, p = 0;
		
		target_ticks = _timer.get_ticks_per_sec() * BENCHMARK_DURATION_SEC; //Rough target run duration in seconds 

		//Run actual version of function and loop overhead
		while (elapsed_ticks < target_ticks) {
			_timer.start();
			UNROLL256((*kernel_fptr)(next_address, &next_address, 0);)
			ticks = _timer.stop();
			elapsed_ticks += ticks;
			passes+=256;
		}

		//Run dummy version of function and loop overhead
		p = 0;
		while (p < passes) {
			_timer.start();
			UNROLL256((*kernel_dummy_fptr)(NULL, NULL, 0);)
			ticks = _timer.stop();
			elapsed_dummy_ticks += ticks;
			p+=256;
		}
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t accesses_per_pass = _len / sizeof(uintptr_t);
		passes = _passes_per_iteration;

		//Time actual version of function and loop overhead
		_timer.start();
		for (uint64_t p = 0; p < _passes_per_iteration; p++)
			(*kernel_fptr)(next_address, &next_address, _len);
		elapsed_ticks = _timer.stop();

		//Time dummy version of function and loop overhead
		next_address = static_cast<uintptr_t*>(_mem_array); 
		_timer.start();
		for (uint64_t p = 0; p < _passes_per_iteration; p++)
			(*kernel_dummy_fptr)(NULL, NULL, _len);
		elapsed_dummy_ticks = _timer.stop();
#endif

		adjusted_ticks = elapsed_ticks - elapsed_dummy_ticks; //In some odd circumstance where dummy is slower than real function, this will go to extremely high value. This will generate a WARNING in the console, but otherwise the benchmark will not be invalidated.
		
		//Warn if something looks fishy
		if (elapsed_dummy_ticks >= elapsed_ticks || elapsed_ticks < MIN_ELAPSED_TICKS || adjusted_ticks < 0.5 * elapsed_ticks) {
			iter_warning = true;
			_warning = true;
		}
	
		if (g_verbose) { //Report metrics for this iteration
			std::cout << "Iter " << i+1 << " had " << passes << " passes, with " << accesses_per_pass << " accesses per pass:";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...clock ticks == " << adjusted_ticks << " (adjusted by -" << elapsed_dummy_ticks << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...ns == " << adjusted_ticks * _timer.get_ns_per_tick() << " (adjusted by -" << elapsed_dummy_ticks * _timer.get_ns_per_tick() << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;

			std::cout << "...sec == " << adjusted_ticks * _timer.get_ns_per_tick() / 1e9 << " (adjusted by -" << elapsed_dummy_ticks * _timer.get_ns_per_tick() / 1e9 << ")";
			if (iter_warning) std::cout << " -- WARNING";
			std::cout << std::endl;
		}
		
		//Compute metric for this iteration
		_metricOnIter[i] = static_cast<double>(adjusted_ticks * _timer.get_ns_per_tick())  /  static_cast<double>(passes * num_pointers);
		_averageMetric += _metricOnIter[i];
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
	
	//Unset processor affinity
	if (locked)
		unlock_thread_to_numa_node();

	//Revert thread priority
#ifdef _WIN32
	SetPriorityClass(GetCurrentProcess(), originalPriorityClass);
	SetThreadPriority(GetCurrentThread(), originalPriority);
#endif
#ifdef __gnu_linux__
	if (nice(0) == EPERM) //Set priority to nominal. Note that this doesn't necessarily affect performance as Linux is still using a round-robin scheduler by default.
		std::cerr << "WARNING: Failed to set Linux thread scheduling priority using default \"nice\" value of 0. This shouldn\'t have happened." << std::endl;
#endif

	//Run metadata
	_averageMetric /= static_cast<double>(_iterations);
	_hasRun = true;

	return true;
}
