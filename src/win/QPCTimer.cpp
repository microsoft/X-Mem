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
 * @brief Header file for the QPCTimer class.
 */

//Headers
#include <include/win/QPCTimer.h>

//Libraries
#ifdef _WIN32
#include <windows.h>
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif

using namespace xmem::timers::win;

QPCTimer::QPCTimer() :
	Timer(),
	__start_tick(),
	__stop_tick()
{	
	//Override default Timer constructor initialization of __ticks_per_sec and __ns_per_tick using the OS-derived values
#ifdef _WIN32
	LARGE_INTEGER tmp;
	QueryPerformanceFrequency(&tmp);
	_ticks_per_sec = tmp.QuadPart;
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif
	_ns_per_tick = (1/(double)(_ticks_per_sec)) * 1e9;
}

void QPCTimer::start() {
#ifdef _WIN32
	QueryPerformanceCounter(&__start_tick);
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif
}

uint64_t QPCTimer::stop() {
#ifdef _WIN32
	QueryPerformanceCounter(&__stop_tick);
	return (__stop_tick.QuadPart - __start_tick.QuadPart); 
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif
}

uint64_t xmem::timers::win::get_qpc_time() {
#ifdef _WIN32
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return static_cast<uint64_t>(tmp.QuadPart);
#endif
}
