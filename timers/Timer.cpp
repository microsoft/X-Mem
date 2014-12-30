/**
 * @file
 *
 * @brief Implementation file for the Timer class.
 *
 * (C) 2014 Microsoft Corporation
 */

#include "Timer.h"

using namespace xmem::timers;

Timer::Timer() :
	_ticks_per_sec(0),
	_ns_per_tick(0)
{	
}

double Timer::stop_in_ns() {
	return stop() * _ns_per_tick;
}

uint64_t Timer::get_ticks_per_sec() {
	return _ticks_per_sec;
}

double Timer::get_ns_per_tick() {
	return _ns_per_tick;
}
