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
 * @brief Header file for the BenchmarkManager class.
 */

#ifndef __BENCHMARK_MANAGER_H
#define __BENCHMARK_MANAGER_H

//Headers
#include <common/common.h>
#include <timers/Timer.h>
#include <power/NativeDRAMPowerReader.h>
#include "Benchmark.h"
#include "ThroughputBenchmark.h"
#include "LatencyBenchmark.h"

//Libraries
#include <cstdint>
#include <vector>
#include <fstream>

namespace xmem {
	namespace benchmark {
		/**
		 * @brief Manages running all benchmarks at a high level.
		 */
		class BenchmarkManager {
		public:
			/**
			 * @brief Constructor.
			 * @param working_set_size Total memory to test in bytes on each NUMA node. The BenchmarkManager will try to allocate them by itself.
			 * @param iterations_per_benchmark Number of passes to run for each individual benchmark.
			 * @param output_to_file If true, write to file specified by results_filename.
			 * @param results_filename Filename to write results to if output_to_file is true.
			 */
			BenchmarkManager(size_t working_set_size, uint32_t iterations_per_benchmark, bool output_to_file, std::string results_filename);

			/**
			 * @brief Destructor.
			 */
			~BenchmarkManager();

			/**
			 * @brief Runs all possible benchmark configurations.
			 * @returns True on success.
			 */
			bool runAll();

			/**
			 * @brief Runs the throughput benchmarks.
			 * @returns True on benchmarking success.
			 */
			bool runThroughputBenchmarks();

			/**
			 * @brief Runs the latency benchmark.
			 * @returns True on benchmarking success.
			 */
			bool runLatencyBenchmarks();

		private:
			/**
			 * @brief Allocates memory for all working sets.
			 * @param working_set_size Memory size in bytes, per enabled NUMA node.
			 */
			void __setupWorkingSets(size_t working_set_size);

			/**
			 * @brief Constructs and initializes all configured throughput benchmarks.
			 */
			void __buildThroughputBenchmarks();

			/**
			 * @brief Constructs and initializes all configured latency benchmarks.
			 */
			void __buildLatencyBenchmarks();

			uint32_t __num_numa_nodes; /**< Number of NUMA nodes in the system. */
			uint32_t __benchmark_num_numa_nodes; /**< Number of NUMA nodes to use in benchmarks. */
			uint32_t __num_worker_threads; /**< Number of worker threads to use in each benchmark. */
			std::vector<void*> __mem_arrays; /**< Memory regions to use in benchmarks. One for each benchmarked NUMA node. */
			std::vector<size_t> __mem_array_lens; /**< Length of each memory region to use in benchmarks. */
			std::vector<ThroughputBenchmark*> __tp_benchmarks; /**< Set of throughput benchmarks. */
			std::vector<LatencyBenchmark*> __lat_benchmarks; /**< Set of latency benchmarks. */
			std::vector<power::PowerReader*> __dram_power_readers; /**< Set of power measurement objects for DRAM on each NUMA node. */

#if defined(_WIN32) && defined(USE_QPC_TIMER)
			xmem::timers::win::QPCTimer __timer; /**< Timer object. TODO: Make this cleaner. We can't use these objects in thread safe manner so it isn't used in any multithreaded scenario at the moment. */
#endif
#if defined(ARCH_INTEL_X86_64) && defined(USE_TSC_TIMER)
			xmem::timers::x86_64::TSCTimer __timer; /**< Timer object. TODO: Make this cleaner. We can't use these objects in thread safe manner so it isn't used in any multithreaded scenario at the moment. */
#endif
			bool __output_to_file; /**< If true, output benchmark results to a CSV file. */
			std::string __results_filename; /**< Benchmark results CSV filename. */
			std::fstream __results_file; /**< The results CSV file. */
			bool __built_throughput_benchmarks; /**< If true, finished building throughput benchmarks. */
			bool __built_latency_benchmarks; /**< If true, finished building latency benchmarks. */
			uint32_t __iterations_per_benchmark; /**< Iterations per benchmark. */
		};
	};
};

#endif
