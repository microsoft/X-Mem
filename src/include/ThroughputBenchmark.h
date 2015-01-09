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
 * @brief Header file for the ThroughputBenchmark class.
 */

#ifndef __THROUGHPUT_BENCHMARK_H
#define __THROUGHPUT_BENCHMARK_H

//Headers
#include <include/Benchmark.h>
#include <include/common.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {
	namespace benchmark {
		/**
		 * @brief A type of benchmark that measures memory throughput either via sequential, strided sequential, or random access patterns.
		 */
		class ThroughputBenchmark : public Benchmark {
		public:
			typedef int32_t(*ThroughputBenchFunction)(void*, void*); //Core benchmark function pointer

			/**
			 * @brief Constructor.
			 * @param mem_array a pointer to a contiguous chunk of memory that has been allocated for benchmarking among the worker threads. This should be aligned to a 256-bit boundary and should be the working set size times number of threads large.
			 * @param len Length of the raw_mem_array in bytes. This should be a multiple of 4 KB pages.
			 * @param iterations Number of iterations (passes) to do of the complete benchmark.
			 * @param chunk_size encoded size of an individual memory access.
			 * @param cpu_node the logical CPU NUMA node to use in the benchmark
			 * @param mem_node the logical memory NUMA node used in the benchmark
			 * @param num_worker_threads number of worker threads to use in the benchmark
			 * @param name name of the benchmark to use when reporting
			 * @param timer pointer to an existing Timer object
			 * @param dram_power_readers vector of pointers to PowerReader objects for measuring DRAM power
			 * @param stride_size For sequential access patterns, the stride counted in chunks. Negative values mean reverse access pattern. A stride of 1 is purely sequential.
			 * @param pattern_mode Indicates sequential or random access.
			 * @param rw_mode Indicates reads or writes. TODO: allow for a mixture
			 */
			ThroughputBenchmark(
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
				std::vector<xmem::power::PowerReader*> dram_power_readers,
				int64_t stride_size,
				xmem::common::pattern_mode_t pattern_mode,
				xmem::common::rw_mode_t rw_mode
			);

			/**
			 * @brief Runs the benchmark.
			 * @returns true on success
			 */
			virtual bool run();

			/**
			 * @brief Reports benchmark configuration details to the console.
			 */
			virtual void report_benchmark_info();

			/**
			 * @brief Reports results to the console.
			 */
			virtual void report_results();

			/**
			 * @brief Gets the stride size for this benchmark.
			 * @returns The stride size in chunks.
			 */
			int64_t getStrideSize();

			/**
			 * @brief Gets the pattern mode for this benchmark.
			 * @returns The pattern mode enumerator.
			 */
			xmem::common::pattern_mode_t getPatternMode();

			/**
			 * @brief Gets the read/write mode for this benchmark.
			 * @returns The read/write mix mode.
			 */
			xmem::common::rw_mode_t getRWMode();

		private:
			/**
			 * @brief The core benchmark method.
			 * @returns True on success.
			 */
			bool __run_core();

			/**
			 * @brief Constructs a random access pattern across the whole working set.
			 * For successive iterations, the access pattern is identical.
			 * @returns True on success.
			 */
			bool __buildRandomPointerPermutation();

			/**
			 * @brief Outputs a report of the benchmark results to the console if they ran.
			 */
			void __internal_report_results();

			int64_t __stride_size; /**< Stride size in chunks for sequential pattern mode only. */
			xmem::common::pattern_mode_t __pattern_mode; /**< Access pattern mode. */
			xmem::common::rw_mode_t __rw_mode; /**< Read/write mode. */
			ThroughputBenchFunction __bench_fptr; /**< Points to the memory test kernel to use. */
			ThroughputBenchFunction __dummy_fptr; /**< Points to a dummy version of the memory test kernel to use. */
		};
	};
};

#endif
