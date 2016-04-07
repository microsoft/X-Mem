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
 * @brief Header file for the MemoryWorker class.
 */

#ifndef MEMORY_WORKER_H
#define MEMORY_WORKER_H

//Headers
#include <common.h>
#include <Runnable.h>

//Libraries
#include <cstdint>

namespace xmem {
    /** 
     * @brief Multithreading-friendly class to run memory access kernels.
     */
    class MemoryWorker : public Runnable {
        public:
            
            /** 
             * @brief Constructor.
             * @param mem_array Pointer to the memory region to use by this worker.
             * @param len Length of the memory region to use by this worker.
             * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
             */
            MemoryWorker(
                void* mem_array,
                size_t len,
                int32_t cpu_affinity
            );
            
            /**
             * @brief Destructor.
             */
            virtual ~MemoryWorker();

            /**
             * @brief Thread-safe worker method.
             */
            virtual void run() = 0;
        
            /**
             * @brief Gets the length of the memory region used by this worker.
             * @returns Length of memory region in bytes.
             */
            size_t getLen();

            /**
             * @brief Gets the number of bytes used in each pass of the benchmark by this worker.
             * @returns Number of bytes in each pass.
             */
            uint32_t getBytesPerPass();

            /**
             * @brief Gets the number of passes for this worker.
             * @returns The number of passes.
             */
            uint32_t getPasses();

            /**
             * @brief Gets the elapsed ticks for this worker on the core benchmark kernel.
             * @returns The number of elapsed ticks.
             */
            tick_t getElapsedTicks();

            /**
             * @brief Gets the elapsed ticks for this worker on the dummy version of the core benchmark kernel.
             * @returns The number of elapsed dummy ticks.
             */
            tick_t getElapsedDummyTicks();

            /**
             * @brief Gets the adjusted ticks for this worker. This is elapsed ticks minus elapsed dummy ticks.
             * @returns The adjusted ticks for this worker.
             */
            tick_t getAdjustedTicks();

            /**
             * @brief Indicates whether worker's results may be questionable/inaccurate/invalid.
             * @returns True if the worker's results had a warning.
             */
            bool hadWarning();

        protected:
            // ONLY ACCESS OBJECT VARIABLES UNDER THE RUNNABLE OBJECT LOCK!!!!
            void* mem_array_; /**< The memory region for this worker. */
            size_t len_; /**< The length of the memory region for this worker. */
            int32_t cpu_affinity_; /**< The logical CPU affinity for this worker. */
            uint32_t bytes_per_pass_; /**< Number of bytes accessed in each kernel pass. */
            uint32_t passes_; /**< Number of passes. */
            tick_t elapsed_ticks_; /**< Total elapsed ticks on the kernel routine. */
            tick_t elapsed_dummy_ticks_; /**< Total elapsed ticks on the dummy kernel routine. */
            tick_t adjusted_ticks_; /**< Elapsed ticks minus dummy elapsed ticks. */
            bool warning_; /**< If true, results may be suspect. */
            bool completed_; /**< If true, worker completed. */
    };
};

#endif
