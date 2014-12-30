/**
 * @file
 *
 * @brief Header file for the QPCTimer class.
 *
 * (C) 2014 Microsoft Corporation
 */

#ifndef __QPCTIMER_H
#define __QPCTIMER_H

//Headers
#include <timers/Timer.h>

//Libraries
#include <cstdint>
#ifdef _WIN32
#include <Windows.h>
#else
#error Windows is the only supported OS at this time.
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
#error Windows is the only supported OS at this time.
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
