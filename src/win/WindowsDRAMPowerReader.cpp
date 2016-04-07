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
 *
 * Author: Mark Gottscho <mgottscho@ucla.edu>
 */

/**
 * @file
 * 
 * @brief Implementation file for the WindowsDRAMPowerReader class.
 */

#ifdef _WIN32

//Headers
#include <win/WindowsDRAMPowerReader.h>
#include <common.h>
#include <win/win_CPdhQuery.h>
#include <Timer.h>

//Libraries
#include <cstdint>
#include <vector>

using namespace xmem;

WindowsDRAMPowerReader::WindowsDRAMPowerReader(uint32_t counter_cpu_index, uint32_t sampling_period, double power_units, std::string name, int32_t cpu_affinity) :
    PowerReader(sampling_period, power_units, name, cpu_affinity),
    counter_cpu_index_(counter_cpu_index),
    perf_counter_name_(""),
    pdhQuery_(NULL)
{
    perf_counter_name_ = static_cast<std::ostringstream*>(&(std::ostringstream() << "\\CPU Power Counters(" << counter_cpu_index_ << ")\\DRAM Power"))->str();
    try {
        pdhQuery_ = new CPdhQuery(perf_counter_name_);
    }
    catch (CPdhQuery::CException const &e) {
        //tcout << e.What() << std::endl;
    }
    if (pdhQuery_ == NULL) {
        std::cerr << "WARNING: Unable to collect DRAM power." << std::endl;
    }
}

WindowsDRAMPowerReader::~WindowsDRAMPowerReader() {
    delete pdhQuery_;
}

void WindowsDRAMPowerReader::run() {
    bool done = false;

    while (!done) {
        tick_t start_tick = start_timer();
        if (acquireLock(-1)) { //Wait indefinitely for the lock
            if (stop_signal_) //we're done here, let's wrap up
                done = true;
            releaseLock();
        }

        if (!done) {
            //Read power from performance counter
            double result = 0;

            if (pdhQuery_ != NULL) { //It might be NULL for different reasons, like the required performance counter is not present on the system. This is not fatal.
                try {
                    std::map<std::tstring, double> querydata;
                    querydata = pdhQuery_->CollectQueryData();
                    result = DumpMapValue(querydata);
                }
                catch (CPdhQuery::CException const &e) {
                    //tcout << e.What() << std::endl;
                }
            }

            //Thread-safe update of power trace
            if (acquireLock(-1)) { //Wait indefinitely for the lock
                if (num_samples_ >= power_trace_.capacity())
                    power_trace_.reserve(power_trace_.capacity() + 256); //add more space
                power_trace_.push_back(static_cast<double>(result));
                num_samples_++;
                releaseLock();
            }

            calculateMetrics();
            tick_t stop_tick = stop_timer();
            tick_t elapsed_ticks = stop_tick - start_tick;
            Sleep(static_cast<DWORD>(sampling_period_ - elapsed_ticks*g_ns_per_tick*1e-9*1000)); //Account for any loop overhead
        }
    }
}

#else
#error This file should only be used in Windows builds.
#endif
