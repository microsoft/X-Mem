/**
 * @file
 *
 * @brief Header file for the QPCTimer class.
 *
 * (C) 2014 Microsoft Corporation
 */

//Headers
#include "QPCTimer.h"

//Libraries
#ifdef _WIN32
#include <windows.h>
#else
#error Windows is the only supported OS at this time.
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
#error Windows is the only supported OS at this time.
#endif
	_ns_per_tick = (1/(double)(_ticks_per_sec)) * 1e9;
}

void QPCTimer::start() {
#ifdef _WIN32
	QueryPerformanceCounter(&__start_tick);
#else
#error Windows is the only supported OS at this time.
#endif
}

uint64_t QPCTimer::stop() {
#ifdef _WIN32
	QueryPerformanceCounter(&__stop_tick);
	return (__stop_tick.QuadPart - __start_tick.QuadPart); 
#else
#error Windows is the only supported OS at this time.
#endif
}

uint64_t xmem::timers::win::get_qpc_time() {
#ifdef _WIN32
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return static_cast<uint64_t>(tmp.QuadPart);
#endif
}
