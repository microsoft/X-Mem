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
 * @brief Implementation file for the TSCTimer class as well as some C-style functions for working with the TSC timer hardware directly.
 */

//Headers
#include <x86_64/TSCTimer.h>

//Libraries
#ifdef _WIN32
#include <windows.h> 
#include <intrin.h>
#endif

#ifdef __gnu_linux__
#include <immintrin.h>
#include <time.h>
#include <cpuid.h>
#include <x86intrin.h>
#endif

using namespace xmem::timers::x86_64;

TSCTimer::TSCTimer() :
	Timer(),
	__start_tick(0),
	__stop_tick(0)
{	
	start();
#ifdef _WIN32
	Sleep(1000);
#endif
#ifdef __gnu_linux__
	struct timespec duration, remainder;
	duration.tv_sec = 1;
	duration.tv_nsec = 0;
	nanosleep(&duration, &remainder);
#endif
	_ticks_per_sec = stop();
	_ns_per_tick = 1/((double)(_ticks_per_sec)) * 1e9;
}

void TSCTimer::start() {
	__start_tick = start_tsc_timer();
}

uint64_t TSCTimer::stop() {
	__stop_tick = stop_tsc_timer();
	return (__stop_tick - __start_tick);
}


uint64_t xmem::timers::x86_64::start_tsc_timer() {
#ifdef _WIN32
	int32_t dontcare[4];
	__cpuid(dontcare, 0); //Serializing instruction. This forces all previous instructions to finish
#endif
#ifdef __gnu_linux__
	int32_t dc0, dc1, dc2, dc3, dc4;
	__cpuid(dc0, dc1, dc2, dc3, dc4); //Serializing instruction. This forces all previous instructions to finish
#endif
	return __rdtsc(); //Get clock tick
}

uint64_t xmem::timers::x86_64::stop_tsc_timer() {
	uint64_t tick;
	uint32_t filler;
#ifdef _WIN32
	int32_t dontcare[4];
	tick = __rdtscp(&filler); //Get clock tick. This is a partially serializing instruction. All previous instructions must finish
	__cpuid(dontcare, 0); //Fully serializing instruction. We do this to prevent later instructions from being moved inside the timed section
#endif
#ifdef __gnu_linux__
	int32_t dc0, dc1, dc2, dc3, dc4;
	tick = __rdtscp(&filler); //Get clock tick. This is a partially serializing instruction. All previous instructions must finish
	__cpuid(dc0, dc1, dc2, dc3, dc4); //Serializing instruction. This forces all previous instructions to finish
#endif
	return tick;
}
