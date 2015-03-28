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
 * @brief Header file for the LatencyBenchmark_Delays class.
 */

#ifndef __LATENCY_BENCHMARK_DELAYS_H
#define __LATENCY_BENCHMARK_DELAYS_H

//Headers
#include <LatencyBenchmark.h>
#include <common.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {

	/**
	 * @brief A type of benchmark that measures loaded memory latency via random pointer chasing while load threads provide memory interference with delay injection to generate different degrees of loading.
	 */
	class LatencyBenchmark_Delays : public LatencyBenchmark {
	public:
		
		/**
		 * @brief Constructor. Parameters are passed directly to the LatencyBenchmark constructor. See LatencyBenchmark class documentation for parameter semantics. The access pattern is hard-coded to SEQUENTIAL, read/write pattern to READ, chunk size to CHUNK_64b, and stride to 1.
		 */
		LatencyBenchmark_Delays(
			void* mem_array,
			size_t len,
			uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
			uint64_t passes_per_iteration,
#endif
			uint32_t num_worker_threads,
			uint32_t mem_node,
			uint32_t cpu_node,
			std::vector<PowerReader*> dram_power_readers,
			std::string name
		);
		
		/**
		 * @brief Destructor.
		 */
		virtual ~LatencyBenchmark_Delays() {}

	protected:
		virtual bool _run_core();
	};
};

#endif
