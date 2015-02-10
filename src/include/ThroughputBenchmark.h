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
#include <Benchmark.h>
#include <common.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {

	/**
	 * @brief A type of benchmark that measures memory throughput.
	 */
	class ThroughputBenchmark : public Benchmark {
	public:
		typedef int32_t(*ThroughputBenchFunction)(void*, void*); //Core benchmark function pointer

		/**
		 * @brief Constructor. Parameters are passed directly to the Benchmark constructor. See Benchmark class documentation for parameter semantics.
		 */
		ThroughputBenchmark(
			void* mem_array,
			size_t len,
			uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
			uint64_t passes_per_iteration,
#endif
			chunk_size_t chunk_size,
			int64_t stride_size,
			uint32_t cpu_node,
			uint32_t mem_node,
			uint32_t num_worker_threads,
			std::string name,
			Timer *timer,
			std::vector<PowerReader*> dram_power_readers,
			pattern_mode_t pattern_mode,
			rw_mode_t rw_mode
		);

		/**
		 * @brief Destructor.
		 */
		virtual ~ThroughputBenchmark();

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

		ThroughputBenchFunction __bench_fptr; /**< Points to the memory test kernel to use. */
		ThroughputBenchFunction __dummy_fptr; /**< Points to a dummy version of the memory test kernel to use. */
	};
};

#endif
