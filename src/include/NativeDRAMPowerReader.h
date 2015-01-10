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
 * @brief Header file for the NativeDRAMPowerReader class.
 */

#ifndef __NATIVE_DRAM_POWER_READER_H
#define __NATIVE_DRAM_POWER_READER_H

//Headers
#include <common.h>
#include <PowerReader.h>
#include <Runnable.h>
#include <win/win_CPdhQuery.h>

//Libraries
#include <cstdint>
#include <vector>
#include <string>

namespace xmem {
	namespace power {
		/**
		 * @brief A class for measuring socket-level DRAM power from the OS performance counter interface.
		 */
		class NativeDRAMPowerReader : public PowerReader {
		public:
			/**
			 * @brief Constructor.
			 * @param counter_cpu_index Which CPU's DRAM power counter to sample. A single hardware counter might be shared across different CPUs.
			 * @param sampling_period The time between power samples in seconds.
			 * @param power_units The power units for each sample in watts.
			 * @param cpu_affinity The CPU affinity for this object's run() method for any thread that calls it. If negative, no affinity preference.
			 */
			NativeDRAMPowerReader(uint32_t counter_cpu_index, double sampling_period, double power_units, std::string name, int32_t cpu_affinity);

			/**
			 * @brief Destructor.
			 */
			~NativeDRAMPowerReader();

			/**
			 * @brief Starts measuring power at the rate implied by the sampling_period passed in the constructor. Terminates when stop() is called.
			 */
			virtual void run();

		private:
			uint32_t __counter_cpu_index; /**< The CPU index to use when measuring performance counters. */
#ifdef _WIN32
			std::string __perf_counter_name; /**< The performance counter name exposed by the OS. */
			xmem::common::win::third_party::CPdhQuery* __pdhQuery; /**< Pointer to the object used to query OS for the performance counter. If this is NULL, it means we cannot access it for some reason, perhaps permissions. This is not fatal and should be handled gracefully. */
#else
#error Windows is the only supported OS at this time.
#endif
		};
	};
};

#endif
