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
 * @brief Header file for the LatencyWorker class.
 */

#ifndef __LATENCY_WORKER_H
#define __LATENCY_WORKER_H

//Headers
#include <MemoryWorker.h>
#include <benchmark_kernels.h>
#include <common.h>

namespace xmem {
	/** 
	 * @brief Multithreading-friendly class to do memory loading.
	 */
	class LatencyWorker : public MemoryWorker {
		public:
			
			/** 
			 * @brief Constructor for sequential-access patterns.
			 * @param mem_array Pointer to the memory region to use by this worker.
			 * @param len Length of the memory region to use by this worker.
			 * @param kernel_fptr Pointer to the sequential core benchmark kernel to use.
			 * @param kernel_dummy_fptr Pointer to the sequential dummy version of the core benchmark kernel to use.
			 * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
			 */
			LatencyWorker(
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
			virtual ~LatencyWorker();

			/**
			 * @brief Thread-safe worker method.
			 */
			virtual void run();

		private:
			// ONLY ACCESS OBJECT VARIABLES UNDER THE RUNNABLE OBJECT LOCK!!!!
			RandomFunction __kernel_fptr; /**< Points to the memory test core routine to use. */
			RandomFunction __kernel_dummy_fptr; /**< Points to a dummy version of the memory test core routine to use. */
	};
};

#endif
