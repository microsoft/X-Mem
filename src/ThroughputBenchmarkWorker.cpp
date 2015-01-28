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
 * @brief Implementation file for the ThroughputBenchmarkWorker class.
 */

//Headers
#include <ThroughputBenchmarkWorker.h>
#include <benchmark_kernels.h>
#include <common.h>

#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
#include <x86_64/TSCTimer.h>
#endif

#ifdef _WIN32
#ifdef USE_QPC_TIMER
#include <win/QPCTimer.h>
#endif
#endif

#ifdef _WIN32
#include <windows.h>
//#include <intrin.h>
#include <processthreadsapi.h>
#endif

#ifdef __gnu_linux__
#include <unistd.h> //for nice()
#endif


//Libraries
#include <iostream>
using namespace xmem::benchmark;
using namespace xmem::common;

ThroughputBenchmarkWorker::ThroughputBenchmarkWorker(
		void* mem_array,
		size_t len,
	#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t passes_per_iteration,
	#endif
		BenchFunction bench_fptr,
		BenchFunction dummy_fptr,
		uint32_t cpu_affinity
	) :
	__mem_array(mem_array),
	__len(len),
	__cpu_affinity(cpu_affinity),
	__bench_fptr(bench_fptr),
	__dummy_fptr(dummy_fptr),
	__bytes_per_pass(0),
	__passes(0),
	__elapsed_ticks(0),
	__elapsed_dummy_ticks(0),
	__adjusted_ticks(0),
	__warning(false),
#ifdef USE_SIZE_BASED_BENCHMARKS
	__passes_per_iteration(passes_per_iteration),
#endif
	__completed(false)
	{

}

ThroughputBenchmarkWorker::~ThroughputBenchmarkWorker() {
}

void ThroughputBenchmarkWorker::run() {
	//Set up relevant state -- localized to this thread's stack
	uint32_t cpu_affinity = 0;
	BenchFunction bench_fptr = NULL;
	BenchFunction dummy_fptr = NULL;
	void* start_address = NULL;
	void* end_address = NULL;
	void* prime_start_address = NULL;
	void* prime_end_address = NULL;
	uint64_t bytes_per_pass = 0;
	uint64_t passes = 0;
	uint64_t start_tick = 0;
	uint64_t stop_tick = 0;
	uint64_t elapsed_ticks = 0;
	uint64_t elapsed_dummy_ticks = 0;
	uint64_t adjusted_ticks = 0;
	bool warning = false;

#ifdef USE_TIME_BASED_BENCHMARKS
	void* mem_array = NULL;
	size_t len = 0;
#ifdef USE_TSC_TIMER
	timers::x86_64::TSCTimer helper_timer;
#endif
#ifdef USE_QPC_TIMER
	timers::win::QPCTimer helper_timer;
#endif
	uint64_t target_ticks = helper_timer.get_ticks_per_sec() * BENCHMARK_DURATION_SEC; //Rough target run duration in seconds 
	uint64_t p = 0;
	bytes_per_pass = THROUGHPUT_BENCHMARK_BYTES_PER_PASS;
#endif
	
	//Grab relevant setup state thread-safely and keep it local
	if (_acquireLock(-1)) {
#ifdef USE_TIME_BASED_BENCHMARKS
		mem_array = __mem_array;
		len = __len;
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
		bytes_per_pass = __len;
		passes = __passes_per_iteration;
#endif
		cpu_affinity = __cpu_affinity;
		bench_fptr = __bench_fptr;
		dummy_fptr = __dummy_fptr;
		start_address = __mem_array;
		end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(__mem_array)+bytes_per_pass);
		prime_start_address = __mem_array; 
		prime_end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(__mem_array) + __len);
		_releaseLock();
	}
	
	//Set processor affinity
	bool locked = lock_thread_to_cpu(cpu_affinity);
	if (!locked)
		std::cerr << "WARNING: Failed to lock thread to NUMA CPU Node " << __cpu_affinity << "! Results may not be correct." << std::endl;

	//Increase scheduling priority for this thread
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

	//Prime memory
	benchmark_kernels::forwSequentialWrite_Word64(start_address, end_address);  //initialize memory by writing and force page faults if pages are not resident in physical memory
	benchmark_kernels::forwSequentialRead_Word64(start_address, end_address); //dependent reads on the memory, make sure caches are ready, coherence, etc...
	for (uint64_t i = 0; i < 4; i++) {
		(*__bench_fptr)(start_address, end_address); //run the benchmark several times to ensure caches are warmed up for the algorithm and CPU is ready to go
	}

	//Run the benchmark!
#ifdef USE_TIME_BASED_BENCHMARKS
	//Run actual version of function and loop overhead
	while (elapsed_ticks < target_ticks) {
#ifdef USE_TSC_TIMER
		start_tick = xmem::timers::x86_64::start_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
		start_tick = xmem::timers::win::get_qpc_time();
#endif
		UNROLL1024(
			(*bench_fptr)(start_address, end_address);
			/*if (end_address > reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+len))
				start_address = mem_array;
			else
				start_address = end_address;*/
			start_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+(reinterpret_cast<uint64_t>(start_address)+bytes_per_pass) % len);
			end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(start_address) + bytes_per_pass);
		)
#ifdef USE_TSC_TIMER
		stop_tick = xmem::timers::x86_64::stop_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
		stop_tick = xmem::timers::win::get_qpc_time();
#endif

		elapsed_ticks += (stop_tick - start_tick);
		passes+=1024;
	}

	//Run dummy version of function and loop overhead
	p = 0;
	start_address = mem_array;
	end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array) + bytes_per_pass);
	while (p < passes) {
#ifdef USE_TSC_TIMER
		start_tick = xmem::timers::x86_64::start_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
		start_tick = xmem::timers::win::get_qpc_time();
#endif
		UNROLL1024(
			(*dummy_fptr)(start_address, end_address);
			/*if (end_address >= reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+len))
				start_address = mem_array;
			else
				start_address = end_address;*/
			start_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array)+(reinterpret_cast<uint64_t>(start_address)+bytes_per_pass) % len);
			end_address = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(start_address) + bytes_per_pass);
		)
#ifdef USE_TSC_TIMER
		stop_tick = xmem::timers::x86_64::stop_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
		stop_tick = xmem::timers::win::get_qpc_time();
#endif
		elapsed_dummy_ticks += (stop_tick - start_tick);
		p+=1024;
	}

#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
#ifdef USE_TSC_TIMER
	start_tick = xmem::timers::x86_64::start_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
	start_tick = xmem::timers::win::get_qpc_time();
#endif
	for (uint64_t p = 0; p < __passes_per_iteration; p++)
		(*bench_fptr)(start_address, end_address);
#ifdef USE_TSC_TIMER
	stop_tick = xmem::timers::x86_64::stop_tsc_timer();
#endif
#ifdef USE_QPC_TIMER
	stop_tick = xmem::timers::win::get_qpc_time();
#endif
	elapsed_ticks = stop_tick - start_tick;

	//Time dummy version of function and loop overhead
	start_tick = xmem::timers::x86_64::start_tsc_timer();
	for (uint64_t p = 0; p < __passes_per_iteration; p++)
		(*dummy_fptr)(start_address, end_address);
	stop_tick = xmem::timers::x86_64::stop_tsc_timer();
	elapsed_dummy_ticks = stop_tick - start_tick;
#endif

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

	adjusted_ticks = elapsed_ticks - elapsed_dummy_ticks; //In some odd circumstance where dummy is slower than real function, this will go to extremely high value. This will generate a WARNING in the console, but otherwise the benchmark will not be invalidated.
	
	//Warn if something looks fishy
	if (elapsed_dummy_ticks >= elapsed_ticks || elapsed_ticks < MIN_ELAPSED_TICKS || adjusted_ticks < 0.5 * elapsed_ticks) {
		warning = true;
	}

	//Update the object state thread-safely
	if (_acquireLock(-1)) {
		__adjusted_ticks = adjusted_ticks;
		__elapsed_ticks = elapsed_ticks;
		__elapsed_dummy_ticks = elapsed_dummy_ticks;
		__warning = warning;
		__bytes_per_pass = bytes_per_pass;
		__completed = true;
		__passes = passes;
		_releaseLock();
	}
}

size_t ThroughputBenchmarkWorker::getLen() {
	size_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __len;
		_releaseLock();
	}

	return retval;
}

uint64_t ThroughputBenchmarkWorker::getBytesPerPass() {
	size_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __bytes_per_pass;
		_releaseLock();
	}

	return retval;
}

uint64_t ThroughputBenchmarkWorker::getPasses() {
	size_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __passes;
		_releaseLock();
	}

	return retval;
}
				
uint64_t ThroughputBenchmarkWorker::getElapsedTicks() {
	uint64_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __elapsed_ticks;
		_releaseLock();
	}

	return retval;
}

uint64_t ThroughputBenchmarkWorker::getElapsedDummyTicks() {
	uint64_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __elapsed_dummy_ticks;
		_releaseLock();
	}

	return retval;
}

uint64_t ThroughputBenchmarkWorker::getAdjustedTicks() {
	uint64_t retval = 0;
	if (_acquireLock(-1)) {
		retval = __adjusted_ticks;
		_releaseLock();
	}

	return retval;
}

bool ThroughputBenchmarkWorker::hadWarning() {
	bool retval = true;
	if (_acquireLock(-1)) {
		retval = __warning;
		_releaseLock();
	}

	return retval;
}

