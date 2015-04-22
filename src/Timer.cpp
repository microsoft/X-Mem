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
 * @brief Implementation file for the Timer class.
 */

//Headers
#include <Timer.h>
#include <common.h>

//Libraries
#ifdef _WIN32
#include <windows.h> 
#endif

#ifdef __gnu_linux__
#include <time.h>
#endif

using namespace xmem;

Timer::Timer() :
	_ticks_per_ms(0),
	_ns_per_tick(0)
{	
	tick_t start_tick, stop_tick;
	
	start_tick = start_timer();
#ifdef _WIN32
	Sleep(100);
#endif
#ifdef __gnu_linux__
	struct timespec duration, remainder;
	duration.tv_sec = 0;
	duration.tv_nsec = 1e8;
	nanosleep(&duration, &remainder);
#endif
	stop_tick = stop_timer();
	_ticks_per_ms = (stop_tick - start_tick) / 100;
	_ns_per_tick = 1/(static_cast<double>(_ticks_per_ms)) * 1e6;
}

tick_t Timer::get_ticks_per_ms() {
	return _ticks_per_ms;
}

double Timer::get_ns_per_tick() {
	return _ns_per_tick;
}
