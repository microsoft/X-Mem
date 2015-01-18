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
 * @brief Header file for the TSCTimer class as well as some C-style functions for working with the TSC timer hardware directly.
 */

#ifndef __TSCTIMER_H
#define __TSCTIMER_H

//Headers
#include <common.h>

#ifdef ARCH_INTEL_X86_64

//Headers
#include <Timer.h>

//Libraries
#include <cstdint>

namespace xmem {
	namespace timers {
		namespace x86_64 {
			/**
			 * @brief This class implements a simple high resolution stopwatch timer based on Intel's TimeStamp Counter (TSC) hardware timer.
			 *
			 * WARNING: These objects are NOT thread safe!
			 *
			 * Note that the nanoseconds per tick -could- be inaccurate due to dynamic frequency scaling of the processor on certain platforms. For this reason,
			 * you should either fix the clock frequency and/or validate the ns per tick before trusting results from this timer, or otherwise know that your processor has a
			 * frequency-invariant TSC timer.
			 *
			 * Also, the hardware implementation of this timer may vary, and in some cases, different CPU cores may have different timers
			 * and they are not guaranteed to be synchronized! It is recommended to ensure the thread using the TSCTimer stays on the same core for the duration of the
			 * timed code.
			 *
			 * Definitely only use this class if you know what you are doing!
			 */
			class TSCTimer : public Timer {
			public:
				/**
				 * @brief Constructor.
				 *
				 * This may take a noticeable amount of time (milliseconds to seconds)
				 * in order to measure the tick rate against another known timer.
				 */
				TSCTimer();

				/**
				 * @brief Starts the timer.
				 */
				virtual void start();

				/**
				 * @brief Stops the timer.
				 * @returns Elapsed time since last start() call in ticks.
				 */
				virtual uint64_t stop();

			private:
				uint64_t __start_tick; /**< The reading from the TSC register when start() was last called. */
				uint64_t __stop_tick; /**< The reading from the TSC register when stop() was last called. */
			};

			/**
			 * @brief Query the TSC hardware timer for the start of a timed section of code.
			 * @returns The starting tick for some timed section of code using the hardware TSC timer.
			 */
			uint64_t start_tsc_timer();

			/**
			 * @brief Query the TSC hardware timer for the end of a timed section of code.
			 * @returns The ending tick for some timed section of code using the hardware TSC timer.
			 */
			uint64_t stop_tsc_timer();
		};
	};
};

#else
#error This file should only be included in Intel x86_64 builds.
#endif

#endif
