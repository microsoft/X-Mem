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
 * @brief Implementation file for the PowerReader class.
 */

//Headers
#include <PowerReader.h>
#include <common.h>

//Libraries
#include <cstdint>
#include <vector>
#include <iostream>

using namespace xmem;

PowerReader::PowerReader(uint32_t sampling_period, double power_units, std::string name, int32_t cpu_affinity) :
    stop_signal_(false),
    power_units_(power_units),
    name_(name),
    cpu_affinity_(cpu_affinity),
    power_trace_(),
    mean_power_(0),
    peak_power_(0),
    num_samples_(0),
    sampling_period_(sampling_period)
{
    power_trace_.reserve(16); //Arbitrary default
}

PowerReader::~PowerReader() {
}

bool PowerReader::stop() {
    bool success = false;

    if (acquireLock(-1)) { //Wait indefinitely for the lock
        stop_signal_ = true;
        success = releaseLock();
    }
    
    return success;
}

bool PowerReader::calculateMetrics() {
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        mean_power_ = 0;
        peak_power_ = 0;
        num_samples_ = power_trace_.size();
        for (uint32_t i = 0; i < num_samples_; i++) {
            mean_power_ += power_trace_[i];
            if (power_trace_[i] > peak_power_)
                peak_power_ = power_trace_[i];
        }
        if (num_samples_ > 0)
            mean_power_ /= num_samples_;

        return releaseLock();
    } else
        return false;
}

bool PowerReader::clear() {
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        power_trace_.clear();
        mean_power_ = 0;
        peak_power_ = 0;
        num_samples_ = 0;
        return releaseLock();
    } else
        return false;
}

bool PowerReader::clearAndReset() {
    if (stop() && clear()) {
        if (acquireLock(-1)) { //Wait indefinitely for the lock
            stop_signal_ = false;
            return releaseLock();
        }
    }

    return false;
}

std::vector<double> PowerReader::getPowerTrace() {
    std::vector<double> retval;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = power_trace_;
        releaseLock();
    }
    return retval;
}

double PowerReader::getMeanPower() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = mean_power_;
        releaseLock();
    }
    return retval;
}

double PowerReader::getPeakPower() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = peak_power_;
        releaseLock();
    }
    return retval;
}

double PowerReader::getLastSample() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = power_trace_[power_trace_.size() - 1];
        releaseLock();
    }
    return retval;
}

uint32_t PowerReader::getSamplingPeriod() {
    uint32_t retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = sampling_period_;
        releaseLock();
    }
    return retval;
}

double PowerReader::getPowerUnits() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = power_units_;
        releaseLock();
    }
    return retval;
}

size_t PowerReader::getNumSamples() {
    size_t retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = num_samples_;
        releaseLock();
    }
    return retval;
}

std::string PowerReader::name() {
    std::string retval;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = name_;
        releaseLock();
    }
    return retval;
}
