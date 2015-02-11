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
#include <common.h>
#include <Timer.h>
#include <PowerReader.h>
#include <Benchmark.h>
#include <ThroughputBenchmark.h>
#include <LatencyBenchmark.h>
#include <Configurator.h>

//Libraries
#include <cstdint>
#include <vector>
#include <fstream>

namespace xmem {
	/**
	 * @brief Manages running all benchmarks at a high level.
	 */
	class BenchmarkManager {
	public:
		/**
		 * @brief Constructor.
		 * @param config The configuration object containing run-time options for this X-Mem execution instance.
		 */
		BenchmarkManager(Configurator &config);

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

		Configurator __config;

		uint32_t __num_numa_nodes; /**< Number of NUMA nodes in the system. */
		uint32_t __benchmark_num_numa_nodes; /**< Number of NUMA nodes to use in benchmarks. */
		std::vector<void*> __mem_arrays; /**< Memory regions to use in benchmarks. One for each benchmarked NUMA node. */
		std::vector<size_t> __mem_array_lens; /**< Length of each memory region to use in benchmarks. */
		std::vector<ThroughputBenchmark*> __tp_benchmarks; /**< Set of throughput benchmarks. */
		std::vector<LatencyBenchmark*> __lat_benchmarks; /**< Set of latency benchmarks. */
		std::vector<PowerReader*> __dram_power_readers; /**< Set of power measurement objects for DRAM on each NUMA node. */
		std::fstream __results_file; /**< The results CSV file. */
		bool __built_throughput_benchmarks; /**< If true, finished building throughput benchmarks. */
		bool __built_latency_benchmarks; /**< If true, finished building latency benchmarks. */
	};
};

#endif
