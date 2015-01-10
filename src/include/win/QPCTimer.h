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

#ifndef __QPCTIMER_H
#define __QPCTIMER_H

//Headers
#include <Timer.h>

//Libraries
#include <cstdint>
#ifdef _WIN32
#include <Windows.h>
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif

namespace xmem {
	namespace timers {
		namespace win {
			/**
			 * @brief This class implements a simple high resolution stopwatch timer based on Windows' QueryPerformanceCounter API.
			 * WARNING: these objects are NOT thread safe.
			 */
			class QPCTimer : public Timer {
			public:
				/**
				 * @brief Constructor. This may take a noticeable amount of time.
				 */
				QPCTimer();

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
#ifdef _WIN32
				LARGE_INTEGER __start_tick; /**< The timer tick when start() was last called. */
				LARGE_INTEGER __stop_tick; /**< The timer tick when stop() was last called. */
#else
#error Windows is the only supported OS for the QPCTimer class.
#endif
			};

			/**
			 * @brief Query the QPC timer.
			 * @returns The current QPC timer tick.
			 */
			uint64_t get_qpc_time();
		};
	};
};

#endif
