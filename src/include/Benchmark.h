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
 * @brief Header file for the Benchmark class.
 */

#ifndef __BENCHMARK_H
#define __BENCHMARK_H

//Headers
#include <common.h>
#include <Timer.h>

#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
#include <x86_64/TSCTimer.h>
#endif

#if defined(_WIN32) && defined(USE_QPC_TIMER)
#include <win/QPCTimer.h>
#endif

#include <PowerReader.h>
#include <Thread.h>
#include <Runnable.h>

//Libraries
#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
#include <intrin.h>
#endif

namespace xmem {

	/**
	 * @brief Flexible abstract class for any memory benchmark.
	 *
	 * This class provides a generic interface for interacting with a benchmark. All benchmarks should be derived
	 * from this class.
	 */
	class Benchmark {
	public:
		/**
		 * @brief Constructor.
		 * @param mem_array A pointer to a contiguous chunk of memory that has been allocated for benchmarking among potentially several worker threads. This should be aligned to a 256-bit boundary.
		 * @param len Length of mem_array in bytes. This must be a multiple of 4 KB and should be at least the per-thread working set size times the number of worker threads.
		 * @param iterations Number of iterations of the complete benchmark. Used to average results and provide a measure of consistency and reproducibility.
		 * @param passes_per_iteration Number of passes to do in each iteration, to ensure timed section of code is "long enough".
		 * @param num_worker_threads The number of worker threads to use in the benchmark.
		 * @param mem_node The logical memory NUMA node used in the benchmark.
		 * @param cpu_node The logical CPU NUMA node to use for the benchmark.
		 * @param pattern_mode This indicates the general type of access pattern used, e.g. sequential or random.
		 * @param rw_mode This indicates the general type of read/write mix used, e.g. pure reads or pure writes.
		 * @param chunk_size Size of an individual memory access for load-generating worker threads.
		 * @param stride_size For sequential access patterns, this is the address distance between successive accesses, counted in chunks. Negative values indicate a reversed access pattern. A stride of +/-1 is purely sequential.
		 * @param timer A Timer object for benchmarking.
		 * @param dram_power_readers A group of PowerReader objects for measuring DRAM power.
		 * @param name The name of the benchmark to use when reporting to console.
		 */
		Benchmark(
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
		);

		/**
		 * @brief Destructor.
		 */
		virtual ~Benchmark();

		/**
		 * @brief Runs the benchmark.
		 * @returns True on benchmark success
		 */
		bool run();

		/**
		 * @brief Reports benchmark configuration details to the console.
		 */
		void report_benchmark_info() const;

		/**
		 * @brief Reports results to the console.
		 */
		void report_results() const;

		/**
		 * @brief Checks to see that the object is in a valid state.
		 * @returns True if the object was constructed correctly and can be used.
		 */
		bool isValid() const;

		/**
		 * @brief Checks to see if the benchmark has run.
		 * @returns True if run() has already completed successfully.
		 */
		bool hasRun() const;

		/**
		 * @brief Extracts the metric of interest for a given iteration. Units are interpreted by the inheriting class.
		 * @param iter Iteration to extract.
		 * @returns The metric on the iteration specified by the input.
		 */
		double getMetricOnIter(uint32_t iter) const;

		/**
		 * @brief Gets the average benchmark metric across all iterations.
		 * @returns The average metric.
		 */
		double getAverageMetric() const;

		/**
		 * @brief Gets the average DRAM power over the benchmark.
		 * @returns The average DRAM power for a given socket in watts, or 0 if the data does not exist (power was unable to be collected or the benchmark has not run).
		 */
		double getAverageDRAMPower(uint32_t socket_id) const;
		
		/**
		 * @brief Gets the peak DRAM power over the benchmark.
		 * @returns The peak DRAM power for a given socket in watts, or 0 if the data does not exist (power was unable to be collected or the benchmark has not run).
		 */
		double getPeakDRAMPower(uint32_t socket_id) const;

		/**
		 * @brief Gets the length of the memory region in bytes. This is not necessarily the "working set size" depending on multithreading configuration.
		 * @returns Length of the memory region in bytes.
		 */
		size_t getLen() const;

		/**
		 * @brief Gets the number of iterations for this benchmark.
		 * @returns The number of iterations for this benchmark.
		 */
		uint32_t getIterations() const;

#ifdef USE_SIZE_BASED_BENCHMARKS
		/**
		 * @brief Gets the number of passes in each iteration.
		 * @returns The number of passes per iteration for this benchmark.
		 */
		uint64_t getPassesPerIteration() const;
#endif

		/**
		 * @brief Gets the width of memory access used in this benchmark.
		 * @returns The chunk size for this benchmark.
		 */
		chunk_size_t getChunkSize() const;

		/**
		 * @brief Gets the stride size for this benchmark.
		 * @returns The stride size in chunks.
		 */
		int64_t getStrideSize() const;

		/**
		 * @brief Gets the CPU NUMA node used in this benchmark.
		 * @returns The NUMA CPU node used in this benchmark.
		 */
		uint32_t getCPUNode() const;

		/**
		 * @brief Gets the memory NUMA node used in this benchmark.
		 * @returns The NUMA memory node used in this benchmark.
		 */
		uint32_t getMemNode() const;

		/**
		 * @brief Gets the number of worker threads used in this benchmark.
		 * @returns The number of worker threads used in this benchmark.
		 */
		uint32_t getNumThreads() const;

		/**
		 * @brief Gets the human-friendly name of this benchmark.
		 * @returns The benchmark test name.
		 */
		std::string getName() const;

		/**
		 * @brief Gets the pattern mode for this benchmark.
		 * @returns The pattern mode enumerator.
		 */
		pattern_mode_t getPatternMode() const;

		/**
		 * @brief Gets the read/write mode for this benchmark.
		 * @returns The read/write mix mode.
		 */
		rw_mode_t getRWMode() const;

	protected:
	
		/**
		 * @brief The core benchmark function. 
		 * @returns True on success.
		 */
		virtual bool _run_core() = 0;

		/**
		 * @brief Builds a random chain of pointers within the memory region under test.
		 * @returns True on success.
		 */
		bool _buildRandomPointerPermutation();

		/**
		 * @brief Starts the DRAM power measurement threads.
		 * @returns True on success.
		 */
		bool _start_power_threads();

		/**
		 * @brief Stops the DRAM power measurement threads.
		 * This is a blocking call.
		 * @returns True on success.
		 */
		bool _stop_power_threads();


		//Memory region under test
		void* _mem_array; /**< Pointer to the memory region to use in this benchmark. */
		size_t _len; /**< Length of the memory region in bytes. This is not the working set size per thread! */

		//Benchmark repetition
		uint32_t _iterations; /**< Number of iterations used in this benchmark. */
#ifdef USE_SIZE_BASED_BENCHMARKS
		uint64_t _passes_per_iteration; /**< Number of passes per iteration in this benchmark. */
#endif
	
		//Threading and affinity
		uint32_t _num_worker_threads; /**< The number of worker threads used in this benchmark. */
		uint32_t _mem_node; /**< The memory NUMA node used in this benchmark. */
		uint32_t _cpu_node; /**< The CPU NUMA node used in this benchmark. */
		
		//Benchmarking settings
		pattern_mode_t _pattern_mode; /**< Access pattern mode. */
		rw_mode_t _rw_mode; /**< Read/write mode. */
		chunk_size_t _chunk_size; /**< Chunk size of memory accesses in this benchmark. */
		int64_t _stride_size; /**< Stride size in chunks for sequential pattern mode only. */

		//Benchmark timing
		Timer& _timer; /**< The reference timer for this benchmark. */
		
		//Power measurement
		std::vector<PowerReader*> _dram_power_readers; /**< The power reading objects for measuring DRAM power on a per-socket basis during the benchmark. */
		std::vector<Thread*> _dram_power_threads; /**< The power reading threads for measuring DRAM power on a per-socket basis during the benchmark. These work with the DRAM power readers. Although they are worker threads, they are not counted as the "official" benchmarking worker threads. */
		
		//Benchmark results
		std::vector<double> _metricOnIter; /**< Metrics for each iteration of the benchmark. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
		double _averageMetric; /**< Average metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
		std::string _metricUnits; /**< String representing the units of measurement for the metric. */
		std::vector<double> _average_dram_power_socket; /**< The average DRAM power in this benchmark, per socket. */
		std::vector<double> _peak_dram_power_socket; /**< The peak DRAM power in this benchmark, per socket. */

		//Metadata
		std::string _name; /**< Name of this benchmark. */
		bool _obj_valid; /**< Indicates whether this benchmark object is valid. */
		bool _hasRun; /**< Indicates whether the benchmark has run. */
		bool _warning; /**< Indicates whether the benchmarks results might be clearly questionable/inaccurate/incorrect due to a variety of factors. */
	};
};

#endif
