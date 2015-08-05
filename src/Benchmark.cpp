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
        std::string metricUnits,
        std::string name
    ) :
        _mem_array(mem_array),
        _len(len),
        _iterations(iterations),
        _num_worker_threads(num_worker_threads),
        _mem_node(mem_node),
        _cpu_node(cpu_node),
        _pattern_mode(pattern_mode),
        _rw_mode(rw_mode),
        _chunk_size(chunk_size),
        _stride_size(stride_size),
        _dram_power_readers(dram_power_readers),
        _dram_power_threads(),
        _metricOnIter(),
        _meanMetric(0),
        _minMetric(0),
        _25PercentileMetric(0),
        _medianMetric(0),
        _75PercentileMetric(0),
        _95PercentileMetric(0),
        _99PercentileMetric(0),
        _maxMetric(0),
        _modeMetric(0),
        _metricUnits(metricUnits),
        _mean_dram_power_socket(),
        _peak_dram_power_socket(),
        _name(name),
        _obj_valid(false),
        _hasRun(false),
        _warning(false)
    {
    
    for (uint32_t i = 0; i < _iterations; i++) 
        _metricOnIter.push_back(-1);
}

Benchmark::~Benchmark() {
}

bool Benchmark::run() {
    if (_hasRun) //A benchmark should only be run once per object
        return false;

    print_benchmark_header();
    report_benchmark_info(); 

    //Write to all of the memory region of interest to make sure
    //pages are resident in physical memory and are not shared
    forwSequentialWrite_Word32(_mem_array,
                               reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_mem_array) + _len));

    bool success = _run_core();
    if (success) {
        return true;
    } else {
        std::cerr << "WARNING: Benchmark " << _name << " failed!" << std::endl;
        return false;
    }
}

void Benchmark::print_benchmark_header() const {
    //Spit out useful info
    std::cout << std::endl;
    std::cout << "-------- Running Benchmark: " << _name;
    std::cout << " ----------" << std::endl;
}

void Benchmark::report_benchmark_info() const {
    std::cout << "CPU NUMA Node: " << _cpu_node << std::endl;
    std::cout << "Memory NUMA Node: " << _mem_node << std::endl;
    std::cout << "Chunk Size: ";
    switch (_chunk_size) {
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
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << std::endl;

    std::cout << "Access Pattern: ";
    switch (_pattern_mode) {
        case SEQUENTIAL:
            if (_stride_size > 0)
                std::cout << "forward ";
            else if (_stride_size < 0)
                std::cout << "reverse ";
            else 
                std::cout << "UNKNOWN ";

            if (_stride_size == 1 || _stride_size == -1)
                std::cout << "sequential";
            else 
                std::cout << "strides of " << _stride_size << " chunks";
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
    switch (_rw_mode) {
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

    std::cout << "Number of worker threads: " << _num_worker_threads;
    std::cout << std::endl;

    std::cout << std::endl;
}


void Benchmark::report_results() const {
    std::cout << std::endl;
    std::cout << "*** RESULTS";
    std::cout << "***" << std::endl;
    std::cout << std::endl;
 
    if (_hasRun) {
        for (uint32_t i = 0; i < _iterations; i++) {
            std::cout << "Iter #" << i + 1 << ": " << _metricOnIter[i] << " " << _metricUnits;
            if (_warning)
                std::cout << " (WARNING)";
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << std::endl;

        std::cout << "Mean: " << _meanMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;

        std::cout << "Min: " << _minMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "25th Percentile: " << _25PercentileMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Median: " << _medianMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "75th Percentile: " << _75PercentileMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "95th Percentile: " << _95PercentileMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "99th Percentile: " << _99PercentileMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Max: " << _maxMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
         
        std::cout << "Mode: " << _modeMetric << " " << _metricUnits;
        if (_warning)
            std::cout << " (WARNING)";
        std::cout << std::endl;
       
        std::cout << std::endl;
        std::cout << std::endl;
        
        for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
            if (_dram_power_readers[i] != NULL) {
                std::cout << _dram_power_readers[i]->name() << " Power Statistics..." << std::endl;
                std::cout << "...Mean Power: " << _dram_power_readers[i]->getMeanPower() * _dram_power_readers[i]->getPowerUnits() << " W" << std::endl;
                std::cout << "...Peak Power: " << _dram_power_readers[i]->getPeakPower() * _dram_power_readers[i]->getPowerUnits() << " W" << std::endl;
            }
        }
    }
    else
        std::cerr << "WARNING: Benchmark has not run yet. No reported results." << std::endl;
}

bool Benchmark::isValid() const { return _obj_valid; }

bool Benchmark::hasRun() const { return _hasRun; }

double Benchmark::getMetricOnIter(uint32_t iter) const {
    if (_hasRun && iter - 1 <= _iterations)
        return _metricOnIter[iter - 1];
    else //bad call
        return -1;
}

double Benchmark::getMeanMetric() const {
    if (_hasRun)
        return _meanMetric;
    else //bad call
        return -1;
}

double Benchmark::getMinMetric() const {
    if (_hasRun)
        return _minMetric;
    else //bad call
        return -1;
}

double Benchmark::get25PercentileMetric() const {
    if (_hasRun)
        return _25PercentileMetric;
    else //bad call
        return -1;
}

double Benchmark::getMedianMetric() const {
    if (_hasRun)
        return _medianMetric;
    else //bad call
        return -1;
}

double Benchmark::get75PercentileMetric() const {
    if (_hasRun)
        return _75PercentileMetric;
    else //bad call
        return -1;
}

double Benchmark::get95PercentileMetric() const {
    if (_hasRun)
        return _95PercentileMetric;
    else //bad call
        return -1;
}

double Benchmark::get99PercentileMetric() const {
    if (_hasRun)
        return _99PercentileMetric;
    else //bad call
        return -1;
}

double Benchmark::getMaxMetric() const {
    if (_hasRun)
        return _maxMetric;
    else //bad call
        return -1;
}

double Benchmark::getModeMetric() const {
    if (_hasRun)
        return _modeMetric;
    else //bad call
        return -1;
}

std::string Benchmark::getMetricUnits() const {
    return _metricUnits;
}
            
double Benchmark::getMeanDRAMPower(uint32_t socket_id) const {
    if (_mean_dram_power_socket.size() > socket_id)
        return _mean_dram_power_socket[socket_id];
    else
        return 0;
}

double Benchmark::getPeakDRAMPower(uint32_t socket_id) const {
    if (_peak_dram_power_socket.size() > socket_id)
        return _peak_dram_power_socket[socket_id];
    else
        return 0;
}

size_t Benchmark::getLen() const {
    return _len;
}

uint32_t Benchmark::getIterations() const {
    return _iterations;
}

chunk_size_t Benchmark::getChunkSize() const {
    return _chunk_size;
}

int32_t Benchmark::getStrideSize() const {
    return _stride_size;
}

uint32_t Benchmark::getCPUNode() const {
    return _cpu_node;
}

uint32_t Benchmark::getMemNode() const {
    return _mem_node;
}

uint32_t Benchmark::getNumThreads() const {
    return _num_worker_threads;
}

std::string Benchmark::getName() const {
    return _name;
}

pattern_mode_t Benchmark::getPatternMode() const {
    return _pattern_mode;
}

rw_mode_t Benchmark::getRWMode() const {
    return _rw_mode;
}

void Benchmark::_computeMetrics() {
    if (_hasRun) {
        //Compute mean
        _meanMetric = 0;
        for (uint32_t i = 0; i < _iterations; i++)
            _meanMetric += _metricOnIter[i]; 
        _meanMetric /= _iterations;

        //Build sorted array of metrics from each iteration
        std::vector<double> sortedMetrics = _metricOnIter;;
        std::sort(sortedMetrics.begin(), sortedMetrics.end());

        //Compute percentiles
        _minMetric = sortedMetrics.front();
        _25PercentileMetric = sortedMetrics[sortedMetrics.size()/4];
        _75PercentileMetric = sortedMetrics[sortedMetrics.size()*3/4];
        _medianMetric = sortedMetrics[sortedMetrics.size()/2];
        _95PercentileMetric = sortedMetrics[sortedMetrics.size()*95/100];
        _99PercentileMetric = sortedMetrics[sortedMetrics.size()*99/100];
        _maxMetric = sortedMetrics.back();

        //Compute mode
        std::map<double,uint32_t> metricCounts;
        for (uint32_t i = 0; i < _iterations; i++)
            metricCounts[_metricOnIter[i]]++;
        _modeMetric = 0;
        uint32_t greatest_count = 0;
        for (auto it = metricCounts.cbegin(); it != metricCounts.cend(); it++) {
            if (it->second > greatest_count)
                _modeMetric = it->first;
        }
    }
}

bool Benchmark::_start_power_threads() {
    bool success = true;

    //Create all power threads
    for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
        Thread* mythread = NULL;
        if (_dram_power_readers[i] != NULL)  {
            _dram_power_readers[i]->clear_and_reset(); //clear the state of the reader
            mythread = new Thread(_dram_power_readers[i]);
            if (mythread == NULL) {
                std::cerr << "WARNING: Failed to allocate a DRAM power measurement thread." << std::endl;
                success = false;
            }
            else {
                _dram_power_threads.push_back(mythread);
                if (!_dram_power_threads[i]->create_and_start()) { //Create and start the power threads
                    std::cerr << "WARNING: Failed to create and start a DRAM power measurement thread." << std::endl;
                    success = false;
                }
            }
        }
    }

    return success;
}

bool Benchmark::_stop_power_threads() {
    bool success = true;

    //Indicate end to the power threads politely
    for (uint32_t i = 0; i < _dram_power_threads.size(); i++) {
        if (_dram_power_threads[i] != NULL) {
            if (_dram_power_threads[i]->started() && !_dram_power_readers[i]->stop()) {
                std::cerr << "WARNING: Failed to indicate end of power measurement to a power measurement object. The corresponding worker thread might not terminate." << std::endl;
                success = false;
            }
        }
    }

    //Wait for all worker threads to complete now that they were signaled to stop
    for (uint32_t i = 0; i < _dram_power_threads.size(); i++) {
        if (_dram_power_threads[i] != NULL) {
            if (!_dram_power_threads[i]->join()) { 
                std::cerr << "WARNING: A power measurement thread failed to join! Forcing the thread to stop." << std::endl;
                if (!_dram_power_threads[i]->cancel())
                    std::cerr << "WARNING: Failed to force stop a power measurement thread. Its behavior may be unpredictable." << std::endl;
            }
        }

        //Collect power data
        for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
            if (_dram_power_readers[i] != NULL) {
                _mean_dram_power_socket.push_back(_dram_power_readers[i]->getMeanPower() * _dram_power_readers[i]->getPowerUnits());
                _peak_dram_power_socket.push_back(_dram_power_readers[i]->getPeakPower() * _dram_power_readers[i]->getPowerUnits());
            }
        }
    }

    return success;
}
