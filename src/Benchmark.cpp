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
 * @brief Implementation file for the Benchmark class.
 */

//Headers
#include <Benchmark.h>
#include <common.h>
#include <PowerReader.h>

//Libraries
#include <cstdint>
#include <iostream>
#include <vector>

using namespace xmem::benchmark;
using namespace xmem::common;

Benchmark::Benchmark(
	void* mem_array,
	size_t len,
	uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
	uint64_t passes_per_iteration,
#endif
	chunk_size_t chunk_size,
	uint32_t cpu_node,
	uint32_t mem_node,
	uint32_t num_worker_threads,
	std::string name,
	xmem::timers::Timer *timer,
	std::vector<xmem::power::PowerReader*> dram_power_readers
	) :
	_mem_array(mem_array),
	_len(len),
	_iterations(iterations),
#ifdef USE_SIZE_BASED_BENCHMARKS
	_passes_per_iteration(passes_per_iteration),
#endif
	_chunk_size(chunk_size),
	_indices(nullptr),
	_cpu_node(cpu_node),
	_mem_node(mem_node),
	_num_worker_threads(num_worker_threads),
	_timer(timer),
	_dram_power_readers(dram_power_readers),
	_dram_power_threads(),
	_average_dram_power_socket(),
	_peak_dram_power_socket(),
	_hasRun(false),
	_metricOnIter(),
	_averageMetric(0),
	_name(name),
	_warning(false)
	{
	for (uint32_t i = 0; i < _iterations; i++) 
		_metricOnIter.push_back(-1);
}

Benchmark::~Benchmark() {
	if (_indices != nullptr)
		delete[] _indices;
}

void Benchmark::report_power_results() {
	for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
		if (_dram_power_readers[i] != NULL) {
			std::cout << _dram_power_readers[i]->name() << " Power Statistics..." << std::endl;
			std::cout << "...Average Power: " << _dram_power_readers[i]->getAveragePower() * _dram_power_readers[i]->getPowerUnits() << " W" << std::endl;
			std::cout << "...Peak Power: " << _dram_power_readers[i]->getPeakPower() * _dram_power_readers[i]->getPowerUnits() << " W" << std::endl;
		}
	}
}

bool Benchmark::isValid() { return _obj_valid; }

bool Benchmark::hasRun() { return _hasRun; }

double Benchmark::getMetricOnIter(uint32_t iter) {
	if (_hasRun && iter - 1 <= _iterations)
		return _metricOnIter[iter - 1];
	else //bad call
		return -1;
}

double Benchmark::getAverageMetric() {
	if (_hasRun)
		return _averageMetric;
	else //bad call
		return -1;
}
			
double Benchmark::getAverageDRAMPower(uint32_t socket_id) {
	if (_average_dram_power_socket.size() > socket_id)
		return _average_dram_power_socket[socket_id];
	else
		return 0;
}

double Benchmark::getPeakDRAMPower(uint32_t socket_id) {
	if (_peak_dram_power_socket.size() > socket_id)
		return _peak_dram_power_socket[socket_id];
	else
		return 0;
}

size_t Benchmark::getLen() {
	return _len;
}

uint32_t Benchmark::getIterations() {
	return _iterations;
}

#ifdef USE_SIZE_BASED_BENCHMARKS
uint64_t Benchmark::getPassesPerIteration() {
	return _passes_per_iteration;
}
#endif

xmem::common::chunk_size_t Benchmark::getChunkSize() {
	return _chunk_size;
}

uint32_t Benchmark::getCPUNode() {
	return _cpu_node;
}

uint32_t Benchmark::getMemNode() {
	return _mem_node;
}

uint32_t Benchmark::getNumThreads() {
	return _num_worker_threads;
}

std::string Benchmark::getName() {
	return _name;
}

bool Benchmark::_start_power_threads() {
	bool success = true;

	//Create all power threads
	for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
		if (i >= _dram_power_threads.capacity())
			_dram_power_threads.reserve(_dram_power_threads.capacity() + 1);
		thread::Thread* mythread = NULL;
		if (_dram_power_readers[i] != NULL)  {
			_dram_power_readers[i]->clear_and_reset(); //clear the state of the reader
			mythread = new thread::Thread(_dram_power_readers[i]);
		}
		_dram_power_threads.push_back(mythread);
		if (mythread == NULL) {
			std::cerr << "WARNING: Failed to create a DRAM power measurement thread." << std::endl;
			success = false;
		}
		else {
			_dram_power_threads[i]->create();
		}
	}

	//Start the power threads
	for (uint32_t i = 0; i < _dram_power_threads.size(); i++) {
		if (_dram_power_threads[i] != NULL && !_dram_power_threads[i]->start()) {
			std::cerr << "WARNING: Failed to start a DRAM power measurement thread." << std::endl;
			success = false;
		}
	}

	return success;
}

bool Benchmark::_stop_power_threads() {
	bool success = true;

	//Indicate end to the power threads politely
	for (uint32_t i = 0; i < _dram_power_threads.size(); i++) {
		if (_dram_power_threads[i] != NULL && _dram_power_threads[i]->started()) {
			if (!_dram_power_readers[i]->stop()) {
				std::cerr << "WARNING: Failed to indicate end of power measurement to a power measurement object. The corresponding worker thread might not terminate." << std::endl;
				success = false;
			}
		}
	}

	//Wait for all worker threads to complete now that they were signaled to stop
	for (uint32_t i = 0; i < _dram_power_threads.size(); i++) {
		if (_dram_power_threads[i] != NULL && !_dram_power_threads[i]->join(DEFAULT_THREAD_JOIN_TIMEOUT)) { //Give 3 seconds maximum to terminate each power thread.
			std::cerr << "WARNING: A power measurement thread did not complete on time as expected! Forcing the thread to stop." << std::endl;
			if (!_dram_power_threads[i]->cancel())
				std::cerr << "WARNING: Failed to force stop a power measurement thread. Its behavior may be unpredictable." << std::endl;
		}

		//Collect power data
		_average_dram_power_socket.reserve(_dram_power_readers.size());
		_peak_dram_power_socket.reserve(_dram_power_readers.size());
		for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
			if (_dram_power_readers[i] != NULL) {
				_average_dram_power_socket.push_back(_dram_power_readers[i]->getAveragePower() * _dram_power_readers[i]->getPowerUnits());
				_peak_dram_power_socket.push_back(_dram_power_readers[i]->getPeakPower() * _dram_power_readers[i]->getPowerUnits());
			}
		}
	}

	return success;
}
