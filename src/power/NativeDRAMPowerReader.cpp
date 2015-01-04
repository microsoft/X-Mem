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
 * @brief Implementation file for the NativeDRAMPowerReader class.
 */

//Headers
#include <power/NativeDRAMPowerReader.h>
#include <common/common.h>
#ifdef _WIN32
#include <common/win/third_party/win_CPdhQuery.h>
#else
#error Windows is the only supported OS at this time.
#endif

//Libraries
#include <cstdint>
#include <vector>

using namespace xmem::power;

NativeDRAMPowerReader::NativeDRAMPowerReader(uint32_t counter_cpu_index, double sampling_period, double power_units, std::string name, int32_t cpu_affinity) :
	PowerReader(sampling_period, power_units, name, cpu_affinity),
	__counter_cpu_index(counter_cpu_index),
#ifdef _WIN32
	__perf_counter_name(""),
	__pdhQuery(NULL)
#else
#error Windows is the only supported OS at this time.
#endif
{
#ifdef _WIN32
	__perf_counter_name = static_cast<std::ostringstream*>(&(std::ostringstream() << "\\CPU Power Counters(" << __counter_cpu_index << ")\\DRAM Power"))->str();
	try {
		__pdhQuery = new xmem::common::win::third_party::CPdhQuery(__perf_counter_name);
	}
	catch (xmem::common::win::third_party::CPdhQuery::CException const &e) {
		//tcout << e.What() << std::endl;
	}
	if (__pdhQuery == NULL) {
		std::cerr << "WARNING: Unable to collect DRAM power." << std::endl;
	}
#else
#error Windows is the only supported OS at this time.
#endif
}

NativeDRAMPowerReader::~NativeDRAMPowerReader() {
#ifdef _WIN32
	if (__pdhQuery != NULL)
		delete __pdhQuery;
#else
#error Windows is the only supported OS at this time.
#endif
}

void NativeDRAMPowerReader::run() {
	bool done = false;

	while (!done) {
#ifdef _WIN32
		if (_acquireLock(-1)) { //Wait indefinitely for the lock
			if (_stop_signal) //we're done here, let's wrap up
				done = true;
			_releaseLock();
		}

		if (!done) {
			//Read power from performance counter
			double result = 0;

			if (__pdhQuery != NULL) { //It might be NULL for different reasons, like the required performance counter is not present on the system. This is not fatal.
				try {
					std::map<std::tstring, double> querydata;
					querydata = __pdhQuery->CollectQueryData();
					result = common::win::third_party::DumpMapValue(querydata);
				}
				catch (xmem::common::win::third_party::CPdhQuery::CException const &e) {
					//tcout << e.What() << std::endl;
				}
			}

			//Thread-safe update of power trace
			if (_acquireLock(-1)) { //Wait indefinitely for the lock
				if (_num_samples >= _power_trace.capacity())
					_power_trace.reserve(_power_trace.capacity() + 256); //add more space
				_power_trace.push_back(static_cast<double>(result));
				_num_samples++;
				_releaseLock();
			}

			calculateMetrics();
			Sleep(static_cast<DWORD>(_sampling_period* 1000)); //FIXME: this assumes that the loop duration itself is negligible. This might not always be true. If the duration is non-negligible the sampling period will actually be longer than expected.
		}
	}
#else
#error Windows is the only supported OS at this time.
#endif
}
