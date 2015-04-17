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
 * @brief Implementation file for the LoadWorker class.
 */

//Headers
#include <LoadWorker.h>
#include <benchmark_kernels.h>
#include <common.h>

//Libraries
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

#ifdef __gnu_linux__
#include <unistd.h>
#endif

using namespace xmem;

LoadWorker::LoadWorker(
		void* mem_array,
		size_t len,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint32_t passes_per_iteration,
#endif
		SequentialFunction kernel_fptr,
		SequentialFunction kernel_dummy_fptr,
		int32_t cpu_affinity
	) :
		MemoryWorker(
			mem_array,
			len,
#ifdef USE_SIZE_BASED_BENCHMARKS
			passes_per_iteration,
#endif
			cpu_affinity
		),
		__use_sequential_kernel_fptr(true),
		__kernel_fptr_seq(kernel_fptr),
		__kernel_dummy_fptr_seq(kernel_dummy_fptr),
		__kernel_fptr_ran(NULL),
		__kernel_dummy_fptr_ran(NULL)
	{
}

LoadWorker::LoadWorker(
		void* mem_array,
		size_t len,
	#ifdef USE_SIZE_BASED_BENCHMARKS
		uint32_t passes_per_iteration,
	#endif
		RandomFunction kernel_fptr,
		RandomFunction kernel_dummy_fptr,
		int32_t cpu_affinity
	) :
		MemoryWorker(
			mem_array,
			len,
#ifdef USE_SIZE_BASED_BENCHMARKS
			passes_per_iteration,
#endif
			cpu_affinity
		),
		__use_sequential_kernel_fptr(false),
		__kernel_fptr_seq(NULL),
		__kernel_dummy_fptr_seq(NULL),
		__kernel_fptr_ran(kernel_fptr),
		__kernel_dummy_fptr_ran(kernel_dummy_fptr)
	{
}

LoadWorker::~LoadWorker() {
}

void LoadWorker::run() {
	//Set up relevant state -- localized to this thread's stack
	int32_t cpu_affinity = 0;
	bool use_sequential_kernel_fptr = false;
	SequentialFunction kernel_fptr_seq = NULL;
	SequentialFunction kernel_dummy_fptr_seq = NULL;
	RandomFunction kernel_fptr_ran = NULL;
	RandomFunction kernel_dummy_fptr_ran = NULL;
	void* start_address = NULL;
	void* end_address = NULL;
	void* prime_start_address = NULL;
	void* prime_end_address = NULL;
	uint32_t bytes_per_pass = 0;
	uint32_t passes = 0;
	tick_t start_tick = 0;
	tick_t stop_tick = 0;
	tick_t elapsed_ticks = 0;
	tick_t elapsed_dummy_ticks = 0;
	tick_t adjusted_ticks = 0;
	bool warning = false;

#ifdef USE_TIME_BASED_BENCHMARKS
	void* mem_array = NULL;
	size_t len = 0;
	tick_t target_ticks = g_ticks_per_ms * BENCHMARK_DURATION_MS; //Rough target run duration in ticks
	uint32_t p = 0;
	bytes_per_pass = THROUGHPUT_BENCHMARK_BYTES_PER_PASS;
#endif
	
	//Grab relevant setup state thread-safely and keep it local
	if (_acquireLock(-1)) {
#ifdef USE_TIME_BASED_BENCHMARKS
		mem_array = _mem_array;
		len = _len;
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
		bytes_per_pass = _len;
		passes = _passes_per_iteration;
#endif
		cpu_affinity = _cpu_affinity;
		use_sequential_kernel_fptr = __use_sequential_kernel_fptr;
		kernel_fptr_seq = __kernel_fptr_seq;
		kernel_dummy_fptr_seq = __kernel_dummy_fptr_seq;
		kernel_fptr_ran = __kernel_fptr_ran;
		kernel_dummy_fptr_ran = __kernel_dummy_fptr_ran;
		start_address = _mem_array;
		end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array)+bytes_per_pass);
		prime_start_address = _mem_array; 
		prime_end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + _len);
		_releaseLock();
	}
	
	//Set processor affinity
	bool locked = lock_thread_to_cpu(cpu_affinity);
	if (!locked)
		std::cerr << "WARNING: Failed to lock thread to logical CPU " << cpu_affinity << "! Results may not be correct." << std::endl;

	//Increase scheduling priority
#ifdef _WIN32
	DWORD originalPriorityClass;
	DWORD originalPriority;
	if (!boostSchedulingPriority(originalPriorityClass, originalPriority))
#endif
#ifdef __gnu_linux__
	if (!boostSchedulingPriority())
#endif
		std::cerr << "WARNING: Failed to boost scheduling priority. Perhaps running in Administrator mode would help." << std::endl;

	//Prime memory
	for (uint32_t i = 0; i < 4; i++) {
		forwSequentialRead_Word64(prime_start_address, prime_end_address); //dependent reads on the memory, make sure caches are ready, coherence, etc...
	}

	//Run the benchmark!
	uintptr_t* next_address = static_cast<uintptr_t*>(mem_array);
#ifdef USE_TIME_BASED_BENCHMARKS
	//Run actual version of function and loop overhead
	while (elapsed_ticks < target_ticks) {
		if (use_sequential_kernel_fptr) { //sequential function semantics
			start_tick = start_timer();
			UNROLL1024(
				(*kernel_fptr_seq)(start_address, end_address);
				start_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+(reinterpret_cast<uint64_t>(start_address)+bytes_per_pass) % len);
				end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(start_address) + bytes_per_pass);
			)
			stop_tick = stop_timer();
			passes+=1024;
		} else { //random function semantics
			start_tick = start_timer();
			UNROLL1024((*kernel_fptr_ran)(next_address, &next_address, bytes_per_pass);)
			stop_tick = stop_timer();
			passes+=1024;
		}
		elapsed_ticks += (stop_tick - start_tick);
	}

	//Run dummy version of function and loop overhead
	p = 0;
	start_address = mem_array;
	end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array) + bytes_per_pass);
	next_address = static_cast<uintptr_t*>(mem_array);
	while (p < passes) {
		if (use_sequential_kernel_fptr) { //sequential function semantics
			start_tick = start_timer();
			UNROLL1024(
				(*kernel_dummy_fptr_seq)(start_address, end_address);
				start_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+(reinterpret_cast<uint64_t>(start_address)+bytes_per_pass) % len);
				end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(start_address) + bytes_per_pass);
			)
			stop_tick = stop_timer();
			p+=1024;
		} else { //random function semantics
			start_tick = start_timer();
			UNROLL1024((*kernel_dummy_fptr_ran)(next_address, &next_address, bytes_per_pass);)
			stop_tick = stop_timer();
			p+=1024;
		}

		elapsed_dummy_ticks += (stop_tick - start_tick);
	}
#endif

#ifdef USE_SIZE_BASED_BENCHMARKS
	next_address = static_cast<uintptr_t*>(mem_array);
	if (use_sequential_kernel_fptr) { //sequential function semantics
		start_tick = start_timer();
		for (uint32_t p = 0; p < __passes_per_iteration; p++)
			(*kernel_fptr_seq)(start_address, end_address);
		stop_tick = stop_timer();
	} else { //random function semantics
		start_tick = start_timer();
		for (uint32_t p = 0; p < __passes_per_iteration; p++)
			(*kernel_fptr_ran)(next_address, &next_address, bytes_per_pass);
		stop_tick = stop_timer();
	}
	elapsed_ticks = stop_tick - start_tick;

	//Time dummy version of function and loop overhead
	next_address = static_cast<uintptr_t*>(mem_array);
	if (use_sequential_kernel_fptr) { //sequential function semantics
		start_tick = start_timer();
		for (uint32_t p = 0; p < __passes_per_iteration; p++)
			(*kernel_dummy_fptr_seq)(start_address, end_address);
		stop_tick = stop_timer();
	} else { //random function semantics
		start_tick = start_timer();
		for (uint32_t p = 0; p < __passes_per_iteration; p++)
			(*kernel_dummy_fptr_ran)(next_address, &next_address, bytes_per_pass);
		stop_tick = stop_timer();
	}
	elapsed_dummy_ticks = stop_tick - start_tick;
#endif

	//Unset processor affinity
	if (locked)
		unlock_thread_to_numa_node();

	//Revert thread priority
#ifdef _WIN32
	if (!revertSchedulingPriority(originalPriorityClass, originalPriority))
#endif
#ifdef __gnu_linux__
	if (!revertSchedulingPriority())
#endif
		std::cerr << "WARNING: Failed to revert scheduling priority. Perhaps running in Administrator mode would help." << std::endl;

	adjusted_ticks = elapsed_ticks - elapsed_dummy_ticks;

	//Warn if something looks fishy
	if (elapsed_dummy_ticks >= elapsed_ticks || elapsed_ticks < MIN_ELAPSED_TICKS || adjusted_ticks < 0.5 * elapsed_ticks)
		warning = true;

	//Update the object state thread-safely
	if (_acquireLock(-1)) {
		_adjusted_ticks = adjusted_ticks;
		_elapsed_ticks = elapsed_ticks;
		_elapsed_dummy_ticks = elapsed_dummy_ticks;
		_warning = warning;
		_bytes_per_pass = bytes_per_pass;
		_completed = true;
		_passes = passes;
		_releaseLock();
	}
}
