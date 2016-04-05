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
 * @brief Header file for the BenchmarkManager class.
 */

#ifndef BENCHMARK_MANAGER_H
#define BENCHMARK_MANAGER_H

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
         * @brief Runs all benchmark configurations (does not include extensions).
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

#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK
        /**
         * @brief Runs the delay-injected loaded latency benchmark extension.
         * @returns True on success.
         */
        bool runExtDelayInjectedLoadedLatencyBenchmark();
#endif

#ifdef EXT_STREAM_BENCHMARK
        /**
         * @brief Runs the STREAM-like benchmark extension.
         * @returns True on success.
         */
        bool runExtStreamBenchmark();
#endif

    private:
        /**
         * @brief Allocates memory for all working sets.
         * @param working_set_size Memory size in bytes, per enabled NUMA node.
         */
        void setupWorkingSets(size_t working_set_size);

        /**
         * @brief Constructs and initializes all configured benchmarks.
         * @returns True on success.
         */
        bool buildBenchmarks();

        Configurator config_;

        std::list<uint32_t> cpu_numa_node_affinities_; /**< List of CPU nodes to affinitize for benchmark experiments. */
        std::list<uint32_t> memory_numa_node_affinities_; /**< List of memory nodes to affinitize for benchmark experiments. */
        std::vector<void*> mem_arrays_; /**< Memory regions to use in benchmarks. One for each benchmarked NUMA node. */
#ifndef HAS_NUMA
        void* orig_malloc_addr_; /**< Points to the original address returned by the malloc() for __mem_arrays on non-NUMA machines. Special case. FIXME: do we need this? seems awkward */
#endif
        std::vector<size_t> mem_array_lens_; /**< Length of each memory region to use in benchmarks. */
        std::vector<ThroughputBenchmark*> tp_benchmarks_; /**< Set of throughput benchmarks. */
        std::vector<LatencyBenchmark*> lat_benchmarks_; /**< Set of latency benchmarks. */
        std::vector<PowerReader*> dram_power_readers_; /**< Set of power measurement objects for DRAM on each NUMA node. */
        std::fstream results_file_; /**< The results CSV file. */
        bool built_benchmarks_; /**< If true, finished building all benchmarks. */
    };
};

#endif
