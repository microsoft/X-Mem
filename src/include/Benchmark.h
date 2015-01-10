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

namespace xmem {
	namespace benchmark {
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
			 * @param mem_array a pointer to a contiguous chunk of memory that has been allocated for benchmarking among the worker threads. This should be aligned to a 256-bit boundary and should be the working set size times number of threads large.
			 * @param len Length of the raw_mem_array in bytes. This should be a multiple of 4 KB pages.
			 * @param iterations Number of iterations to do of the complete benchmark, to average out results.
			 * @param passes_per_iteration Number of passes to do in each iteration, to ensure timed section of code is "long enough".
			 * @param chunk_size encoded size of an individual memory access.
			 * @param cpu_node the logical CPU NUMA node to use for the benchmark
			 * @param mem_node the logical memory NUMA node used in the benchmark
			 * @param num_worker_threads number of worker threads to use in the benchmark
			 * @param name name of the benchmark to use when reporting
			 * @param timer pointer to an existing Timer
			 * @param dram_power_readers vector of pointers to PowerReader objects for measuring DRAM power
			 */
			Benchmark(
				void* mem_array,
				size_t len,
				uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
				uint64_t passes_per_iteration,
#endif
				xmem::common::chunk_size_t chunk_size,
				uint32_t cpu_node,
				uint32_t mem_node,
				uint32_t num_worker_threads,
				std::string name,
				xmem::timers::Timer *timer,
				std::vector<xmem::power::PowerReader*> dram_power_readers
			);

			/**
			 * @brief Destructor.
			 */
			~Benchmark();

			/**
			 * @brief Runs the benchmark.
			 * @returns true on benchmark success
			 */
			virtual bool run() = 0;

			/**
			 * @brief Reports benchmark configuration details to the console.
			 */
			virtual void report_benchmark_info() = 0;

			/**
			 * @brief Reports results to the console.
			 */
			virtual void report_results() = 0;

			/**
			 * @brief Reports power measurement results to the console.
			 */
			void report_power_results();

			/**
			 * @brief Checks to see that the object is in a valid state.
			 * @returns True if the object was constructed correctly and can be used.
			 */
			bool isValid();

			/**
			 * @brief Checks to see if the benchmark has run.
			 * @returns True if run() has already completed successfully.
			 */
			bool hasRun();

			/**
			 * @brief Extracts the metric of interest for a given iteration. Units are interpreted by the inheriting class.
			 * @param iter Iteration to extract.
			 * @returns The metric on the iteration specified by the input.
			 */
			double getMetricOnIter(uint32_t iter);

			/**
			 * @brief Gets the average benchmark metric across all iterations.
			 * @returns The average metric.
			 */
			double getAverageMetric();

			/**
			 * @brief Gets the average DRAM power over the benchmark.
			 * @returns The average DRAM power for a given socket in watts, or 0 if the data does not exist (power was unable to be collected or the benchmark has not run).
			 */
			double getAverageDRAMPower(uint32_t socket_id);
			
			/**
			 * @brief Gets the peak DRAM power over the benchmark.
			 * @returns The peak DRAM power for a given socket in watts, or 0 if the data does not exist (power was unable to be collected or the benchmark has not run).
			 */
			double getPeakDRAMPower(uint32_t socket_id);

			/**
			 * @brief Gets the length of the memory region in bytes. This is not necessarily the "working set size" depending on multithreading configuration.
			 * @returns Length of the memory region in bytes.
			 */
			size_t getLen();

			/**
			 * @brief Gets the number of iterations for this benchmark.
			 * @returns The number of iterations for this benchmark.
			 */
			uint32_t getIterations();

#ifdef USE_SIZE_BASED_BENCHMARKS
			/**
			 * @brief Gets the number of passes in each iteration.
			 * @returns The number of passes per iteration for this benchmark.
			 */
			uint64_t getPassesPerIteration();
#endif

			/**
			 * @brief Gets the width of memory access used in this benchmark.
			 * @returns The chunk size for this benchmark.
			 */
			xmem::common::chunk_size_t getChunkSize();

			/**
			 * @brief Gets the CPU NUMA node used in this benchmark.
			 * @returns The NUMA CPU node used in this benchmark.
			 */
			uint32_t getCPUNode();

			/**
			 * @brief Gets the memory NUMA node used in this benchmark.
			 * @returns The NUMA memory node used in this benchmark.
			 */
			uint32_t getMemNode();

			/**
			 * @brief Gets the number of worker threads used in this benchmark.
			 * @returns The number of worker threads used in this benchmark.
			 */
			uint32_t getNumThreads();

			/**
			 * @brief Gets the human-friendly name of this benchmark.
			 * @returns The benchmark test name.
			 */
			std::string getName();

		protected:
			/**
			 * @brief Starts the DRAM power measurement threads.
			 * @returns true on success
			 */
			bool _start_power_threads();

			/**
			 * @brief Stops the DRAM power measurement threads.
			 * This is a blocking call.
			 * @returns true on success
			 */
			bool _stop_power_threads();


			void* _mem_array; /**< Pointer to the memory region to use in this benchmark. */
			size_t _len; /**< Length of the memory region in bytes. This is not the working set size per thread! */

			//Benchmarking config
			uint32_t _iterations; /**< Number of iterations used in this benchmark. */
#ifdef USE_SIZE_BASED_BENCHMARKS
			uint64_t _passes_per_iteration; /**< Number of passes per iteration in this benchmark. */
#endif
			xmem::common::chunk_size_t _chunk_size; /**< Chunk size of memory accesses in this benchmark. TODO: Move this to ThroughputBenchmark.h, as it does not apply in all situations, e.g. in LatencyBenchmark. */
			size_t* _indices; /**< Pointer to a list of indices. This is for indirect memory addressing. Currently unused. TODO: Remove this entirely? */
			uint32_t _cpu_node; /**< The CPU NUMA node used in this benchmark. */
			uint32_t _mem_node; /**< The memory NUMA node used in this benchmark. */
			uint32_t _num_worker_threads; /**< The number of worker threads used in this benchmark. */

			//Benchmarking aids
			xmem::timers::Timer *_timer; /**< The reference timer for this benchmark. TODO: Remove this. It isn't thread safe anyway, so workers don't use it. */
			
			//Power measurement
			std::vector<xmem::power::PowerReader*> _dram_power_readers; /**< The power reading objects for measuring DRAM power on a per-socket basis during the benchmark. */
			std::vector<xmem::thread::Thread*> _dram_power_threads; /**< The power reading threads for measuring DRAM power on a per-socket basis during the benchmark. These work with the DRAM power readers. Although they are worker threads, they are not counted as the "official" benchmarking worker threads. */
			std::vector<double> _average_dram_power_socket; /**< The average DRAM power in this benchmark, per socket. */
			std::vector<double> _peak_dram_power_socket; /**< The peak DRAM power in this benchmark, per socket. */

			//Benchmark results
			bool _hasRun; /**< Indicates whether the benchmark has run. */
			std::vector<double> _metricOnIter; /**< Metrics for each iteration of the benchmark. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
			double _averageMetric; /**< Average metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */

			//Metadata
			std::string _name; /**< Name of this benchmark. */
			bool _obj_valid; /**< Indicates whether this benchmark object is valid. */
			bool _warning; /**< Indicates whether the benchmarks results might be clearly questionable/inaccurate/incorrect due to a variety of factors. */
		};
	};
};

#endif
