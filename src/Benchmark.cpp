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
 * @brief Implementation file for the Benchmark class.
 */

//Headers
#include <Benchmark.h>
#include <common.h>
#include <benchmark_kernels.h>
#include <PowerReader.h>

//Libraries
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <time.h>

using namespace xmem;

Benchmark::Benchmark(
        void* mem_array,
        size_t len,
        uint32_t iterations,
        uint32_t num_worker_threads,
        uint32_t mem_node,
        uint32_t cpu_node,
        pattern_mode_t pattern_mode,
        rw_mode_t rw_mode,
        chunk_size_t chunk_size,
        int32_t stride_size,
        std::vector<PowerReader*> dram_power_readers,
        std::string metric_units,
        std::string name
    ) :
        mem_array_(mem_array),
        len_(len),
        iterations_(iterations),
        num_worker_threads_(num_worker_threads),
        mem_node_(mem_node),
        cpu_node_(cpu_node),
        pattern_mode_(pattern_mode),
        rw_mode_(rw_mode),
        chunk_size_(chunk_size),
        stride_size_(stride_size),
        dram_power_readers_(dram_power_readers),
        dram_power_threads_(),
        metric_on_iter_(),
        mean_metric_(0),
        min_metric_(0),
        25_percentile_metric_(0),
        median_metric_(0),
        75_percentile_metric_(0),
        95_percentile_metric_(0),
        99_percentile_metric_(0),
        max_metric_(0),
        mode_metric_(0),
        metric_units_(metric_units),
        mean_dram_power_socket_(),
        peak_dram_power_socket_(),
        name_(name),
        obj_valid_(false),
        hasRun_(false),
        warning_(false)
    {
    
    for (uint32_t i = 0; i < iterations_; i++) 
        metric_on_iter_.push_back(-1);
}

Benchmark::~Benchmark() {
}

bool Benchmark::run() {
    if (has_run_) //A benchmark should only be run once per object
        return false;

    printBenchmarkHeader();
    reportBenchmarkInfo(); 

    //Write to all of the memory region of interest to make sure
    //pages are resident in physical memory and are not shared
    forwSequentialWrite_Word32(_mem_array,
                               reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_) + len_));

    bool success = runCore();
    if (success) {
        return true;
    } else {
        std::cerr << "WARNING: Benchmark " << name_ << " failed!" << std::endl;
        return false;
    }
}

void Benchmark::printBenchmarkHeader() const {
    //Spit out useful info
    std::cout << std::endl;
    std::cout << "-------- Running Benchmark: " << name_;
    std::cout << " ----------" << std::endl;
}

void Benchmark::reportBenchmarkInfo() const {
    std::cout << "CPU NUMA Node: " << cpu_node_ << std::endl;
    std::cout << "Memory NUMA Node: " << mem_node_ << std::endl;
    std::cout << "Chunk Size: ";
    switch (chunk_size_) {
        case CHUNK_32b:
            std::cout << "32-bit";
            break;
#ifdef HAS_WORD_64
        case CHUNK_64b:
            std::cout << "64-bit";
            break;
#endif
#ifdef HAS_WORD_128
        case CHUNK_128b:
            std::cout << "128-bit";
            break;
#endif
#ifdef HAS_WORD_256
        case CHUNK_256b:
            std::cout << "256-bit";
            break;
#endif
#ifdef HAS_WORD_512
        case CHUNK_512b:
            std::cout << "512-bit";
            break;
#endif
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << std::endl;

    std::cout << "Access Pattern: ";
    switch (pattern_mode_) {
        case SEQUENTIAL:
            if (stride_size_ > 0)
                std::cout << "forward ";
            else if (stride_size_ < 0)
                std::cout << "reverse ";
            else 
                std::cout << "UNKNOWN ";

            if (stride_size_ == 1 || stride_size_ == -1)
                std::cout << "sequential";
            else 
                std::cout << "strides of " << stride_size_ << " chunks";
            break;
        case RANDOM:
            std::cout << "random";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << std::endl;


    std::cout << "Read/Write Mode: ";
    switch (rw_mode_) {
        case READ:
            std::cout << "read";
            break;
        case WRITE:
            if (_pattern_mode == RANDOM) //special case
                std::cout << "read+write";
            else
                std::cout << "write";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << std::endl;

    std::cout << "Number of worker threads: " << num_worker_threads_;
    std::cout << std::endl;

    std::cout << std::endl;
}


void Benchmark::reportResults() const {
    std::cout << std::endl;
    std::cout << "*** RESULTS";
    std::cout << "***" << std::endl;
    std::cout << std::endl;
 
    if (hasRun_) {
        for (uint32_t i = 0; i < iterations_; i++) {
            std::printf("Iter #%4d:    %0.3f    %s", i, metric_on_iter_[i], metric_units_.c_str());
            //std::cout << "Iter #" << i << ": " << metric_on_iter_[i] << " " << metric_units_;
            if (warning_)
                std::cout << " (WARNING)";
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << std::endl;

        std::cout << "Mean: " << mean_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;

        std::cout << "Min: " << min_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "25th Percentile: " << 25_percentile_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Median: " << median_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "75th Percentile: " << 75_percentile_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "95th Percentile: " << 95_percentile_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "99th Percentile: " << 99_percentile_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Max: " << max_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
         
        std::cout << "Mode: " << mode_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
       
        std::cout << std::endl;
        std::cout << std::endl;
        
        for (uint32_t i = 0; i < dram_power_readers_.size(); i++) {
            if (dram_power_readers_[i] != NULL) {
                std::cout << dram_power_readers_[i]->name() << " Power Statistics..." << std::endl;
                std::cout << "...Mean Power: " << dram_power_readers_[i]->getMeanPower() * dram_power_readers_[i]->getPowerUnits() << " W" << std::endl;
                std::cout << "...Peak Power: " << dram_power_readers_[i]->getPeakPower() * dram_power_readers_[i]->getPowerUnits() << " W" << std::endl;
            }
        }
    }
    else
        std::cerr << "WARNING: Benchmark has not run yet. No reported results." << std::endl;
}

bool Benchmark::isValid() const { return obj_valid_; }

bool Benchmark::hasRun() const { return has_run_; }

double Benchmark::getMetricOnIter(uint32_t iter) const {
    if (has_run_ && iter - 1 <= iterations_)
        return metric_on_iter_[iter - 1];
    else //bad call
        return -1;
}

double Benchmark::getMeanMetric() const {
    if (has_run_)
        return mean_metric_;
    else //bad call
        return -1;
}

double Benchmark::getMinMetric() const {
    if (has_run_)
        return min_metric_;
    else //bad call
        return -1;
}

double Benchmark::get25PercentileMetric() const {
    if (has_run_)
        return 25_percentile_metric_;
    else //bad call
        return -1;
}

double Benchmark::getMedianMetric() const {
    if (has_run_)
        return median_metric_;
    else //bad call
        return -1;
}

double Benchmark::get75PercentileMetric() const {
    if (has_run_)
        return 75_percentile_metric_;
    else //bad call
        return -1;
}

double Benchmark::get95PercentileMetric() const {
    if (has_run_)
        return 95_percentile_metric_;
    else //bad call
        return -1;
}

double Benchmark::get99PercentileMetric() const {
    if (has_run_)
        return 99_percentile_metric_;
    else //bad call
        return -1;
}

double Benchmark::getMaxMetric() const {
    if (has_run_)
        return max_metric_;
    else //bad call
        return -1;
}

double Benchmark::getModeMetric() const {
    if (has_run_)
        return mode_metric_;
    else //bad call
        return -1;
}

std::string Benchmark::getMetricUnits() const {
    return metric_units_;
}
            
double Benchmark::getMeanDRAMPower(uint32_t socket_id) const {
    if (mean_dram_power_socket_.size() > socket_id)
        return mean_dram_power_socket_[socket_id];
    else
        return 0;
}

double Benchmark::getPeakDRAMPower(uint32_t socket_id) const {
    if (peak_dram_power_socket_.size() > socket_id)
        return peak_dram_power_socket_[socket_id];
    else
        return 0;
}

size_t Benchmark::getLen() const {
    return len_;
}

uint32_t Benchmark::getIterations() const {
    return iterations_;
}

chunk_size_t Benchmark::getChunkSize() const {
    return chunk_size_;
}

int32_t Benchmark::getStrideSize() const {
    return stride_size_;
}

uint32_t Benchmark::getCPUNode() const {
    return cpu_node_;
}

uint32_t Benchmark::getMemNode() const {
    return mem_node_;
}

uint32_t Benchmark::getNumThreads() const {
    return num_worker_threads_;
}

std::string Benchmark::getName() const {
    return name_;
}

pattern_mode_t Benchmark::getPatternMode() const {
    return pattern_mode_;
}

rw_mode_t Benchmark::getRWMode() const {
    return rw_mode_;
}

void Benchmark::computeMetrics() {
    if (has_run_) {
        //Compute mean
        mean_metric_ = 0;
        for (uint32_t i = 0; i < iterations_; i++)
            mean_metric_ += metric_on_iter_[i]; 
        mean_metric_ /= iterations_;

        //Build sorted array of metrics from each iteration
        std::vector<double> sortedMetrics = metric_on_iter_;;
        std::sort(sortedMetrics.begin(), sortedMetrics.end());

        //Compute percentiles
        min_metric_ = sortedMetrics.front();
        25_percentile_metric_ = sortedMetrics[sortedMetrics.size()/4];
        75_percentile_metric_ = sortedMetrics[sortedMetrics.size()*3/4];
        median_metric_ = sortedMetrics[sortedMetrics.size()/2];
        95_percentile_metric_ = sortedMetrics[sortedMetrics.size()*95/100];
        99_percentile_metric_ = sortedMetrics[sortedMetrics.size()*99/100];
        max_metric_ = sortedMetrics.back();

        //Compute mode
        std::map<double,uint32_t> metricCounts;
        for (uint32_t i = 0; i < iterations_; i++)
            metricCounts[metric_on_iter_[i]]++;
        mode_metric_ = 0;
        uint32_t greatest_count = 0;
        for (auto it = metricCounts.cbegin(); it != metricCounts.cend(); it++) {
            if (it->second > greatest_count)
                mode_metric_ = it->first;
        }
    }
}

bool Benchmark::startPowerThreads() {
    bool success = true;

    //Create all power threads
    for (uint32_t i = 0; i < dram_power_readers_.size(); i++) {
        Thread* mythread = NULL;
        if (dram_power_readers_[i] != NULL)  {
            dram_power_readers_[i]->clear_and_reset(); //clear the state of the reader
            mythread = new Thread(dram_power_readers_[i]);
            if (mythread == NULL) {
                std::cerr << "WARNING: Failed to allocate a DRAM power measurement thread." << std::endl;
                success = false;
            }
            else {
                dram_power_threads_.push_back(mythread);
                if (!dram_power_threads_[i]->create_and_start()) { //Create and start the power threads
                    std::cerr << "WARNING: Failed to create and start a DRAM power measurement thread." << std::endl;
                    success = false;
                }
            }
        }
    }

    return success;
}

bool Benchmark::stopPowerThreads() {
    bool success = true;

    //Indicate end to the power threads politely
    for (uint32_t i = 0; i < dram_power_threads_.size(); i++) {
        if (dram_power_threads_[i] != NULL) {
            if (dram_power_threads_[i]->started() && !dram_power_readers_[i]->stop()) {
                std::cerr << "WARNING: Failed to indicate end of power measurement to a power measurement object. The corresponding worker thread might not terminate." << std::endl;
                success = false;
            }
        }
    }

    //Wait for all worker threads to complete now that they were signaled to stop
    for (uint32_t i = 0; i < dram_power_threads_.size(); i++) {
        if (dram_power_threads_[i] != NULL) {
            if (!dram_power_threads_[i]->join()) { 
                std::cerr << "WARNING: A power measurement thread failed to join! Forcing the thread to stop." << std::endl;
                if (!dram_power_threads_[i]->cancel())
                    std::cerr << "WARNING: Failed to force stop a power measurement thread. Its behavior may be unpredictable." << std::endl;
            }
        }

        //Collect power data
        for (uint32_t i = 0; i < dram_power_readers_.size(); i++) {
            if (dram_power_readers_[i] != NULL) {
                mean_dram_power_socket_.push_back(dram_power_readers_[i]->getMeanPower() * dram_power_readers_[i]->getPowerUnits());
                peak_dram_power_socket_.push_back(dram_power_readers_[i]->getPeakPower() * dram_power_readers_[i]->getPowerUnits());
            }
        }
    }

    return success;
}
