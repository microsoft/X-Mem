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
    _stop_signal(false),
    _power_units(power_units),
    _name(name),
    _cpu_affinity(cpu_affinity),
    _power_trace(),
    _mean_power(0),
    _peak_power(0),
    _num_samples(0),
    _sampling_period(sampling_period)
{
    _power_trace.reserve(16); //Arbitrary default
}

PowerReader::~PowerReader() {
}

bool PowerReader::stop() {
    bool success = false;

    if (acquireLock(-1)) { //Wait indefinitely for the lock
        _stop_signal = true;
        success = releaseLock();
    }
    
    return success;
}

bool PowerReader::calculateMetrics() {
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        _mean_power = 0;
        _peak_power = 0;
        _num_samples = _power_trace.size();
        for (uint32_t i = 0; i < _num_samples; i++) {
            _mean_power += _power_trace[i];
            if (_power_trace[i] > _peak_power)
                _peak_power = _power_trace[i];
        }
        if (_num_samples > 0)
            _mean_power /= _num_samples;

        return releaseLock();
    } else
        return false;
}

bool PowerReader::clear() {
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        _power_trace.clear();
        _mean_power = 0;
        _peak_power = 0;
        _num_samples = 0;
        return releaseLock();
    } else
        return false;
}

bool PowerReader::clear_and_reset() {
    if (stop() && clear()) {
        if (acquireLock(-1)) { //Wait indefinitely for the lock
            _stop_signal = false;
            return releaseLock();
        }
    }

    return false;
}

std::vector<double> PowerReader::getPowerTrace() {
    std::vector<double> retval;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _power_trace;
        releaseLock();
    }
    return retval;
}

double PowerReader::getMeanPower() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _mean_power;
        releaseLock();
    }
    return retval;
}

double PowerReader::getPeakPower() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _peak_power;
        releaseLock();
    }
    return retval;
}

double PowerReader::getLastSample() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _power_trace[_power_trace.size() - 1];
        releaseLock();
    }
    return retval;
}

uint32_t PowerReader::getSamplingPeriod() {
    uint32_t retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _sampling_period;
        releaseLock();
    }
    return retval;
}

double PowerReader::getPowerUnits() {
    double retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _power_units;
        releaseLock();
    }
    return retval;
}

size_t PowerReader::getNumSamples() {
    size_t retval = 0;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _num_samples;
        releaseLock();
    }
    return retval;
}

std::string PowerReader::name() {
    std::string retval;
    if (acquireLock(-1)) { //Wait indefinitely for the lock
        retval = _name;
        releaseLock();
    }
    return retval;
}
