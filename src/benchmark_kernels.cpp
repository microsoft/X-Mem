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
 * @brief Implementation file for benchmark kernel functions for doing the actual work we care about. :)
 *
 * Optimization tricks include:
 *   - UNROLL macros to manual loop unrolling. This reduces the relative branch overhead of the loop.
 *     We don't want to benchmark loops, we want to benchmark memory! But unrolling too much can hurt
 *     code size and instruction locality, potentially decreasing I-cache utilization and causing extra overheads.
 *     This is why we allow multiple unroll lengths at compile-time.
 *   - volatile keyword to prevent compiler from optimizing the code and removing instructions that we need.
 *     The compiler is too smart for its own good!
 */

//Headers
#include <benchmark_kernels.h>
#include <common.h>

//Libraries
#include <iostream>
#include <random>
#include <algorithm>
#include <time.h>
#if defined(ARCH_INTEL) && (defined(HAS_WORD_128) || defined(HAS_WORD_256) || defined(HAS_WORD_512)) 
//Intel intrinsics
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#endif

#if defined(ARCH_ARM) && defined(ARCH_ARM_NEON)
#include <arm_neon.h>
#endif

using namespace xmem;

#if defined(__gnu_linux__) && defined(ARCH_INTEL)

#ifdef HAS_WORD_128
#define my_32b_set_128b_word(a, b, c, d) _mm_set_epi32(a, b, c, d) //SSE2 intrinsic, corresponds to ??? instruction. Header: emmintrin.h
#define my_64b_set_128b_word(a, b) _mm_set_epi64x(a, b) //SSE2 intrinsic, corresponds to ??? instruction. Header: emmintrin.h
#define my_32b_extractLSB_128b(w) _mm_extract_epi32(w, 0) //SSE 4.1 intrinsic, corresponds to "pextrd" instruction. Header: smmintrin.h
#define my_64b_extractLSB_128b(w) _mm_extract_epi64(w, 0) //SSE 4.1 intrinsic, corresponds to "pextrq" instruction. Header: smmintrin.h
#endif

#ifdef HAS_WORD_256
#define my_32b_set_256b_word(a, b, c, d, e, f, g, h) _mm256_set_epi32(a, b, c, d, e, f, g, h) //AVX intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#define my_64b_set_256b_word(a, b, c, d) _mm256_set_epi64x(a, b, c, d) //AVX intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#define my_32b_extractLSB_256b(w) _mm256_extract_epi32(w, 0) //AVX intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#define my_64b_extractLSB_256b(w) _mm256_extract_epi64(w, 0) //AVX intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#endif


#if defined(HAS_WORD_512) 

#if !defined(ARCH_INTEL_MIC) //MIC (Knight's Corner) has partial ISA overlap with AVX-512. Neither are subsets of the other. What a headache. Knight's Corner also doesn't support legacy SSE or AVX stuff... agh!!!
#define my_32b_set_512b_word(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) //AVX-512 intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#define my_64b_set_512b_word(a, b, c, d, e, f, g, h) _mm512_set_epi64x(a, b, c, d, e, f, g, h) //AVX-512 intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#ifdef HAS_WORD_128 //Two-step extraction required for AVX-512. First to either 128 or 256-bit word, then down to 32/64 native words. We chose 128-bit words mid-step here.
#define my_32b_extractLSB_512b(w) _mm_extract_epi32(_mm512_extracti32x4_epi32(w, 0),0) //AVX-512 intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#define my_64b_extractLSB_512b(w) _mm_extract_epi32(_mm512_extracti64x2_epi64(w, 0),0) //AVX-512 intrinsic, corresponds to ??? (pseudo?) instruction. Header: immintrin.h
#endif
#else
//These are hacks to allow me to set a 512-bit word in Knight's Corner using 32-bit or 64-bit packed components. I store them on the stack one piece at a time, then use a 512-bit vector load to get them into a vector register. What a mess. Is there an easier way??
#define my_32b_set_512b_word(ret, scratchptr, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
scratchptr[0] = a;\
scratchptr[1] = b;\
scratchptr[2] = c;\
scratchptr[3] = d;\
scratchptr[4] = e;\
scratchptr[5] = f;\
scratchptr[6] = g;\
scratchptr[7] = h;\
scratchptr[8] = i;\
scratchptr[9] = j;\
scratchptr[10] = k;\
scratchptr[11] = l;\
scratchptr[12] = m;\
scratchptr[13] = n;\
scratchptr[14] = o;\
scratchptr[15] = p;\
ret = _mm512_load_epi32(scratchptr);

#define my_64b_set_512b_word(ret, scratchptr, a, b, c, d, e, f, g, h) \
scratchptr[0] = a;\
scratchptr[1] = b;\
scratchptr[2] = c;\
scratchptr[3] = d;\
scratchptr[4] = e;\
scratchptr[5] = f;\
scratchptr[6] = g;\
scratchptr[7] = h;\
ret = _mm512_load_epi64(scratchptr);

#define my_32b_extractLSB_512b(ret, scratchptr, w) \
_mm512_store_epi32(scratchptr, w);\
ret = scratchptr[0];

#define my_64b_extractLSB_512b(ret, scratchptr, w) \
_mm512_store_epi64(scratchptr, w);\
ret = scratchptr[0];

#endif

#endif

#endif

#if defined(ARCH_ARM) && (defined(ARCH_ARM_NEON) || defined(HAS_WORD_128))
#define my_32b_set_128b_word(a, b) vcombine_u32(a, b)
#define my_64b_set_128b_word(a, b) vcombine_u64(a, b)

#define my_32b_extractLSB_128b(w) static_cast<uint32_t>(vget_low_u64(w)) //NEON intrinsic, corresponds to "vmov" instruction. Header: arm_neon.h
#define my_64b_extractLSB_128b(w) vget_low_u64(w) //NEON intrinsic, corresponds to "vmov" instruction. Header: arm_neon.h
#endif

#if defined(HAS_WORD_512) && defined(ARCH_INTEL_MIC)
#define my_512b_load(p) _mm512_load_epi64(p) //This is needed because I get the compiler error 'error: class "__m512i" has no suitable assignment operator' using icc for Knight's Corner Xeon Phi. Also, I cannot use do a load on a volatile piece of memory. So the 512-bit code may not work...
#endif


#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
/* Hand-coded assembly functions for the 128-bit and 256-bit benchmark kernels on Windows x86-64 where applicable.
 * These are needed because the VC++ compiler, like gcc, optimizes away the vector instructions unless I use the volatile keyword.
 * But I can't use something like volatile Word256_t* because it is incompatible with _mm_load_si256() with VC++, etc.
 * Then, an alternative is to use inline assembly for Windows. However, the VC++ compiler does not support inline assembly in x86-64!
 * The only remaining choice for this unfortunate circumstance is to use hand-coded assembly on Windows x86-64 builds.
 * In this special case, I implemented the routine as a wrapper around the assembler function.
 */

#ifdef HAS_WORD_128
//128-bit
extern "C" int32_t win_x86_64_asm_forwSequentialRead_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revSequentialRead_Word128(Word128_t* last_word, Word128_t* first_word);
extern "C" int32_t win_x86_64_asm_forwSequentialWrite_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revSequentialWrite_Word128(Word128_t* last_word, Word128_t* first_word); 
extern "C" int32_t win_x86_64_asm_forwStride2Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride2Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride2Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride2Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride4Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride4Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride4Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride4Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride8Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride8Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride8Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride8Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride16Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride16Read_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride16Write_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride16Write_Word128(Word128_t* first_word, Word128_t* last_word);
#endif

#ifdef HAS_WORD_256
//256-bit
extern "C" int32_t win_x86_64_asm_forwSequentialRead_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revSequentialRead_Word256(Word256_t* last_word, Word256_t* first_word);
extern "C" int32_t win_x86_64_asm_forwSequentialWrite_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revSequentialWrite_Word256(Word256_t* last_word, Word256_t* first_word);
extern "C" int32_t win_x86_64_asm_forwStride2Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride2Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride2Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride2Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride4Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride4Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride4Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride4Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride8Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride8Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride8Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride8Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride16Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride16Read_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_forwStride16Write_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_revStride16Write_Word256(Word256_t* first_word, Word256_t* last_word);
#endif

//Dummies
#ifdef HAS_WORD_128
//128-bit
extern "C" int32_t win_x86_64_asm_dummy_forwSequentialLoop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revSequentialLoop_Word128(Word128_t* first_word, Word128_t* last_word); 
extern "C" int32_t win_x86_64_asm_dummy_forwStride2Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride2Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride4Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride4Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride8Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride8Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride16Loop_Word128(Word128_t* first_word, Word128_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride16Loop_Word128(Word128_t* first_word, Word128_t* last_word);
#endif

#ifdef HAS_WORD_256
//256-bit
extern "C" int32_t win_x86_64_asm_dummy_forwSequentialLoop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revSequentialLoop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride2Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride2Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride4Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride4Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride8Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride8Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_forwStride16Loop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int32_t win_x86_64_asm_dummy_revStride16Loop_Word256(Word256_t* first_word, Word256_t* last_word);
#endif
#endif


bool xmem::determine_sequential_kernel(rw_mode_t rw_mode, chunk_size_t chunk_size, int32_t stride_size, SequentialFunction* kernel_function, SequentialFunction* dummy_kernel_function) {
    switch (rw_mode) {
        case READ:
            switch (chunk_size) {
                case CHUNK_32b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialRead_Word32;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word32;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialRead_Word32;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word32;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Read_Word32;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word32;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Read_Word32;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word32;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Read_Word32;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word32;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Read_Word32;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word32;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Read_Word32;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word32;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Read_Word32;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word32;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Read_Word32;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word32;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Read_Word32;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word32;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#ifdef HAS_WORD_64
                case CHUNK_64b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialRead_Word64;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word64;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialRead_Word64;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word64;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Read_Word64;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word64;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Read_Word64;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word64;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Read_Word64;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word64;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Read_Word64;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word64;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Read_Word64;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word64;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Read_Word64;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word64;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Read_Word64;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word64;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Read_Word64;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word64;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_128
                case CHUNK_128b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialRead_Word128;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word128;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialRead_Word128;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word128;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Read_Word128;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word128;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Read_Word128;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word128;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Read_Word128;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word128;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Read_Word128;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word128;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Read_Word128;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word128;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Read_Word128;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word128;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Read_Word128;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word128;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Read_Word128;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word128;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_256
                case CHUNK_256b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialRead_Word256;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word256;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialRead_Word256;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word256;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Read_Word256;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word256;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Read_Word256;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word256;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Read_Word256;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word256;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Read_Word256;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word256;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Read_Word256;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word256;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Read_Word256;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word256;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Read_Word256;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word256;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Read_Word256;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word256;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_512
                case CHUNK_512b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialRead_Word512;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word512;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialRead_Word512;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word512;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Read_Word512;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word512;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Read_Word512;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word512;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Read_Word512;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word512;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Read_Word512;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word512;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Read_Word512;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word512;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Read_Word512;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word512;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Read_Word512;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word512;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Read_Word512;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word512;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif

                default:
                    return false;
            }
            return true;

        case WRITE:
            switch (chunk_size) {
                case CHUNK_32b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialWrite_Word32;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word32;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialWrite_Word32;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word32;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Write_Word32;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word32;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Write_Word32;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word32;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Write_Word32;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word32;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Write_Word32;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word32;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Write_Word32;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word32;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Write_Word32;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word32;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Write_Word32;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word32;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Write_Word32;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word32;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#ifdef HAS_WORD_64
                case CHUNK_64b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialWrite_Word64;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word64;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialWrite_Word64;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word64;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Write_Word64;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word64;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Write_Word64;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word64;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Write_Word64;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word64;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Write_Word64;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word64;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Write_Word64;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word64;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Write_Word64;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word64;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Write_Word64;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word64;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Write_Word64;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word64;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_128
                case CHUNK_128b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialWrite_Word128;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word128;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialWrite_Word128;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word128;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Write_Word128;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word128;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Write_Word128;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word128;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Write_Word128;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word128;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Write_Word128;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word128;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Write_Word128;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word128;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Write_Word128;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word128;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Write_Word128;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word128;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Write_Word128;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word128;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_256
                case CHUNK_256b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialWrite_Word256;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word256;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialWrite_Word256;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word256;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Write_Word256;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word256;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Write_Word256;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word256;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Write_Word256;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word256;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Write_Word256;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word256;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Write_Word256;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word256;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Write_Word256;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word256;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Write_Word256;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word256;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Write_Word256;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word256;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif
#ifdef HAS_WORD_512
                case CHUNK_512b:
                    switch (stride_size) {
                        case 1:
                            *kernel_function = &forwSequentialWrite_Word512;
                            *dummy_kernel_function = &dummy_forwSequentialLoop_Word512;
                            return true;
                        case -1:
                            *kernel_function = &revSequentialWrite_Word512;
                            *dummy_kernel_function = &dummy_revSequentialLoop_Word512;
                            return true;
                        case 2:
                            *kernel_function = &forwStride2Write_Word512;
                            *dummy_kernel_function = &dummy_forwStride2Loop_Word512;
                            return true;
                        case -2:
                            *kernel_function = &revStride2Write_Word512;
                            *dummy_kernel_function = &dummy_revStride2Loop_Word512;
                            return true;
                        case 4:
                            *kernel_function = &forwStride4Write_Word512;
                            *dummy_kernel_function = &dummy_forwStride4Loop_Word512;
                            return true;
                        case -4:
                            *kernel_function = &revStride4Write_Word512;
                            *dummy_kernel_function = &dummy_revStride4Loop_Word512;
                            return true;
                        case 8:
                            *kernel_function = &forwStride8Write_Word512;
                            *dummy_kernel_function = &dummy_forwStride8Loop_Word512;
                            return true;
                        case -8:
                            *kernel_function = &revStride8Write_Word512;
                            *dummy_kernel_function = &dummy_revStride8Loop_Word512;
                            return true;
                        case 16:
                            *kernel_function = &forwStride16Write_Word512;
                            *dummy_kernel_function = &dummy_forwStride16Loop_Word512;
                            return true;
                        case -16:
                            *kernel_function = &revStride16Write_Word512;
                            *dummy_kernel_function = &dummy_revStride16Loop_Word512;
                            return true;
                        default:
                            return false;
                    }
                    return true;
#endif

                default:
                    return false;
            }
            return true;

        default:
            return false;
    }

    return false; //shouldn't reach this point
}
    
bool xmem::determine_random_kernel(rw_mode_t rw_mode, chunk_size_t chunk_size, RandomFunction* kernel_function, RandomFunction* dummy_kernel_function) {
    switch (rw_mode) {
        case READ:
            switch (chunk_size) {
                //special case on 32-bit architectures only.
#ifndef HAS_WORD_64
                case CHUNK_32b:
                    *kernel_function = &randomRead_Word32;
                    *dummy_kernel_function = &dummy_randomLoop_Word32;
                    return true;
#endif
#ifdef HAS_WORD_64
                case CHUNK_64b:
                    *kernel_function = &randomRead_Word64;
                    *dummy_kernel_function = &dummy_randomLoop_Word64;
                    return true;
#endif
#ifdef HAS_WORD_128
                case CHUNK_128b:
                    *kernel_function = &randomRead_Word128;
                    *dummy_kernel_function = &dummy_randomLoop_Word128;
                    return true;
#endif
#ifdef HAS_WORD_256
                case CHUNK_256b:
                    *kernel_function = &randomRead_Word256;
                    *dummy_kernel_function = &dummy_randomLoop_Word256;
                    return true;
#endif
#ifdef HAS_WORD_512
                case CHUNK_512b:
                    *kernel_function = &randomRead_Word512;
                    *dummy_kernel_function = &dummy_randomLoop_Word512;
                    return true;
#endif
                default:
                    return false;
            }
            return true;

        case WRITE:
            switch (chunk_size) {
                //special case on 32-bit architectures only.
#ifndef HAS_WORD_64
                case CHUNK_32b:
                    *kernel_function = &randomWrite_Word32;
                    *dummy_kernel_function = &dummy_randomLoop_Word32;
                    return true;
#endif
#ifdef HAS_WORD_64
                case CHUNK_64b:
                    *kernel_function = &randomWrite_Word64;
                    *dummy_kernel_function = &dummy_randomLoop_Word64;
                    return true;
#endif
#ifdef HAS_WORD_128
                case CHUNK_128b:
                    *kernel_function = &randomWrite_Word128;
                    *dummy_kernel_function = &dummy_randomLoop_Word128;
                    return true;
#endif
#ifdef HAS_WORD_256
                case CHUNK_256b:
                    *kernel_function = &randomWrite_Word256;
                    *dummy_kernel_function = &dummy_randomLoop_Word256;
                    return true;
#endif
#ifdef HAS_WORD_512
                case CHUNK_512b:
                    *kernel_function = &randomWrite_Word512;
                    *dummy_kernel_function = &dummy_randomLoop_Word512;
                    return true;
#endif
                default:
                    return false;
            }
            return true;

        default:
            return false;
    }

    return false;
}

bool xmem::build_random_pointer_permutation(void* start_address, void* end_address, chunk_size_t chunk_size) {
    if (g_verbose)
        std::cout << "Preparing a memory region under test. This might take a while...";

    size_t length = reinterpret_cast<uint8_t*>(end_address) - reinterpret_cast<uint8_t*>(start_address); //length of region in bytes
    size_t num_pointers = 0; //Number of pointers that fit into the memory region of interest
    switch (chunk_size) {
        //special case on 32-bit architectures only.
#ifndef HAS_WORD_64
        case CHUNK_32b:
            num_pointers = length / sizeof(Word32_t);
            break;
#endif
#ifdef HAS_WORD_64
        case CHUNK_64b:
            num_pointers = length / sizeof(Word64_t);
            break;
#endif
#ifdef HAS_WORD_128
        case CHUNK_128b:
            num_pointers = length / sizeof(Word128_t);
            break;
#endif
#ifdef HAS_WORD_256
        case CHUNK_256b:
            num_pointers = length / sizeof(Word256_t);
            break;
#endif
#ifdef HAS_WORD_512
        case CHUNK_512b:
            num_pointers = length / sizeof(Word512_t);
            break;
#endif
        default:
            std::cerr << "ERROR: Chunk size must be at least "
            //special case for 32-bit architectures
#ifndef HAS_WORD_64
            <<"32"
#endif
#ifdef HAS_WORD_64
            <<"64"
#endif
            << "bits for building a random pointer permutation. This should not have happened." << std::endl;
            return false;
    }
            
    std::mt19937_64 gen(time(NULL)); //Mersenne Twister random number generator, seeded at current time
    
    //Do a random shuffle of memory pointers. 
    //I had originally used a random Hamiltonian Cycle generator, but this was much slower and aside from
    //rare instances, did not make any difference in random-access performance measurement.
#ifdef HAS_WORD_64
    Word64_t* mem_region_base = reinterpret_cast<Word64_t*>(start_address);
#else //special case for 32-bit architectures
    Word32_t* mem_region_base = reinterpret_cast<Word32_t*>(start_address);
#endif
    switch (chunk_size) {
#ifdef HAS_WORD_64
        case CHUNK_64b:
            for (size_t i = 0; i < num_pointers; i++) { //Initialize pointers to point at themselves (identity mapping)
                mem_region_base[i] = reinterpret_cast<Word64_t>(mem_region_base+i);
            }
            std::shuffle(mem_region_base, mem_region_base + num_pointers, gen);
            break;
#else //special case for 32-bit architectures
        case CHUNK_32b:
            for (size_t i = 0; i < num_pointers; i++) { //Initialize pointers to point at themselves (identity mapping)
                mem_region_base[i] = reinterpret_cast<Word32_t>(mem_region_base+i);
            }
            std::shuffle(mem_region_base, mem_region_base + num_pointers, gen);
            break;
#endif
#ifdef HAS_WORD_128
        case CHUNK_128b:
            for (size_t i = 0; i < num_pointers; i++) { //Initialize pointers to point at themselves (identity mapping)
#ifdef HAS_WORD_64
                mem_region_base[i*2] = reinterpret_cast<Word64_t>(mem_region_base+(i*2));
                mem_region_base[(i*2)+1] = 0xFFFFFFFFFFFFFFFF; //1-fill upper 64 bits
#else //special case for 32-bit architectures
                mem_region_base[i*4] = reinterpret_cast<Word32_t>(mem_region_base+(i*4));
                mem_region_base[(i*4)+1] = 0xFFFFFFFF; //1-fill upper 96 bits
                mem_region_base[(i*4)+2] = 0xFFFFFFFF; 
                mem_region_base[(i*4)+3] = 0xFFFFFFFF; 
#endif
            }
            std::shuffle(reinterpret_cast<Word128_t*>(mem_region_base), reinterpret_cast<Word128_t*>(mem_region_base) + num_pointers, gen);
            break;
#endif
#ifdef HAS_WORD_256
        case CHUNK_256b:
            for (size_t i = 0; i < num_pointers; i++) { //Initialize pointers to point at themselves (identity mapping)
#ifdef HAS_WORD_64
                mem_region_base[i*4] = reinterpret_cast<Word64_t>(mem_region_base+(i*4));
                mem_region_base[(i*4)+1] = 0xFFFFFFFFFFFFFFFF; //1-fill upper 192 bits
                mem_region_base[(i*4)+2] = 0xFFFFFFFFFFFFFFFF; 
                mem_region_base[(i*4)+3] = 0xFFFFFFFFFFFFFFFF;
#else //special case for 32-bit architectures
                mem_region_base[i*8] = reinterpret_cast<Word32_t>(mem_region_base+(i*8));
                mem_region_base[(i*8)+1] = 0xFFFFFFFF; //1-fill upper 224 bits
                mem_region_base[(i*8)+2] = 0xFFFFFFFF;
                mem_region_base[(i*8)+3] = 0xFFFFFFFF;
                mem_region_base[(i*8)+4] = 0xFFFFFFFF;
                mem_region_base[(i*8)+5] = 0xFFFFFFFF;
                mem_region_base[(i*8)+6] = 0xFFFFFFFF;
                mem_region_base[(i*8)+7] = 0xFFFFFFFF;
#endif
            }
            std::shuffle(reinterpret_cast<Word256_t*>(mem_region_base), reinterpret_cast<Word256_t*>(mem_region_base) + num_pointers, gen);
            break;
#endif
#ifdef HAS_WORD_512
        case CHUNK_512b:
            for (size_t i = 0; i < num_pointers; i++) { //Initialize pointers to point at themselves (identity mapping)
#ifdef HAS_WORD_64
                mem_region_base[i*4] = reinterpret_cast<Word64_t>(mem_region_base+(i*4));
                mem_region_base[(i*4)+1] = 0xFFFFFFFFFFFFFFFF; //1-fill upper 448 bits
                mem_region_base[(i*4)+2] = 0xFFFFFFFFFFFFFFFF; 
                mem_region_base[(i*4)+3] = 0xFFFFFFFFFFFFFFFF;
                mem_region_base[(i*4)+4] = 0xFFFFFFFFFFFFFFFF;
                mem_region_base[(i*4)+5] = 0xFFFFFFFFFFFFFFFF;
                mem_region_base[(i*4)+6] = 0xFFFFFFFFFFFFFFFF;
                mem_region_base[(i*4)+7] = 0xFFFFFFFFFFFFFFFF;
#else //special case for 32-bit architectures
                mem_region_base[i*8] = reinterpret_cast<Word32_t>(mem_region_base+(i*8));
                mem_region_base[(i*8)+1] = 0xFFFFFFFF; //1-fill upper 480 bits
                mem_region_base[(i*8)+2] = 0xFFFFFFFF;
                mem_region_base[(i*8)+3] = 0xFFFFFFFF;
                mem_region_base[(i*8)+4] = 0xFFFFFFFF;
                mem_region_base[(i*8)+5] = 0xFFFFFFFF;
                mem_region_base[(i*8)+6] = 0xFFFFFFFF;
                mem_region_base[(i*8)+7] = 0xFFFFFFFF;
                mem_region_base[(i*8)+8] = 0xFFFFFFFF;
                mem_region_base[(i*8)+9] = 0xFFFFFFFF;
                mem_region_base[(i*8)+10] = 0xFFFFFFFF;
                mem_region_base[(i*8)+11] = 0xFFFFFFFF;
                mem_region_base[(i*8)+12] = 0xFFFFFFFF;
                mem_region_base[(i*8)+13] = 0xFFFFFFFF;
                mem_region_base[(i*8)+14] = 0xFFFFFFFF;
                mem_region_base[(i*8)+15] = 0xFFFFFFFF;
#endif
            }
            std::shuffle(reinterpret_cast<Word512_t*>(mem_region_base), reinterpret_cast<Word512_t*>(mem_region_base) + num_pointers, gen);
            break;
#endif
        default:
            std::cerr << "ERROR: Got an invalid chunk size. This should not have happened." << std::endl;
            return false;
    }

    if (g_verbose) {
        std::cout << "done" << std::endl;
        std::cout << std::endl;
    }

    return true;
}

/***********************************************************************
 ***********************************************************************
 ********************** LATENCY-RELATED BENCHMARK KERNELS **************
 ***********************************************************************
 ***********************************************************************/

/* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

int32_t xmem::dummy_chasePointers(uintptr_t*, uintptr_t**, size_t len) {
    volatile uintptr_t placeholder = 0; //Try to defeat compiler optimizations removing this method
    return 0;
}

/* -------------------- CORE BENCHMARK ROUTINES -------------------------- */

int32_t xmem::chasePointers(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
    volatile uintptr_t* p = first_address;
    UNROLL512(p = reinterpret_cast<uintptr_t*>(*p);)
    *last_touched_address = const_cast<uintptr_t*>(p);
    return 0;
}


/***********************************************************************
 ***********************************************************************
 ******************* THROUGHPUT-RELATED BENCHMARK KERNELS **************
 ***********************************************************************
 ***********************************************************************/

/* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

int32_t xmem::dummy_empty(void*, void*) { 
    return 0;
}
        
/* ------------ SEQUENTIAL LOOP --------------*/

int32_t xmem::dummy_forwSequentialLoop_Word32(void* start_address, void* end_address) {
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
        UNROLL1024(wordptr++;) 
        placeholder = 0;
    }
    return placeholder;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_forwSequentialLoop_Word64(void* start_address, void* end_address) {
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
        UNROLL512(wordptr++;) 
        placeholder = 0;
    }
    return placeholder;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_forwSequentialLoop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwSequentialLoop_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
        UNROLL256(wordptr++;) 
        placeholder = 0;
    }
    return placeholder;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_forwSequentialLoop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwSequentialLoop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#endif
#ifdef __gnu_linux__
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
        UNROLL128(wordptr++;) 
        placeholder = 0;
    }
    return placeholder;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_forwSequentialLoop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_forwSequentialLoop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_forwSequentialLoop_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words on Windows are not currently supported.
#endif
#ifdef __gnu_linux__
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address), *endptr = static_cast<Word512_t*>(end_address); wordptr < endptr;) {
        UNROLL64(wordptr++;) 
        placeholder = 0;
    }
    return placeholder;
#endif
}
#endif

int32_t xmem::dummy_revSequentialLoop_Word32(void* start_address, void* end_address) { 
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
        UNROLL1024(wordptr--;)
        placeholder = 0;
    }
    return placeholder;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_revSequentialLoop_Word64(void* start_address, void* end_address) {
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
        UNROLL512(wordptr--;)
        placeholder = 0;
    }
    return placeholder;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_revSequentialLoop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revSequentialLoop_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#endif
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
        UNROLL256(wordptr--;) 
        placeholder = 0;
    }
    return placeholder;
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_revSequentialLoop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revSequentialLoop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#endif
#ifdef __gnu_linux__
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address), *begptr = static_cast<Word256_t*>(start_address); wordptr > begptr;) {
        UNROLL128(wordptr--;) 
        placeholder = 0;
    }
    return placeholder;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_revSequentialLoop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_revSequentialLoop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_revSequentialLoop_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not currently supported on Windows.
#endif
#ifdef __gnu_linux__
    volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address), *begptr = static_cast<Word512_t*>(start_address); wordptr > begptr;) {
        UNROLL64(wordptr--;) 
        placeholder = 0;
    }
    return placeholder;
#endif
}
#endif

/* ------------ STRIDE 2 LOOP --------------*/

int32_t xmem::dummy_forwStride2Loop_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
        UNROLL512(wordptr += 2;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_forwStride2Loop_Word64(void* start_address, void* end_address) {
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
        UNROLL256(wordptr += 2;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_forwStride2Loop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride2Loop_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
        UNROLL128(wordptr += 2;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_forwStride2Loop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride2Loop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
        UNROLL64(wordptr += 2;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_forwStride2Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_forwStride2Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
//    return win_x86_64_asm_dummy_forwStride2Loop_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 32) {
        UNROLL32(wordptr += 2;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::dummy_revStride2Loop_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
        UNROLL512(wordptr -= 2;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_revStride2Loop_Word64(void* start_address, void* end_address) {
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
        UNROLL256(wordptr -= 2;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_revStride2Loop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride2Loop_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
        UNROLL128(wordptr -= 2;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_revStride2Loop_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride2Loop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
        UNROLL64(wordptr -= 2;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_revStride2Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_revStride2Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_revStride2Loop_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
#error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 32) {
        UNROLL32(wordptr -= 2;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 4 LOOP --------------*/

int32_t xmem::dummy_forwStride4Loop_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
        UNROLL256(wordptr += 4;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_forwStride4Loop_Word64(void* start_address, void* end_address) {
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 128) {
        UNROLL128(wordptr += 4;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_forwStride4Loop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride4Loop_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 64) {
        UNROLL64(wordptr += 4;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_forwStride4Loop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride4Loop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
        UNROLL32(wordptr += 4;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_forwStride4Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_forwStride4Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_forwStride4Loop_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
#error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 16) {
        UNROLL16(wordptr += 4;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::dummy_revStride4Loop_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
        UNROLL256(wordptr -= 4;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_revStride4Loop_Word64(void* start_address, void* end_address) {
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
        UNROLL128(wordptr -= 4;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_revStride4Loop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride4Loop_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
        UNROLL64(wordptr -= 4;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_revStride4Loop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride4Loop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
        UNROLL32(wordptr -= 4;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_revStride4Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_revStride4Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_revStride4Loop_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 16) {
        UNROLL16(wordptr -= 4;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 8 LOOP --------------*/

int32_t xmem::dummy_forwStride8Loop_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
        UNROLL128(wordptr += 8;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_forwStride8Loop_Word64(void* start_address, void* end_address) {
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
        UNROLL64(wordptr += 8;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_forwStride8Loop_Word128(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride8Loop_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
        UNROLL32(wordptr += 8;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_forwStride8Loop_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride8Loop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
        UNROLL16(wordptr += 8;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_forwStride8Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_forwStride8Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_forwStride8Loop_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 8) {
        UNROLL8(wordptr += 8;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::dummy_revStride8Loop_Word32(void* start_address, void* end_address) { 
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
        UNROLL128(wordptr -= 8;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_revStride8Loop_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
        UNROLL64(wordptr -= 8;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_revStride8Loop_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride8Loop_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
        UNROLL32(wordptr -= 8;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_revStride8Loop_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride8Loop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
        UNROLL16(wordptr -= 8;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_revStride8Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_revStride8Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_revStride8Loop_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 8) {
        UNROLL8(wordptr -= 8;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 16 LOOP --------------*/

int32_t xmem::dummy_forwStride16Loop_Word32(void* start_address, void* end_address) { 
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
        UNROLL64(wordptr += 16;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_forwStride16Loop_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
        UNROLL32(wordptr += 16;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_forwStride16Loop_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride16Loop_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
        UNROLL16(wordptr += 16;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_forwStride16Loop_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_forwStride16Loop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 8) {
        UNROLL8(wordptr += 16;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_forwStride16Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_forwStride16Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_forwStride16Loop_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not currently supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 4) {
        UNROLL4(wordptr += 16;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::dummy_revStride16Loop_Word32(void* start_address, void* end_address) { 
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
        UNROLL64(wordptr -= 16;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::dummy_revStride16Loop_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
        UNROLL32(wordptr -= 16;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_revStride16Loop_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride16Loop_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
        UNROLL16(wordptr -= 16;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_revStride16Loop_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_dummy_revStride16Loop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
        UNROLL8(wordptr -= 16;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_revStride16Loop_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_revStride16Loop_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_dummy_revStride16Loop_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 4) {
        UNROLL4(wordptr -= 16;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ RANDOM LOOP --------------*/

#ifndef HAS_WORD_64 //special case: 32-bit architectures
int32_t xmem::dummy_randomLoop_Word32(uintptr_t*, uintptr_t**, size_t len) {
    volatile uintptr_t* placeholder = NULL; //Try to defeat compiler optimizations removing this method
    return 0;
}
#endif

#ifdef HAS_WORD_64
int32_t xmem::dummy_randomLoop_Word64(uintptr_t*, uintptr_t**, size_t len) {
    volatile uintptr_t* placeholder = NULL; //Try to defeat compiler optimizations removing this method
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::dummy_randomLoop_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    //TODO: check that the compiler generates this code correctly
    volatile Word128_t* placeholder = reinterpret_cast<Word128_t*>(first_address);
    register Word128_t val;
    volatile uintptr_t val_extract;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

#ifdef HAS_WORD_64
    UNROLL256(val_extract = my_64b_extractLSB_128b(val);) //Extract 64 LSB.
#else //special case: 32-bit machines
    UNROLL256(val_extract = my_32b_extractLSB_128b(val);) //Extract 32 LSB.
#endif

    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::dummy_randomLoop_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    //TODO: check that the compiler generates this code correctly
    volatile Word256_t* placeholder = reinterpret_cast<Word256_t*>(first_address);
    register Word256_t val;
    volatile uintptr_t val_extract;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

#ifdef HAS_WORD_64
        UNROLL128(val_extract = my_64b_extractLSB_256b(val);) //Extract 64 LSB.
#else //special case: 32-bit machines
        UNROLL128(val_extract = my_32b_extractLSB_256b(val);) //Extract 32 LSB.
#endif

    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::dummy_randomLoop_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::dummy_randomLoop_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    //TODO: check that the compiler generates this code correctly
    volatile Word512_t* placeholder = reinterpret_cast<Word512_t*>(first_address);
    register Word512_t val;
    volatile uintptr_t val_extract;

    uint64_t scratchptr[8];
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    UNROLL64(my_64b_extractLSB_512b(val_extract, scratchptr, val);) //Extract 64 LSB.

    return 0;
#endif
}
#endif

/* -------------------- CORE BENCHMARK ROUTINES -------------------------- 
 *
 * These routines access the memory in different ways for each benchmark type.
 * Optimization tricks include:
 *   - register keyword to hint to compiler values that should not be used in memory access
 *   - UNROLL macros to manual loop unrolling. This reduces the relative branch overhead of the loop.
 *     We don't want to benchmark loops, we want to benchmark memory!
 *   - volatile keyword to prevent compiler from optimizing the code and removing instructions that we need.
 *     The compiler is too smart for its own good!
 *   - Hardcoding stride and chunk sizes in each benchmark snippet, so that way they do not have to waste cycles being computed at runtime.
 *     This makes code harder to maintain and to read but ensures more accurate memory benchmarking.
 *
 * ----------------------------------------------------------------------- */

/* ------------ SEQUENTIAL READ --------------*/

int32_t xmem::forwSequentialRead_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
        UNROLL1024(val = *wordptr++;) 
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwSequentialRead_Word64(void* start_address, void* end_address) {
    register Word64_t val;
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
        UNROLL512(val = *wordptr++;)
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwSequentialRead_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwSequentialRead_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
        UNROLL256(val = *wordptr++;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwSequentialRead_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwSequentialRead_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
        UNROLL128(val = *wordptr++;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwSequentialRead_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwSequentialRead_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
//    return win_x86_64_asm_forwSequentialRead_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address), *endptr = static_cast<Word512_t*>(end_address); wordptr < endptr;) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address), *endptr = static_cast<Word512_t*>(end_address); wordptr < endptr;) {
        //UNROLL64(val = *wordptr++;)
        UNROLL64(val = my_512b_load(wordptr++);)
    }
    return 0;
#endif
}
#endif

int32_t xmem::revSequentialRead_Word32(void* start_address, void* end_address) { 
    register Word32_t val;
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
        UNROLL1024(val = *wordptr--;)
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revSequentialRead_Word64(void* start_address, void* end_address) {
    register Word64_t val;
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
        UNROLL512(val = *wordptr--;)
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revSequentialRead_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revSequentialRead_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
        UNROLL256(val = *wordptr--;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revSequentialRead_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revSequentialRead_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address), *begptr = static_cast<Word256_t*>(start_address); wordptr > begptr;) {
        UNROLL128(val = *wordptr--;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revSequentialRead_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revSequentialRead_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revSequentialRead_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address), *begptr = static_cast<Word512_t*>(start_address); wordptr > begptr;) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address), *begptr = static_cast<Word512_t*>(start_address); wordptr > begptr;) {
        //UNROLL64(val = *wordptr--;)
        UNROLL64(val = my_512b_load(wordptr--);)
    }
    return 0;
#endif
}
#endif

/* ------------ SEQUENTIAL WRITE --------------*/

int32_t xmem::forwSequentialWrite_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF; 
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
        UNROLL1024(*wordptr++ = val;) 
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwSequentialWrite_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
        UNROLL512(*wordptr++ = val;) 
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwSequentialWrite_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwSequentialWrite_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
        UNROLL256(*wordptr++ = val;) 
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwSequentialWrite_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwSequentialWrite_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
        UNROLL128(*wordptr++ = val;) 
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwSequentialWrite_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwSequentialWrite_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwSequentialWrite_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address), *endptr = static_cast<Word512_t*>(end_address); wordptr < endptr;) {
        UNROLL64(*wordptr++ = val;) 
    }
    return 0;
#endif
}
#endif

int32_t xmem::revSequentialWrite_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF;
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
        UNROLL1024(*wordptr-- = val;)
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revSequentialWrite_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF;
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
        UNROLL512(*wordptr-- = val;)
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revSequentialWrite_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revSequentialWrite_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
        UNROLL256(*wordptr-- = val;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revSequentialWrite_Word256(void* start_address, void* end_address) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revSequentialWrite_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address), *begptr = static_cast<Word256_t*>(start_address); wordptr > begptr;) {
        UNROLL128(*wordptr-- = val;)
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revSequentialWrite_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revSequentialWrite_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revSequentialWrite_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address), *begptr = static_cast<Word512_t*>(start_address); wordptr > begptr;) {
        UNROLL64(*wordptr-- = val;)
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 2 READ --------------*/

int32_t xmem::forwStride2Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
        UNROLL512(val = *wordptr; wordptr += 2;) 
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride2Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
        UNROLL256(val = *wordptr; wordptr += 2;) 
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride2Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride2Read_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr += 2;) 
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride2Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride2Read_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr += 2;) 
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride2Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride2Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride2Read_Word512(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 32) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 32) {
        //UNROLL32(val = *wordptr; wordptr += 2;) 
        UNROLL32(val = my_512b_load(wordptr); wordptr += 2;) 
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride2Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
        UNROLL512(val = *wordptr; wordptr -= 2;) 
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride2Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
        UNROLL256(val = *wordptr; wordptr -= 2;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride2Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride2Read_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr -= 2;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride2Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride2Read_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr -= 2;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride2Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride2Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride2Read_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 32) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 32) {
        //UNROLL32(val = *wordptr; wordptr -= 2;)
        UNROLL32(val = my_512b_load(wordptr); wordptr -= 2;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 2 WRITE --------------*/

int32_t xmem::forwStride2Write_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
        UNROLL512(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride2Write_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
        UNROLL256(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride2Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride2Write_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride2Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride2Write_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride2Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride2Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride2Write_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride2Write_Word32(void* start_address, void* end_address) { 
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
        UNROLL512(*wordptr = val; wordptr -= 2;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride2Write_Word64(void* start_address, void* end_address) { 
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
        UNROLL256(*wordptr = val; wordptr -= 2;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride2Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride2Write_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr -= 2;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride2Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride2Write_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr -= 2;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride2Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride2Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride2Write_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr += 2;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 4 READ --------------*/

int32_t xmem::forwStride4Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
        UNROLL256(val = *wordptr; wordptr += 4;) 
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride4Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr += 4;) 
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride4Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride4Read_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr += 4;) 
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride4Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride4Read_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr += 4;) 
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride4Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride4Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride4Read_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 16) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 16) {
        //UNROLL16(val = *wordptr; wordptr += 4;) 
        UNROLL16(val = my_512b_load(wordptr); wordptr += 4;) 
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride4Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
        UNROLL256(val = *wordptr; wordptr -= 4;) 
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride4Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr -= 4;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride4Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride4Read_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr -= 4;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride4Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride4Read_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr -= 4;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride4Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride4Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride4Read_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 16) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 16) {
        //UNROLL16(val = *wordptr; wordptr -= 4;)
        UNROLL16(val = my_512b_load(wordptr); wordptr -= 4;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 4 WRITE --------------*/

int32_t xmem::forwStride4Write_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
        UNROLL256(*wordptr = val; wordptr += 4;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride4Write_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr += 4;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride4Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride4Write_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr += 4;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride4Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride4Write_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr += 4;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride4Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride4Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride4Write_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr += 4;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride4Write_Word32(void* start_address, void* end_address) { 
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
        UNROLL256(*wordptr = val; wordptr -= 4;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride4Write_Word64(void* start_address, void* end_address) { 
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr -= 4;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride4Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride4Write_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr -= 4;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride4Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride4Write_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr -= 4;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride4Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride4Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride4Write_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr -= 4;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 8 READ --------------*/

int32_t xmem::forwStride8Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr += 8;) 
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride8Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr += 8;) 
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride8Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride8Read_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr += 8;) 
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride8Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride8Read_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
        UNROLL16(val = *wordptr; wordptr += 8;) 
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride8Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride8Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride8Read_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 8) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 8) {
        //UNROLL8(val = *wordptr; wordptr += 8;) 
        UNROLL8(val = my_512b_load(wordptr); wordptr += 8;) 
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride8Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
        UNROLL128(val = *wordptr; wordptr -= 8;) 
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride8Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr -= 8;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride8Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride8Read_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr -= 8;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride8Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride8Read_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
        UNROLL16(val = *wordptr; wordptr -= 8;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride8Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride8Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride8Read_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 8) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 8) {
        //UNROLL8(val = *wordptr; wordptr -= 8;)
        UNROLL8(val = my_512b_load(wordptr); wordptr -= 8;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 8 WRITE --------------*/

int32_t xmem::forwStride8Write_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr += 8;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride8Write_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr += 8;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride8Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride8Write_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr += 8;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride8Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride8Write_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr += 8;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride8Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride8Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride8Write_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 8) {
        UNROLL8(*wordptr = val; wordptr += 8;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride8Write_Word32(void* start_address, void* end_address) { 
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
        UNROLL128(*wordptr = val; wordptr -= 8;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride8Write_Word64(void* start_address, void* end_address) { 
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr -= 8;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride8Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride8Write_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr -= 8;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride8Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride8Write_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr -= 8;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride8Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride8Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride8Write_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 8) {
        UNROLL8(*wordptr = val; wordptr -= 8;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 16 READ --------------*/

int32_t xmem::forwStride16Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr += 16;) 
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride16Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr += 16;) 
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride16Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride16Read_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
        UNROLL16(val = *wordptr; wordptr += 16;) 
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride16Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride16Read_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 8) {
        UNROLL8(val = *wordptr; wordptr += 16;) 
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride16Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride16Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride16Read_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 4) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 4) {
        //UNROLL4(val = *wordptr; wordptr += 16;) 
        UNROLL4(val = my_512b_load(wordptr); wordptr += 16;) 
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride16Read_Word32(void* start_address, void* end_address) {
    register Word32_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
        UNROLL64(val = *wordptr; wordptr -= 16;) 
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride16Read_Word64(void* start_address, void* end_address) { 
    register Word64_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
        UNROLL32(val = *wordptr; wordptr -= 16;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride16Read_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride16Read_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
        UNROLL16(val = *wordptr; wordptr -= 16;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride16Read_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride16Read_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
        UNROLL8(val = *wordptr; wordptr -= 16;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride16Read_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride16Read_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride16Read_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    //for (volatile Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 4) {
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 4) {
        //UNROLL4(val = *wordptr; wordptr -= 16;)
        UNROLL4(val = my_512b_load(wordptr); wordptr -= 16;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }
    return 0;
#endif
}
#endif

/* ------------ STRIDE 16 WRITE --------------*/

int32_t xmem::forwStride16Write_Word32(void* start_address, void* end_address) {
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr += 16;)
        if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::forwStride16Write_Word64(void* start_address, void* end_address) {
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr += 16;)
        if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::forwStride16Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride16Write_Word128(static_cast<Word128_t*>(start_address), static_cast<Word128_t*>(end_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr += 16;)
        if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::forwStride16Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_forwStride16Write_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 8) {
        UNROLL8(*wordptr = val; wordptr += 16;)
        if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::forwStride16Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::forwStride16Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_forwStride16Write_Word512(static_cast<Word512_t*>(start_address), static_cast<Word512_t*>(end_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(start_address); i < len; i += 4) {
        UNROLL4(*wordptr = val; wordptr += 16;)
        if (wordptr >= static_cast<Word512_t*>(end_address)) //end, modulo
            wordptr -= len;
    }
    return 0;
#endif
}
#endif

int32_t xmem::revStride16Write_Word32(void* start_address, void* end_address) { 
    register Word32_t val = 0xFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word32_t);
    for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
        UNROLL64(*wordptr = val; wordptr -= 16;)
        if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
            wordptr += len;
    }

    return 0;
}

#ifdef HAS_WORD_64
int32_t xmem::revStride16Write_Word64(void* start_address, void* end_address) { 
    register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word64_t);
    for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
        UNROLL32(*wordptr = val; wordptr -= 16;)
        if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
            wordptr += len;
    }

    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::revStride16Write_Word128(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride16Write_Word128(static_cast<Word128_t*>(end_address), static_cast<Word128_t*>(start_address));
#else
    register Word128_t val;
    val = my_64b_set_128b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word128_t);
    for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
        UNROLL16(*wordptr = val; wordptr -= 16;)
        if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
            wordptr += len;
    }

    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::revStride16Write_Word256(void* start_address, void* end_address) { 
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return win_x86_64_asm_revStride16Write_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#else
    register Word256_t val;
    val = my_64b_set_256b_word(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word256_t);
    for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
        UNROLL8(*wordptr = val; wordptr -= 16;)
        if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
            wordptr += len;
    }

    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::revStride16Write_Word512(void* start_address, void* end_address) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::revStride16Write_Word512(void* start_address, void* end_address) { 
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    //return win_x86_64_asm_revStride16Write_Word512(static_cast<Word512_t*>(end_address), static_cast<Word512_t*>(start_address));
    #error 512-bit words are not supported on Windows.
#else
    register Word512_t val;
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    my_64b_set_512b_word(val, scratchptr, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
    register uint32_t i = 0;
    register uint32_t len = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end_address)-reinterpret_cast<uintptr_t>(start_address)) / sizeof(Word512_t);
    for (Word512_t* wordptr = static_cast<Word512_t*>(end_address); i < len; i += 4) {
        UNROLL4(*wordptr = val; wordptr -= 16;)
        if (wordptr <= static_cast<Word512_t*>(start_address)) //end, modulo
            wordptr += len;
    }

    return 0;
#endif
}
#endif

/* ------------ RANDOM READ --------------*/

#ifndef HAS_WORD_64 //special case: 32-bit machine
int32_t xmem::randomRead_Word32(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
    volatile uintptr_t* p = first_address;

    UNROLL1024(p = reinterpret_cast<uintptr_t*>(*p);)
    *last_touched_address = const_cast<uintptr_t*>(p);
    return 0;
}
#endif

#ifdef HAS_WORD_64
int32_t xmem::randomRead_Word64(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
    volatile uintptr_t* p = first_address;

    UNROLL512(p = reinterpret_cast<uintptr_t*>(*p);)
    *last_touched_address = const_cast<uintptr_t*>(p);
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::randomRead_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    volatile Word128_t* p = reinterpret_cast<Word128_t*>(first_address);
    register Word128_t val;

#ifdef HAS_WORD_64
    UNROLL256(val = *p; p = reinterpret_cast<Word128_t*>(my_64b_extractLSB_128b(val));) //Do 128-bit load. Then extract 64 LSB to use as next load address.
#else //special case: 32-bit machine
    UNROLL256(val = *p; p = reinterpret_cast<Word128_t*>(my_32b_extractLSB_128b(val));) //Do 128-bit load. Then extract 32 LSB to use as next load address.
#endif

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word128_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::randomRead_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    volatile Word256_t* p = reinterpret_cast<Word256_t*>(first_address);
    register Word256_t val;

#ifdef HAS_WORD_64
    UNROLL128(val = *p; p = reinterpret_cast<Word256_t*>(my_64b_extractLSB_256b(val));) //Do 256-bit load. Then extract 64 LSB to use as next load address.
#else //special case: 32-bit machine
    UNROLL128(val = *p; p = reinterpret_cast<Word256_t*>(my_32b_extractLSB_256b(val));) //Do 256-bit load. Then extract 32 LSB to use as next load address.
#endif

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word256_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::randomRead_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::randomRead_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    #error 512-bit words are not supported on Windows.
#else
    //volatile Word512_t* p = reinterpret_cast<Word512_t*>(first_address);
    Word512_t* p = reinterpret_cast<Word512_t*>(first_address);
    register Word512_t val;

    //UNROLL64(val = *p; p = reinterpret_cast<Word512_t*>(my_64b_extractLSB_512b(val));) //Do 512-bit load. Then extract 64 LSB to use as next load address.
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    uint64_t tmp;
    UNROLL64(val = my_512b_load(p); my_64b_extractLSB_512b(tmp, scratchptr, val); p = reinterpret_cast<Word512_t*>(tmp);) //Do 512-bit load. Then extract 64 LSB to use as next load address.

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word512_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif

/* ------------ RANDOM WRITE --------------*/

#ifndef HAS_WORD_64 //special case: 32-bit machine
int32_t xmem::randomWrite_Word32(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
    volatile uintptr_t* p = first_address;
    volatile uintptr_t* p2 = NULL;

    UNROLL1024(p2 = reinterpret_cast<uintptr_t*>(*p); *p = reinterpret_cast<uintptr_t>(p2); p = p2;)
    *last_touched_address = const_cast<uintptr_t*>(p);
    return 0;
}
#endif

#ifdef HAS_WORD_64
int32_t xmem::randomWrite_Word64(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
    volatile uintptr_t* p = first_address;
    volatile uintptr_t* p2 = NULL;

    UNROLL512(p2 = reinterpret_cast<uintptr_t*>(*p); *p = reinterpret_cast<uintptr_t>(p2); p = p2;)
    *last_touched_address = const_cast<uintptr_t*>(p);
    return 0;
}
#endif

#ifdef HAS_WORD_128
int32_t xmem::randomWrite_Word128(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    volatile Word128_t* p = reinterpret_cast<Word128_t*>(first_address);
    register Word128_t val;

#ifdef HAS_WORD_64
    UNROLL256(val = *p; *p = val; p = reinterpret_cast<Word128_t*>(my_64b_extractLSB_128b(val));) //Do 128-bit load. Then do 128-bit store. Then extract 64 LSB to use as next load address.
#else //special case: 32-bit machine
    UNROLL256(val = *p; *p = val; p = reinterpret_cast<Word128_t*>(my_32b_extractLSB_128b(val));) //Do 128-bit load. Then do 128-bit store. Then extract 32 LSB to use as next load address.
#endif

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word128_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_256
int32_t xmem::randomWrite_Word256(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
    return 0; //TODO: Implement for Windows.
#else
    volatile Word256_t* p = reinterpret_cast<Word256_t*>(first_address);
    register Word256_t val;

#ifdef HAS_WORD_64
    UNROLL128(val = *p; *p = val; p = reinterpret_cast<Word256_t*>(my_64b_extractLSB_256b(val));) //Do 256-bit load. Then do 256-bit store. Then extract 64 LSB to use as next load address.
#else //special case: 32-bit machine
    UNROLL128(val = *p; *p = val; p = reinterpret_cast<Word256_t*>(my_32b_extractLSB_256b(val));) //Do 256-bit load. Then do 256-bit store. Then extract 32 LSB to use as next load address.
#endif

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word256_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif

#ifdef HAS_WORD_512
#ifdef _WIN32
int32_t xmem::randomWrite_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#else
//FIXME: We flag GCC/ICC not to optimize this function. The problem is that my_512b_load() -- which maps to _mm512_load_epi64 Intel AVX-512/KNC intrinsic -- cannot accept volatile* arguments. The only way to prevent the entire benchmark loop from being optimized away is to force the compiler not to optimize the function at all. Unfortunately, this will cause extra unwanted code to be included. The only solution seems to be to write inline assembly or allow eventual support for assignment of __m512i variables, e.g., val = *wordptr++; which is how the code for the other word sizes is written.
int32_t __attribute__((optimize("O0"))) xmem::randomWrite_Word512(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
#endif
#if defined(_WIN32) && defined(ARCH_INTEL_X86_64)
#error 512-bit words are not supported on Windows.
#else
    //volatile Word512_t* p = reinterpret_cast<Word512_t*>(first_address);
    Word512_t* p = reinterpret_cast<Word512_t*>(first_address);
    register Word512_t val;

    //UNROLL64(val = *p; *p = val; p = reinterpret_cast<Word512_t*>(my_64b_extractLSB_512b(val));) //Do 512-bit load. Then do 512-bit store. Then extract 64 LSB to use as next load address.
    uint64_t scratchptr[8] __attribute__ ((aligned(512)));
    uint64_t tmp;
    UNROLL64(val = my_512b_load(p); *p = val; my_64b_extractLSB_512b(tmp, scratchptr, val); p = reinterpret_cast<Word512_t*>(tmp);) //Do 512-bit load. Then do 512-bit store. Then extract 64 LSB to use as next load address.

    *last_touched_address = reinterpret_cast<uintptr_t*>(const_cast<Word512_t*>(p)); //Trick compiler. First get rid of volatile qualifier, and then reinterpret pointer
    return 0;
#endif
}
#endif
