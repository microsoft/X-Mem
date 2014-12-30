/**
 * @file
 * 
 * @brief Header file for the NativeDRAMPowerReader class.
 * 
 * (C) 2014 Microsoft Corporation
 */

#ifndef __NATIVE_DRAM_POWER_READER_H
#define __NATIVE_DRAM_POWER_READER_H

//Headers
#include <common/common.h>
#include <power/PowerReader.h>
#include <thread/Runnable.h>
#include <common/win/third_party/win_CPdhQuery.h>

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
