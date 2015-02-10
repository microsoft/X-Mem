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
 * @brief Header file for the LatencyBenchmark class.
 */

#ifndef __LATENCY_BENCHMARK_H
#define __LATENCY_BENCHMARK_H

//Headers
#include <Benchmark.h>
#include <common.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {

	/**
	 * @brief A type of benchmark that measures unloaded memory latency via random pointer chasing.
	 */
	class LatencyBenchmark : public Benchmark {
	public:
		typedef int32_t(*LatencyBenchFunction)(uintptr_t*, uintptr_t**, size_t); //Core benchmark function pointer

		/**
		 * @brief Constructor. Parameters are passed directly to the Benchmark constructor. See Benchmark class documentation for parameter semantics.
		 */
		LatencyBenchmark(
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
			std::string name,
			Timer *timer,
			std::vector<PowerReader*> dram_power_readers,
			pattern_mode_t pattern_mode,
			rw_mode_t rw_mode
		);
		
		/**
		 * @brief Destructor.
		 */
		virtual ~LatencyBenchmark();

		/**
		 * @brief Runs the benchmark.
		 * @returns True on success.
		 */
		virtual bool run();

		/**
		 * @brief Outputs the benchmark configuration to the console.
		 */
		virtual void report_benchmark_info();

		/**
		 * @brief Outputs a report of the benchmark results to the console if run() returned true.
		 */
		virtual void report_results();

	private:
		/**
		 * @brief Primes the whole memory space to be used in the benchmark.
		 * This is to warm up the caches and make sure the OS has physical pages allocated as well.
		 * @param passes The number of priming passes to do.
		 */
		void __primeMemory(uint64_t passes);

		/**
		 * @brief The core benchmark method.
		 * @returns True on success.
		 */
		bool __run_core();

		/**
		 * @brief Constructs a cycle of pointers in the whole memory space, such that each word will be reached exactly once. This is essentially a random linked list.
		 * For successive iterations, the access pattern is identical.
		 * @returns True on success.
		 */
		bool __buildRandomPointerPermutation();

		/**
		 * @brief Outputs a report of the benchmark results to the console if they ran.
		 */
		void __internal_report_results();

		LatencyBenchFunction __bench_fptr; /**< Points to the memory test core routine to use. */
		LatencyBenchFunction __dummy_fptr; /**< Points to a dummy version of the memory test core routine to use. */
	};
};

#endif
