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
 * @brief Header file for the PowerReader class.
 */

#ifndef __POWER_READER_H
#define __POWER_READER_H

//Headers
#include <common.h>
#include <Runnable.h>

//Libraries
#include <cstdint>
#include <vector>
#include <string>

namespace xmem {

    /**
     * @brief An abstract base class for measuring power from an arbitrary source. This class is runnable using a worker thread.
     */
    class PowerReader : public Runnable {
    public:
        /**
         * @brief Constructor.
         * @param sampling_period The time between power samples in milliseconds.
         * @param power_units The power units for each sample in watts.
         * @param name The human-friendly name of this object.
         * @param cpu_affinity The logical CPU to be used by the thread calling this object's run() method. If negative, any CPU is OK (no affinity).
         */
        PowerReader(uint32_t sampling_period, double power_units, std::string name, int32_t cpu_affinity);

        /**
         * @brief Destructor.
         */
        ~PowerReader();

        /**
         * @brief Starts measuring power at the rate implied by the sampling_period passed in the constructor.
         * Call stop() to indicate to stop measuring.
         */
        virtual void run() = 0;

        /**
         * @brief Signals to stop measuring power.
         * This is a non-blocking call and return does not indicate the measurement has actually stopped.
         * @returns True if it successfully signaled a stop.
         */
        bool stop();

        /**
         * @brief Calculates the relevant metrics.
         * @returns True on success.
         */
        bool calculateMetrics(); 

        /**
         * @brief Clears the stored power data.
         * @returns True on success.
         */
        bool clear();
        
        /**
         * @brief Clears the stored power data and resets state so that a new thread can be used with this object.
         * @returns True on success.
         */
        bool clear_and_reset();

        /**
         * @brief Gets the power trace.
         * @returns The measured power trace in a vector. If no data was collected, the vector will be empty.
         */
        std::vector<double> getPowerTrace();

        /**
         * @brief Gets the average power.
         * @returns The average power from the measurements. If no data was collected, returns 0.
         */
        double getAveragePower();

        /**
         * @brief Gets the peak power.
         * @returns The peak power sample from the measurements. If no data was collected, returns 0.
         */
        double getPeakPower();

        /**
         * @brief Gets the last sample.
         * @returns The last power sample measured.
         */
        double getLastSample();

        /**
         * @brief Gets the sampling period.
         * @returns The sampling period of the measurements in milliseconds.
         */
        uint32_t getSamplingPeriod();

        /**
         * @brief Gets the units of samples in watts.
         * @returns The power units for each measurement sample in watts. For example, if each measurement is in milliwatts, then this returns 1e-3.
         */
        double getPowerUnits();

        /**
         * @brief Gets the number of samples collected.
         * @returns Number of samples collected.
         */
        size_t getNumSamples();

        /**
         * @brief Gets the name of this object.
         * @returns The human-friendly name of this PowerReader.
         */
        std::string name();

    protected:
        /////
        //ONLY ACCESS THESE WHILE HOLDING THIS OBJECT'S LOCK FOR THREAD SAFETY
        bool _stop_signal; /**< When true, the run() function should finish after the current sample iteration it is working on. */
        double _power_units; /**< Power units in watts. */
        std::string _name; /**< Name of this object. */
        int32_t _cpu_affinity; /**< CPU affinity for any thread using this object's run() method. If negative, no affinity preference. */
        std::vector<double> _power_trace; /**< The time-ordered list of power samples. The first index is the oldest measurement. */
        double _average_power; /**< The average power. */
        double _peak_power; /**< The peak power observed. */
        size_t _num_samples; /**< The number of samples collected. */
        uint32_t _sampling_period; /**< Power sampling period in milliseconds. */
        //
        /////
    };
};

#endif
