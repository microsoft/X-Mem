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
 * @brief Header file for the DelayInjectedLoadedLatencyBenchmark class.
 */

#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK

#ifndef __DELAY_INJECTED_LOADED_LATENCY_BENCHMARK_H
#define __DELAY_INJECTED_LOADED_LATENCY_BENCHMARK_H

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
	class DelayInjectedLoadedLatencyBenchmark : public LatencyBenchmark {
	public:
		
		/**
		 * @brief Constructor. Parameters are passed directly to the LatencyBenchmark constructor. See LatencyBenchmark class documentation for parameter semantics. The access pattern is hard-coded to SEQUENTIAL, read/write pattern to READ, and stride to 1. Chunk size may be either 64-bit or 256-bit.
		 */
		DelayInjectedLoadedLatencyBenchmark(
			void* mem_array,
			size_t len,
			uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
			uint32_t passes_per_iteration,
#endif
			uint32_t num_worker_threads,
			uint32_t mem_node,
			uint32_t cpu_node,
			chunk_size_t chunk_size,
			std::vector<PowerReader*> dram_power_readers,
			std::string name,
			uint32_t delay
		);
		
		/**
		 * @brief Destructor.
		 */
		virtual ~DelayInjectedLoadedLatencyBenchmark() {}
	
		/**
		 * @brief Reports benchmark configuration details to the console.
		 */
		virtual void report_benchmark_info() const;

		/**
		 * @brief Gets the delay injection used in load thread kernels. A delay of 5 corresponds to 5 nop instructions.
		 * @returns The delay value.
		 */
		uint32_t getDelay() const;

	protected:
		virtual bool _run_core();

	private:
		uint32_t __delay; /**< Number of nops to insert between load thread memory instructions. This is a form of delay injection to reduce memory loading. */
	};
};

#endif

#endif
