/**
 * @file
 *
 * @brief Header file for the Timer class.
 *
 * (C) 2014 Microsoft Corporation
 */

#ifndef __TIMER_H
#define __TIMER_H

//Libraries
#include <cstdint>

namespace xmem {
	namespace timers {
		/**
		 * @brief This class abstracts a simple high resolution stopwatch timer.
		 * WARNING: these objects are NOT thread safe.
		 */
		class Timer {
		public:
			/**
			 * @brief Constructor. This may take a noticeable amount of time.
			 */
			Timer();

			/**
			 * @brief Starts the timer.
			 */
			virtual void start() = 0;

			/**
			 * @brief Stops the timer.
			 * @returns Elapsed time since last start() call in ticks.
			 */
			virtual uint64_t stop() = 0;

			/**
			 * @brief Stops the timer.
			 * @returns Elapsed time since last start() call in nanoseconds.
			 */
			double stop_in_ns();

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
};

#endif
