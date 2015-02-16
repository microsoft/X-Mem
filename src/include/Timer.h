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
 * @brief Header file for the Timer class.
 */

#ifndef __TIMER_H
#define __TIMER_H

//Libraries
#include <cstdint>

namespace xmem {
	/**
	 * @brief This class abstracts some characteristics of simple high resolution stopwatch timer.
	 * However, due to the inability or complexity of abstracting shared hardware timers,
	 * this class does not actually provide start and stop functions.
	 */
	class Timer {
	public:
		/**
		 * @brief Constructor. This may take a noticeable amount of time.
		 */
		Timer();

		/**
		 * @brief Gets ticks per second for this timer.
		 * @returns The reported number of ticks per second.
		 */
		uint64_t get_ticks_per_sec();

		/**
		 * @brief Gets nanoseconds per tick for this timer.
		 * @returns the number of nanoseconds per tick
		 */
		double get_ns_per_tick();

	protected:
		uint64_t _ticks_per_sec; /**< Ticks per second for this timer. */
		double _ns_per_tick; /**< Nanoseconds per tick for this timer. */
	};
};

#endif
