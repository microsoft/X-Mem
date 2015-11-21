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
 * @brief Header file for benchmark kernel functions for doing the actual work we care about. :)
 */

#ifndef __BENCHMARK_KERNELS_H
#define __BENCHMARK_KERNELS_H

//Headers
#include <common.h>

//Libraries
#include <cstdint>
#include <cstddef>

namespace xmem {

    
    typedef int32_t(*SequentialFunction)(void*, void*);
    typedef int32_t(*RandomFunction)(uintptr_t*, uintptr_t**, size_t); 
    
    /**
     * @brief Determines which sequential memory access kernel to use based on the read/write mode, chunk size, and stride size.
     * @param rw_mode Read/write mode.
     * @param chunk_size Access granularity.
     * @param stride_size Distance between successive accesses.
     * @param kernel_function Function pointer that will be set to the matching kernel function.
     * @param dummy_kernel_function Function pointer that will be set to the matching dummy kernel function.
     * @returns True on success.
     */
    bool determineSequentialKernel(rw_mode_t rw_mode, chunk_size_t chunk_size, int32_t stride_size, SequentialFunction* kernel_function, SequentialFunction* dummy_kernel_function);
    
    /**
     * @brief Determines which random memory access kernel to use based on the read/write mode, chunk size, and stride size.
     * @param rw_mode Read/write mode.
     * @param chunk_size Access granularity.
     * @param kernel_function Function pointer that will be set to the matching kernel function.
     * @param dummy_kernel_function Function pointer that will be set to the matching dummy kernel function.
     * @returns True on success.
     */
    bool determineRandomKernel(rw_mode_t rw_mode, chunk_size_t chunk_size, RandomFunction* kernel_function, RandomFunction* dummy_kernel_function);

    /**
     * @brief Builds a random chain of pointers within the specified memory region.
     * @param start_address Beginning address of the memory region.
     * @param end_address End address of the memory region.
     * @param chunk_size Granularity of words to read, dereference, and jump by. This must be at least the minimum pointer size on the system (typically 32 or 64-bit). If the chunk size is more than 64 bits, when chasing pointers, only the first pointer-sized bits of the referenced word are used to make the next hop.
     * @returns True on success.
     */
    bool buildRandomPointerPermutation(void* start_address, void* end_address, chunk_size_t chunk_size);

    /***********************************************************************
     ***********************************************************************
     ********************** LATENCY-RELATED BENCHMARK KERNELS **************
     ***********************************************************************
     ***********************************************************************/

    /* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

    /**
     * @brief Mimics the chasePointers() method but doesn't do the memory accesses.
     * @param len The number of fake pointer deferences. May be unused.
     * @returns Undefined.
     */
    int32_t dummy_chasePointers(uintptr_t*, uintptr_t**, size_t len);

    /* ------------------------------------------------------------------------- */
    /* --------------------- CORE BENCHMARK ROUTINES --------------------------- */
    /* ------------------------------------------------------------------------- */

    /**
     * @brief Walks over the allocated memory in random order by chasing pointers.
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t chasePointers(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
    
    
    
    /***********************************************************************
     ***********************************************************************
     ******************* THROUGHPUT-RELATED BENCHMARK KERNELS **************
     ***********************************************************************
     ***********************************************************************/

    /* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

    /**
     * @brief Does nothing. Used for measuring the time it takes just to call a benchmark routine via function pointer.
     * @returns Undefined.
     */
    int32_t dummy_empty(void*, void*);

    /* ------------ SEQUENTIAL LOOP --------------*/

    /**
     * @brief Used for measuring the time spent doing everything in forward sequential Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwSequentialLoop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in forward sequential Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwSequentialLoop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in forward sequential Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwSequentialLoop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in forward sequential Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwSequentialLoop_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Used for measuring the time spent doing everything in reverse sequential Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revSequentialLoop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in reverse sequential Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revSequentialLoop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in reverse sequential Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revSequentialLoop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in reverse sequential Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revSequentialLoop_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 2 LOOP --------------*/

    /**
     * @brief Used for measuring the time spent doing everything in forward 2-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride2Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in forward 2-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride2Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in forward 2-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride2Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in forward 2-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride2Loop_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Used for measuring the time spent doing everything in reverse 2-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride2Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in reverse 2-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride2Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in reverse 2-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride2Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in reverse 2-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride2Loop_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 4 LOOP --------------*/

    /**
     * @brief Used for measuring the time spent doing everything in forward 4-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride4Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in forward 4-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride4Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in forward 4-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride4Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in forward 4-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride4Loop_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Used for measuring the time spent doing everything in reverse 4-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride4Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in reverse 4-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride4Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in reverse 4-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride4Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in reverse 4-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride4Loop_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 8 LOOP --------------*/

    /**
     * @brief Used for measuring the time spent doing everything in forward 8-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride8Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in forward 8-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride8Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in forward 8-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride8Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in forward 8-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride8Loop_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Used for measuring the time spent doing everything in reverse 8-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride8Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in reverse 8-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride8Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in reverse 8-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride8Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in reverse 8-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride8Loop_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 16 LOOP --------------*/

    /**
     * @brief Used for measuring the time spent doing everything in forward 16-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride16Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in forward 16-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride16Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in forward 16-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride16Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in forward 16-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_forwStride16Loop_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Used for measuring the time spent doing everything in reverse 16-strided Word 32 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride16Loop_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Used for measuring the time spent doing everything in reverse 16-strided Word 64 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride16Loop_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Used for measuring the time spent doing everything in reverse 16-strided Word 128 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride16Loop_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Used for measuring the time spent doing everything in reverse 16-strided Word 256 loops except for the memory access itself.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t dummy_revStride16Loop_Word256(void* start_address, void* end_address);
#endif

    /* ------------ RANDOM LOOP --------------*/

    //SPECIAL CASE: on 32-bit architectures, 64-bit pointers cannot be used, and vice versa.
#ifndef HAS_WORD_64
    /**
     * @brief Mimics the randomRead_Word32 and randomWrite_Word32 functions except for the memory accesses.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t dummy_randomLoop_Word32(uintptr_t*, uintptr_t**, size_t len);
#endif

#ifdef HAS_WORD_64
    /**
     * @brief Mimics the randomRead_Word64 and randomWrite_Word64 functions except for the memory accesses.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t dummy_randomLoop_Word64(uintptr_t*, uintptr_t**, size_t len);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Mimics the randomRead_Word128 and randomWrite_Word128 functions except for the memory accesses.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t dummy_randomLoop_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Mimics the randomRead_Word256 and randomWrite_Word256 functions except for the memory accesses.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t dummy_randomLoop_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

    /* ------------------------------------------------------------------------- */
    /* --------------------- CORE BENCHMARK ROUTINES --------------------------- */
    /* ------------------------------------------------------------------------- */

    /* ------------ SEQUENTIAL READ --------------*/

    /**
     * @brief Walks over the allocated memory forward sequentially, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialRead_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory forward sequentially, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialRead_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory forward sequentially, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialRead_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory forward sequentially, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialRead_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory reverse sequentially, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialRead_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory reverse sequentially, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialRead_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory reverse sequentially, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialRead_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory reverse sequentially, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialRead_Word256(void* start_address, void* end_address);
#endif

    /* ------------ SEQUENTIAL WRITE --------------*/

    /**
     * @brief Walks over the allocated memory forward sequentially, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialWrite_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory forward sequentially, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialWrite_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory forward sequentially, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialWrite_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory forward sequentially, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwSequentialWrite_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory reverse sequentially, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialWrite_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory reverse sequentially, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialWrite_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory reverse sequentially, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialWrite_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory reverse sequentially, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revSequentialWrite_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 2 READ --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 2, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Read_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Read_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 2 WRITE --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 2, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 2, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride2Write_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 2, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride2Write_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 4 READ --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 4, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Read_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Read_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 4 WRITE --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 4, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 4, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride4Write_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 4, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride4Write_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 8 READ --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 8, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Read_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Read_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 8 WRITE --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 8, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     */
    int32_t forwStride8Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 8, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride8Write_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 8, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride8Write_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 16 READ --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 16, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Read_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, reading in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Read_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, reading in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Read_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, reading in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Read_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, reading in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Read_Word256(void* start_address, void* end_address);
#endif

    /* ------------ STRIDE 16 WRITE --------------*/

    /**
     * @brief Walks over the allocated memory in forward strides of size 16, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in forward strides of size 16, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t forwStride16Write_Word256(void* start_address, void* end_address);
#endif

    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, writing in 32-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Write_Word32(void* start_address, void* end_address);

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, writing in 64-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Write_Word64(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, writing in 128-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     */
    int32_t revStride16Write_Word128(void* start_address, void* end_address);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in reverse strides of size 16, writing in 256-bit chunks.
     * @param start_address The beginning of the memory region of interest.
     * @param end_address The end of the memory region of interest.
     * @returns Undefined.
     */
    int32_t revStride16Write_Word256(void* start_address, void* end_address);
#endif

    /* ------------ RANDOM READ --------------*/

#ifndef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers.
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomRead_Word32(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

    //32-bit systems only.
#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in random order by chasing 32-bit pointers.
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomRead_Word64(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers embedded in 128-bit memory words.
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomRead_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers embedded in 256-bit memory words.
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomRead_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

    /* ------------ RANDOM WRITE --------------*/

    //32-bit machines only
#ifndef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in random order by chasing 32-bit pointers. A pointer is read and written back with the same value before chasing to the next pointer. Thus, each memory address is a read followed by immediate write operation. 
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomWrite_Word32(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_64
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers. A pointer is read and written back with the same value before chasing to the next pointer. Thus, each memory address is a read followed by immediate write operation. 
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomWrite_Word64(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_128
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers embedded within 128-bit words. A 128-bit word is read and written back with the same value before chasing to the next location extracted as a 64-bit address in the 128-bit word. Thus, each memory address is a read followed by immediate write operation as well as a vector word extraction. 
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomWrite_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif

#ifdef HAS_WORD_256
    /**
     * @brief Walks over the allocated memory in random order by chasing 64-bit pointers embedded within 256-bit words. A 256-bit word is read and written back with the same value before chasing to the next location extracted as a 64-bit address in the 256-bit word. Thus, each memory address is a read followed by immediate write operation as well as a vector word extraction. 
     * @param first_address Starting address to deference.
     * @param last_touched_address The last visited address.
     * @param len The number of pointers to deference in a chain-like fashion.
     * @returns Undefined.
     */
    int32_t randomWrite_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len);
#endif
};

#endif
