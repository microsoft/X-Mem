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
 * @brief Header file for the LoadWorker class.
 */

#ifndef LOAD_WORKER_H
#define LOAD_WORKER_H

//Headers
#include <MemoryWorker.h>
#include <benchmark_kernels.h>

namespace xmem {
    /** 
     * @brief Multithreading-friendly class to do memory loading.
     */
    class LoadWorker : public MemoryWorker {
        public:
            
            /** 
             * @brief Constructor for sequential-access patterns.
             * @param mem_array Pointer to the memory region to use by this worker.
             * @param len Length of the memory region to use by this worker.
             * @param kernel_fptr Pointer to the sequential core benchmark kernel to use.
             * @param kernel_dummy_fptr Pointer to the sequential dummy version of the core benchmark kernel to use.
             * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
             */
            LoadWorker(
                void* mem_array,
                size_t len,
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
            LoadWorker(
                void* mem_array,
                size_t len,
                RandomFunction kernel_fptr,
                RandomFunction kernel_dummy_fptr,
                int32_t cpu_affinity
            );
            
            /**
             * @brief Destructor.
             */
            virtual ~LoadWorker();

            /**
             * @brief Thread-safe worker method.
             */
            virtual void run();
        
        private:
            // ONLY ACCESS OBJECT VARIABLES UNDER THE RUNNABLE OBJECT LOCK!!!!
            bool use_sequential_kernel_fptr_; /**< If true, use the SequentialFunction, otherwise use the RandomFunction. */
            SequentialFunction kernel_fptr_seq_; /**< Points to the memory test core routine to use of the "sequential" type. */
            SequentialFunction kernel_dummy_fptr_seq_; /**< Points to a dummy version of the memory test core routine to use of the "sequential" type. */
            RandomFunction kernel_fptr_ran_; /**< Points to the memory test core routine to use of the "random" type. */
            RandomFunction kernel_dummy_fptr_ran_; /**< Points to a dummy version of the memory test core routine to use of the "random" type. */
    };
};

#endif
