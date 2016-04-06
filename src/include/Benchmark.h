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
 * @brief Header file for the Benchmark class.
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

//Headers
#include <common.h>

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
         * @param num_worker_threads The number of worker threads to use in the benchmark.
         * @param mem_node The logical memory NUMA node used in the benchmark.
         * @param cpu_node The logical CPU NUMA node to use for the benchmark.
         * @param pattern_mode This indicates the general type of access pattern used, e.g. sequential or random.
         * @param rw_mode This indicates the general type of read/write mix used, e.g. pure reads or pure writes.
         * @param chunk_size Size of an individual memory access for load-generating worker threads.
         * @param stride_size For sequential access patterns, this is the address distance between successive accesses, counted in chunks. Negative values indicate a reversed access pattern. A stride of +/-1 is purely sequential.
         * @param dram_power_readers A group of PowerReader objects for measuring DRAM power.
         * @param name The name of the benchmark to use when reporting to console.
         */
        Benchmark(
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
         * @brief Prints a header piece of information describing the benchmark to the console.
         */
        void printBenchmarkHeader() const;

        /**
         * @brief Reports benchmark configuration details to the console.
         */
        virtual void reportBenchmarkInfo() const;

        /**
         * @brief Reports results to the console.
         */
        virtual void reportResults() const;

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
         * @brief Gets the arithmetic mean benchmark metric across all iterations.
         * @returns The mean metric.
         */
        double getMeanMetric() const;

        /**
         * @brief Gets the minimum benchmark metric across all iterations.
         * @returns The minimum metric.
         */
        double getMinMetric() const;

        /**
         * @brief Gets the 25th percentile benchmark metric across all iterations.
         * @returns The 25th percentile metric.
         */
        double get25PercentileMetric() const;
        
        /**
         * @brief Gets the median benchmark metric across all iterations.
         * @returns The median metric.
         */
        double getMedianMetric() const;
       
        /**
         * @brief Gets the 75th percentile benchmark metric across all iterations.
         * @returns The 75th percentile metric.
         */
        double get75PercentileMetric() const;
        
        /**
         * @brief Gets the 95th percentile benchmark metric across all iterations.
         * @returns The 95th percentile metric.
         */
        double get95PercentileMetric() const;
        
        /**
         * @brief Gets the 99th percentile benchmark metric across all iterations.
         * @returns The 99th percentile metric.
         */
        double get99PercentileMetric() const;

        /**
         * @brief Gets the maximum benchmark metric across all iterations.
         * @returns The maximum metric.
         */
        double getMaxMetric() const;

        /**
         * @brief Gets the mode benchmark metric across all iterations.
         * @returns The mode metric.
         */
        double getModeMetric() const;

        /**
         * @brief Gets the units of the metric for this benchmark.
         * @returns A string representing the units for printing to console and file.
         */
        std::string getMetricUnits() const;

        /**
         * @brief Gets the arithmetic mean DRAM power over the benchmark.
         * @returns The mean DRAM power for a given socket in watts, or 0 if the data does not exist (power was unable to be collected or the benchmark has not run).
         */
        double getMeanDRAMPower(uint32_t socket_id) const;
        
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

        /**
         * @brief Gets the width of memory access used in this benchmark.
         * @returns The chunk size for this benchmark.
         */
        chunk_size_t getChunkSize() const;

        /**
         * @brief Gets the stride size for this benchmark.
         * @returns The stride size in chunks.
         */
        int32_t getStrideSize() const;

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
        virtual bool runCore() = 0;

        /**
         * @brief Computes the metrics across iterations.
         */
        void computeMetrics();

        /**
         * @brief Starts the DRAM power measurement threads.
         * @returns True on success.
         */
        bool startPowerThreads();

        /**
         * @brief Stops the DRAM power measurement threads.
         * This is a blocking call.
         * @returns True on success.
         */
        bool stopPowerThreads();


        //Memory region under test
        void* mem_array_; /**< Pointer to the memory region to use in this benchmark. */
        size_t len_; /**< Length of the memory region in bytes. This is not the working set size per thread! */

        //Benchmark repetition
        uint32_t iterations_; /**< Number of iterations used in this benchmark. */
    
        //Threading and affinity
        uint32_t num_worker_threads_; /**< The number of worker threads used in this benchmark. */
        uint32_t mem_node_; /**< The memory NUMA node used in this benchmark. */
        uint32_t cpu_node_; /**< The CPU NUMA node used in this benchmark. */
        
        //Benchmarking settings
        pattern_mode_t pattern_mode_; /**< Access pattern mode. */
        rw_mode_t rw_mode_; /**< Read/write mode. */
        chunk_size_t chunk_size_; /**< Chunk size of memory accesses in this benchmark. */
        int32_t stride_size_; /**< Stride size in chunks for sequential pattern mode only. */
        
        //Power measurement
        std::vector<PowerReader*> dram_power_readers_; /**< The power reading objects for measuring DRAM power on a per-socket basis during the benchmark. */
        std::vector<Thread*> dram_power_threads_; /**< The power reading threads for measuring DRAM power on a per-socket basis during the benchmark. These work with the DRAM power readers. Although they are worker threads, they are not counted as the "official" benchmarking worker threads. */
        
        //Benchmark results
        std::vector<double> metric_on_iter_; /**< Metrics for each iteration of the benchmark. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double mean_metric_; /**< Average metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double min_metric_; /**< Minimum metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double 25_percentile_metric_; /**< 25th percentile metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double median_metric_; /**< Median metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double 75_percentile_metric_; /**< 75th percentile metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double 95_percentile_metric_; /**< 95th percentile metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double 99_percentile_metric_; /**< 99th percentile metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double max_metric_; /**< Maximum metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        double mode_metric_; /**< Mode metric over all iterations. Unit-less because any benchmark can set this metric as needed. It is up to the descendant class to interpret units. */
        std::string metric_units_; /**< String representing the units of measurement for the metric. */
        std::vector<double> mean_dram_power_socket_; /**< The mean DRAM power in this benchmark, per socket. */
        std::vector<double> peak_dram_power_socket_; /**< The peak DRAM power in this benchmark, per socket. */

        //Metadata
        std::string name_; /**< Name of this benchmark. */
        bool obj_valid_; /**< Indicates whether this benchmark object is valid. */
        bool has_run_; /**< Indicates whether the benchmark has run. */
        bool warning_; /**< Indicates whether the benchmarks results might be clearly questionable/inaccurate/incorrect due to a variety of factors. */
    };
};

#endif
