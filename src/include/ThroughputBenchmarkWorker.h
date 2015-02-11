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
 * @brief Header file for the ThroughputBenchmarkWorker class.
 */

#ifndef __THROUGHPUT_BENCHMARK_WORKER_H
#define __THROUGHPUT_BENCHMARK_WORKER_H

//Headers
#include <Runnable.h>
#include <common.h>
#include <benchmark_kernels.h>

//Libraries
#include <cstdint>

namespace xmem {
	/** 
	 * @brief Helper multithreading-friendly class to do the core throughput benchmark.
	 */
	class ThroughputBenchmarkWorker : public Runnable {
		public:
			
			/** 
			 * @brief Constructor for sequential-access patterns.
			 * @param mem_array Pointer to the memory region to use by this worker.
			 * @param len Length of the memory region to use by this worker.
			 * @param kernel_fptr Pointer to the sequential core benchmark kernel to use.
			 * @param kernel_dummy_fptr Pointer to the sequential dummy version of the core benchmark kernel to use.
			 * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
			 */
			ThroughputBenchmarkWorker(
				void* mem_array,
				size_t len,
#ifdef USE_SIZE_BASED_BENCHMARKS
				uint64_t passes_per_iteration,
#endif
				SequentialFunction kernel_fptr,
				SequentialFunction kernel_dummy_fptr,
				int32_t cpu_affinity
			);
			
			/** 
			 * @brief Constructor for random-access patterns.
			 * @param mem_array Pointer to the memory region to use by this worker.
			 * @param len Length of the memory region to use by this worker.
			 * @param kernel_fptr Pointer to the random core benchmark kernel to use.
			 * @param kernel_dummy_fptr Pointer to the random dummy version of the core benchmark kernel to use.
			 * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
			 */
			ThroughputBenchmarkWorker(
				void* mem_array,
				size_t len,
#ifdef USE_SIZE_BASED_BENCHMARKS
				uint64_t passes_per_iteration,
#endif
				RandomFunction kernel_fptr,
				RandomFunction kernel_dummy_fptr,
				int32_t cpu_affinity
			);
			
			/**
			 * @brief Destructor.
			 */
			virtual ~ThroughputBenchmarkWorker();

			/**
			 * @brief Thread-safe worker method.
			 */
			virtual void run();
		
			/**
			 * @brief Gets the length of the memory region used by this worker.
			 * @returns Length of memory region in bytes.
			 */
			size_t getLen();

			/**
			 * @brief Gets the number of bytes used in each pass of the benchmark by this worker.
			 * @returns Number of bytes in each pass.
			 */
			uint64_t getBytesPerPass();

			/**
			 * @brief Gets the number of passes for this worker.
			 * @returns The number of passes.
			 */
			uint64_t getPasses();

			/**
			 * @brief Gets the elapsed ticks for this worker on the core benchmark kernel.
			 * @returns The number of elapsed ticks.
			 */
			uint64_t getElapsedTicks();

			/**
			 * @brief Gets the elapsed ticks for this worker on the dummy version of the core benchmark kernel.
			 * @returns The number of elapsed dummy ticks.
			 */
			uint64_t getElapsedDummyTicks();

			/**
			 * @brief Gets the adjusted ticks for this worker. This is elapsed ticks minus elapsed dummy ticks.
			 * @returns The adjusted ticks for this worker.
			 */
			uint64_t getAdjustedTicks();

			/**
			 * @brief Indicates whether worker's results may be questionable/inaccurate/invalid.
			 * @returns True if the worker's results had a warning.
			 */
			bool hadWarning();

		private:
			// ONLY ACCESS OBJECT VARIABLES UNDER THE RUNNABLE OBJECT LOCK!!!!
			void* __mem_array; /**< The memory region for this worker. */
			size_t __len; /**< The length of the memory region for this worker. */
			int32_t __cpu_affinity; /**< The logical CPU affinity for this worker. */
			bool __use_sequential_kernel_fptr; /**< If true, use the SequentialFunction, otherwise use the RandomFunction. */
			SequentialFunction __kernel_fptr_seq; /**< Points to the memory test core routine to use of the "sequential" type. */
			SequentialFunction __kernel_dummy_fptr_seq; /**< Points to a dummy version of the memory test core routine to use of the "sequential" type. */
			RandomFunction __kernel_fptr_ran; /**< Points to the memory test core routine to use of the "random" type. */
			RandomFunction __kernel_dummy_fptr_ran; /**< Points to a dummy version of the memory test core routine to use of the "random" type. */
			uint64_t __bytes_per_pass; /**< Number of bytes accessed in each kernel pass. */
			uint64_t __passes; /**< Number of passes. */
			uint64_t __elapsed_ticks; /**< Total elapsed ticks on the kernel routine. */
			uint64_t __elapsed_dummy_ticks; /**< Total elapsed ticks on the dummy kernel routine. */
			uint64_t __adjusted_ticks; /**< Elapsed ticks minus dummy elapsed ticks. */
			bool __warning; /**< If true, results may be suspect. */
			bool __completed; /**< If true, worker completed. */
#ifdef USE_SIZE_BASED_BENCHMARKS
			uint64_t __passes_per_iteration; /**< Number of passes per iteration. */
#endif
	};
};

#endif
