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
#include <random>
#include <algorithm>

using namespace xmem;

Benchmark::Benchmark(
		void* mem_array,
		size_t len,
		uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t passes_per_iteration,
#endif
		uint32_t num_worker_threads,
		uint32_t mem_node,
		uint32_t cpu_node,
		pattern_mode_t pattern_mode,
		rw_mode_t rw_mode,
		chunk_size_t chunk_size,
		int64_t stride_size,
		Timer& timer,
		std::vector<PowerReader*> dram_power_readers,
		std::string metricUnits,
		std::string name
	) :
		_mem_array(mem_array),
		_len(len),
		_iterations(iterations),
#ifdef USE_SIZE_BASED_BENCHMARKS
		_passes_per_iteration(passes_per_iteration),
#endif
		_num_worker_threads(num_worker_threads),
		_mem_node(mem_node),
		_cpu_node(cpu_node),
		_pattern_mode(pattern_mode),
		_rw_mode(rw_mode),
		_chunk_size(chunk_size),
		_stride_size(stride_size),
		_timer(timer),
		_dram_power_readers(dram_power_readers),
		_dram_power_threads(),
		_metricOnIter(),
		_averageMetric(0),
		_metricUnits(metricUnits),
		_average_dram_power_socket(),
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

	return _run_core();
}

void Benchmark::report_benchmark_info() const {
	std::cout << "CPU NUMA Node: " << _cpu_node << std::endl;
	std::cout << "Memory NUMA Node: " << _mem_node << std::endl;
	std::cout << "Chunk Size: ";
	switch (_chunk_size) {
		case CHUNK_32b:
			std::cout << "32-bit";
			break;
		case CHUNK_64b:
			std::cout << "64-bit";
			break;
		case CHUNK_128b:
			std::cout << "128-bit";
			break;
		case CHUNK_256b:
			std::cout << "256-bit";
			break;
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
			std::cout << "write";
			break;
		default:
			std::cout << "UNKNOWN";
			break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}


void Benchmark::report_results() const {
	std::cout << std::endl;
	std::cout << "*** RESULTS";
	std::cout << "***" << std::endl;
	std::cout << std::endl;
 
	if (_hasRun) {
		for (uint32_t i = 0; i < _iterations; i++)
			std::cout << "Iter #" << i + 1 << ": " << _metricOnIter[i] << " " << _metricUnits << std::endl;
		std::cout << "Average: " << _averageMetric << " " << _metricUnits << std::endl;
		if (_warning) std::cout << " (WARNING)";
		std::cout << std::endl;
		
		for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
			if (_dram_power_readers[i] != NULL) {
				std::cout << _dram_power_readers[i]->name() << " Power Statistics..." << std::endl;
				std::cout << "...Average Power: " << _dram_power_readers[i]->getAveragePower() * _dram_power_readers[i]->getPowerUnits() << " W" << std::endl;
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

double Benchmark::getAverageMetric() const {
	if (_hasRun)
		return _averageMetric;
	else //bad call
		return -1;
}
			
double Benchmark::getAverageDRAMPower(uint32_t socket_id) const {
	if (_average_dram_power_socket.size() > socket_id)
		return _average_dram_power_socket[socket_id];
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

#ifdef USE_SIZE_BASED_BENCHMARKS
uint64_t Benchmark::getPassesPerIteration() const {
	return _passes_per_iteration;
}
#endif

chunk_size_t Benchmark::getChunkSize() const {
	return _chunk_size;
}

int64_t Benchmark::getStrideSize() const {
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

bool Benchmark::_start_power_threads() {
	bool success = true;

	//Create all power threads
	for (uint32_t i = 0; i < _dram_power_readers.size(); i++) {
		if (i >= _dram_power_threads.capacity())
			_dram_power_threads.reserve(_dram_power_threads.capacity() + 1);
		Thread* mythread = NULL;
		if (_dram_power_readers[i] != NULL)  {
			_dram_power_readers[i]->clear_and_reset(); //clear the state of the reader
			mythread = new Thread(_dram_power_readers[i]);
		}
		_dram_power_threads.push_back(mythread);
		if (mythread == NULL) {
			std::cerr << "WARNING: Failed to allocate a DRAM power measurement thread." << std::endl;
			success = false;
		}
		else {
			if (!_dram_power_threads[i]->create_and_start()) { //Create and start the power threads
				std::cerr << "WARNING: Failed to create and start a DRAM power measurement thread." << std::endl;
				success = false;
			}
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
		if (_dram_power_threads[i] != NULL && !_dram_power_threads[i]->join()) { //Give 3 seconds maximum to terminate each power thread.
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

bool Benchmark::_buildRandomPointerPermutation() {
	if (g_verbose)
		std::cout << "Preparing memory region under test. This might take a while...";

	size_t num_pointers; //Number of pointers that fit into the memory region of interest
	switch (_chunk_size) {
		case CHUNK_64b:
			num_pointers = _len / sizeof(Word64_t);
			break;
		case CHUNK_128b:
			num_pointers = _len / sizeof(Word128_t);
			break;
		case CHUNK_256b:
			num_pointers = _len / sizeof(Word256_t);
			break;
		default:
			std::cerr << "ERROR: Chunk size must be at least 64 bits for pointer-chasing algorithms. This should not have happened." << std::endl;
			return false;
	}
			
	uintptr_t* mem_base = static_cast<uintptr_t*>(_mem_array); 

	std::mt19937_64 gen(time(NULL)); //Mersenne Twister random number generator, seeded at current time
	
#ifdef USE_LATENCY_BENCHMARK_RANDOM_HAMILTONIAN_CYCLE_PATTERN
	//Build a random directed Hamiltonian Cycle across the entire memory

	//Let W be the list of memory locations that have not been reached yet. Each entry is an index in mem_base.
	std::vector<size_t> W;
	size_t w_index = 0;

	//Initialize W to contain all memory locations, where each memory location appears exactly once in the list. The order does not strictly matter.
	W.resize(num_pointers);
	for (w_index = 0; w_index < num_pointers; w_index++) {
		W.at(w_index) = w_index;
	}
	
	//Build the directed Hamiltonian Cycle
	size_t v = 0; //the current memory location. Always start at the first location for the Hamiltonian Cycle construction
	size_t w = 0; //the next memory location
	w_index = 0;
	while (W.size() > 0) { //while we have not reached all memory locations
		W.erase(W.begin() + w_index);

		//Normal case
		if (W.size() > 0) {
			//Choose the next w_index at random from W
			w_index = gen() % W.size();

			//Extract the memory location corresponding to this w_index
			w = W[w_index];
		} else { //Last element visited needs to point at head of memory to complete the cycle
			w = 0;
		}

		//Create pointer v --> w. This corresponds to a directed edge in the graph with nodes v and w.
		mem_base[v] = reinterpret_cast<uintptr_t>(mem_base + w);

		//Chase this pointer to move to next step
		v = w;
	}
#endif

#ifdef USE_LATENCY_BENCHMARK_RANDOM_SHUFFLE_PATTERN
	//Do a random shuffle of the indices
	for (size_t i = 0; i < num_pointers; i++) //Initialize pointers to point at themselves (identity mapping)
		mem_base[i] = reinterpret_cast<uintptr_t>(mem_base+static_cast<uintptr_t>(i));

	std::shuffle(mem_base, mem_base + num_pointers, gen);
#endif
	if (g_verbose) {
		std::cout << "done" << std::endl;
		std::cout << std::endl;
	}

	return true;
}
