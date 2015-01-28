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
 * @brief Implementation file for benchmark kernel functions for doing the actual work we care about. :)
 *
 * Optimization tricks include:
 *   - UNROLL macros to manual loop unrolling. This reduces the relative branch overhead of the loop.
 *     We don't want to benchmark loops, we want to benchmark memory! But unrolling too much can hurt
 *	   code size and instruction locality, potentially decreasing I-cache utilization and causing extra overheads.
 *	   This is why we allow multiple unroll lengths at compile-time.
 *   - volatile keyword to prevent compiler from optimizing the code and removing instructions that we need.
 *     The compiler is too smart for its own good!
 */

//Headers
#include <benchmark_kernels.h>
#include <common.h>

#ifdef __gnu_linux__
#include <immintrin.h> //for Intel AVX intrinsics
#endif

using namespace xmem::common;
using namespace xmem::benchmark::benchmark_kernels;


/***********************************************************************
 ***********************************************************************
 ********************** LATENCY-RELATED BENCHMARK KERNELS **************
 ***********************************************************************
 ***********************************************************************/

/* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

int32_t xmem::benchmark::benchmark_kernels::dummy_chasePointers(uintptr_t*, uintptr_t**, size_t len) {
	volatile uintptr_t placeholder = 0; //Try to defeat compiler optimizations removing this method
#ifdef USE_SIZE_BASED_BENCHMARKS
	for (size_t i = 0; i < len / sizeof(uintptr_t); i += 512)
		placeholder = 0;
#endif
	return 0;
}

/* -------------------- CORE BENCHMARK ROUTINES -------------------------- */

int32_t xmem::benchmark::benchmark_kernels::chasePointers(uintptr_t* first_address, uintptr_t** last_touched_address, size_t len) {
	volatile uintptr_t* p = first_address;

#ifdef USE_TIME_BASED_BENCHMARKS
	UNROLL512(p = reinterpret_cast<uintptr_t*>(*p);)
#endif
#ifdef USE_SIZE_BASED_BENCHMARKS
	for (size_t i = 0; i < len / sizeof(uintptr_t); i += 512) {
		UNROLL512(p = reinterpret_cast<uintptr_t*>(*p);)
	}
#endif
	*last_touched_address = const_cast<uintptr_t*>(p);
	return 0;
}


/***********************************************************************
 ***********************************************************************
 ******************* THROUGHPUT-RELATED BENCHMARK KERNELS **************
 ***********************************************************************
 ***********************************************************************/

#ifdef _WIN32
//Hand-coded assembly functions for the SSE2/AVX benchmark routines.
//VC++ compiler does not support inline assembly in x86-64.
//And the compiler optimizes away the vector instructions unless I use volatile.
//But I can't use for example volatile Word256_t* because it is incompatible with _mm_load_si256() with VC++. 
//Fortunately, I implemented the routine as a wrapper around a hand-coded assembler C function.

//256-bit
extern "C" int win_asm_forwSequentialRead_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int win_asm_revSequentialRead_Word256(Word256_t* last_word, Word256_t* first_word);
extern "C" int win_asm_forwSequentialWrite_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int win_asm_revSequentialWrite_Word256(Word256_t* last_word, Word256_t* first_word);

//Dummies
//256-bit
extern "C" int win_asm_dummy_forwSequentialLoop_Word256(Word256_t* first_word, Word256_t* last_word);
extern "C" int win_asm_dummy_revSequentialLoop_Word256(Word256_t* first_word, Word256_t* last_word);
#endif

/* --------------------- DUMMY BENCHMARK ROUTINES --------------------------- */

int32_t xmem::benchmark::benchmark_kernels::dummy_empty(void*, void*) { 
	return 0;
}
		
/* ------------ SEQUENTIAL LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_forwSequentialLoop_Word32(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
		UNROLL1024(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwSequentialLoop_Word64(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL512(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwSequentialLoop_Word128(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
		UNROLL256(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwSequentialLoop_Word256(void* start_address, void* end_address) {
#ifdef _WIN32
	//FIXME: this probably does not need to be a dedicated asm function.
	return win_asm_dummy_forwSequentialLoop_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
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

int32_t xmem::benchmark::benchmark_kernels::dummy_revSequentialLoop_Word32(void* start_address, void* end_address) { 
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
		UNROLL1024(wordptr--;)
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revSequentialLoop_Word64(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
		UNROLL512(wordptr--;)
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revSequentialLoop_Word128(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
		UNROLL256(wordptr--;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revSequentialLoop_Word256(void* start_address, void* end_address) {
#ifdef _WIN32
	//FIXME: this probably does not need to be a dedicated asm function.
	return win_asm_dummy_revSequentialLoop_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
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
		
/* ------------ STRIDE 2 LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride2Loop_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
		UNROLL512(wordptr += 2;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride2Loop_Word64(void* start_address, void* end_address) {
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
		UNROLL256(wordptr += 2;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride2Loop_Word128(void* start_address, void* end_address) {
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
		UNROLL128(wordptr += 2;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride2Loop_Word256(void* start_address, void* end_address) {
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
		UNROLL64(wordptr += 2;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride2Loop_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
		UNROLL512(wordptr -= 2;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride2Loop_Word64(void* start_address, void* end_address) {
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
		UNROLL256(wordptr -= 2;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride2Loop_Word128(void* start_address, void* end_address) {
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
		UNROLL128(wordptr -= 2;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride2Loop_Word256(void* start_address, void* end_address) { 
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
		UNROLL64(wordptr -= 2;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

/* ------------ STRIDE 4 LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride4Loop_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
		UNROLL256(wordptr += 4;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride4Loop_Word64(void* start_address, void* end_address) {
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 128) {
		UNROLL128(wordptr += 4;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride4Loop_Word128(void* start_address, void* end_address) {
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 64) {
		UNROLL64(wordptr += 4;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride4Loop_Word256(void* start_address, void* end_address) {
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
		UNROLL32(wordptr += 4;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride4Loop_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
		UNROLL256(wordptr -= 4;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride4Loop_Word64(void* start_address, void* end_address) {
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
		UNROLL128(wordptr -= 4;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride4Loop_Word128(void* start_address, void* end_address) {
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
		UNROLL64(wordptr -= 4;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride4Loop_Word256(void* start_address, void* end_address) {
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
		UNROLL32(wordptr -= 4;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

/* ------------ STRIDE 8 LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride8Loop_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
		UNROLL128(wordptr += 8;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride8Loop_Word64(void* start_address, void* end_address) {
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
		UNROLL64(wordptr += 8;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride8Loop_Word128(void* start_address, void* end_address) {
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
		UNROLL32(wordptr += 8;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride8Loop_Word256(void* start_address, void* end_address) {
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
		UNROLL16(wordptr += 8;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride8Loop_Word32(void* start_address, void* end_address) { 
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
		UNROLL128(wordptr -= 8;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride8Loop_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
		UNROLL64(wordptr -= 8;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride8Loop_Word128(void* start_address, void* end_address) { 
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
		UNROLL32(wordptr -= 8;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride8Loop_Word256(void* start_address, void* end_address) { 
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
		UNROLL16(wordptr -= 8;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

/* ------------ STRIDE 16 LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride16Loop_Word32(void* start_address, void* end_address) { 
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
		UNROLL64(wordptr += 16;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride16Loop_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
		UNROLL32(wordptr += 16;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride16Loop_Word128(void* start_address, void* end_address) { 
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
		UNROLL16(wordptr += 16;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_forwStride16Loop_Word256(void* start_address, void* end_address) { 
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
		UNROLL8(wordptr += 16;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride16Loop_Word32(void* start_address, void* end_address) { 
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
		UNROLL64(wordptr -= 16;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride16Loop_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
		UNROLL32(wordptr -= 16;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride16Loop_Word128(void* start_address, void* end_address) { 
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
		UNROLL16(wordptr -= 16;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_revStride16Loop_Word256(void* start_address, void* end_address) { 
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
		UNROLL8(wordptr -= 16;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

/* ------------ RANDOM LOOP --------------*/

int32_t xmem::benchmark::benchmark_kernels::dummy_randomLoop_Word32(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_randomLoop_Word64(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_randomLoop_Word128(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::dummy_randomLoop_Word256(void* start_address, void* end_address) { 
	return 0;
}

/* -------------------- CORE BENCHMARK ROUTINES -------------------------- 
 *
 * These routines access the memory in different ways for each benchmark type.
 * Optimization tricks include:
 *   - register keyword to hint to compiler values that should not be used in memory access
 *   - UNROLL macros to manual loop unrolling. This reduces the relative branch overhead of the loop.
 *     We don't want to benchmark loops, we want to benchmark memory!
 *   - volatile keyword to prevent compiler from optimizing the code and removing instructions that we need.
 *     The compiler is too smart for its own good!
 *	 - Hardcoding stride and chunk sizes in each benchmark snippet, so that way they do not have to waste cycles being computed at runtime.
 *	   This makes code harder to maintain and to read but ensures more accurate memory benchmarking.
 *
 * ----------------------------------------------------------------------- */

/* ------------ SEQUENTIAL READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwSequentialRead_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
		UNROLL1024(val = *wordptr++;) 
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialRead_Word64(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL512(val = *wordptr++;)
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialRead_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
		UNROLL256(val = *wordptr++;)
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialRead_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return win_asm_forwSequentialRead_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL128(val = *wordptr++;)
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialRead_Word32(void* start_address, void* end_address) { 
	register Word32_t val;
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
		UNROLL1024(val = *wordptr--;)
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialRead_Word64(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
		UNROLL512(val = *wordptr--;)
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialRead_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
		UNROLL256(val = *wordptr--;)
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialRead_Word256(void* start_address, void* end_address) {
#ifdef _WIN32
	return win_asm_revSequentialRead_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address), *begptr = static_cast<Word256_t*>(start_address); wordptr > begptr;) {
		UNROLL128(val = *wordptr--;)
	}
	return 0;
#endif
}

/* ------------ SEQUENTIAL WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwSequentialWrite_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF; 
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address), *endptr = static_cast<Word32_t*>(end_address); wordptr < endptr;) {
		UNROLL1024(*wordptr++ = val;) 
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialWrite_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL512(*wordptr++ = val;) 
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialWrite_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address), *endptr = static_cast<Word128_t*>(end_address); wordptr < endptr;) {
		UNROLL256(*wordptr++ = val;) 
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwSequentialWrite_Word256(void* start_address, void* end_address) {
#ifdef _WIN32
	return win_asm_forwSequentialWrite_Word256(static_cast<Word256_t*>(start_address), static_cast<Word256_t*>(end_address));
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL128(*wordptr++ = val;) 
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialWrite_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF;
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address), *begptr = static_cast<Word32_t*>(start_address); wordptr > begptr;) {
		UNROLL1024(*wordptr-- = val;)
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialWrite_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address), *begptr = static_cast<Word64_t*>(start_address); wordptr > begptr;) {
		UNROLL512(*wordptr-- = val;)
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialWrite_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address), *begptr = static_cast<Word128_t*>(start_address); wordptr > begptr;) {
		UNROLL256(*wordptr-- = val;)
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revSequentialWrite_Word256(void* start_address, void* end_address) {
#ifdef _WIN32
	return win_asm_revSequentialWrite_Word256(static_cast<Word256_t*>(end_address), static_cast<Word256_t*>(start_address));
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address), *begptr = static_cast<Word256_t*>(start_address); wordptr > begptr;) {
		UNROLL128(*wordptr-- = val;)
	}
	return 0;
#endif
}

/* ------------ STRIDE 2 READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride2Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
		UNROLL512(val = *wordptr; wordptr += 2;) 
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
		UNROLL256(val = *wordptr; wordptr += 2;) 
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr += 2;) 
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr += 2;) 
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
		UNROLL512(val = *wordptr; wordptr -= 2;) 
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
		UNROLL256(val = *wordptr; wordptr -= 2;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr -= 2;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr -= 2;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 2 WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride2Write_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 512) {
		UNROLL512(*wordptr = val; wordptr += 2;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Write_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 256) {
		UNROLL256(*wordptr = val; wordptr += 2;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr += 2;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride2Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr += 2;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Write_Word32(void* start_address, void* end_address) { 
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 512) {
		UNROLL512(*wordptr = val; wordptr -= 2;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Write_Word64(void* start_address, void* end_address) { 
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 256) {
		UNROLL256(*wordptr = val; wordptr -= 2;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr -= 2;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride2Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr -= 2;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 4 READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride4Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
		UNROLL256(val = *wordptr; wordptr += 4;) 
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr += 4;) 
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr += 4;) 
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr += 4;) 
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
		UNROLL256(val = *wordptr; wordptr -= 4;) 
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr -= 4;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr -= 4;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr -= 4;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 4 WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride4Write_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 256) {
		UNROLL256(*wordptr = val; wordptr += 4;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Write_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr += 4;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr += 4;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride4Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr += 4;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Write_Word32(void* start_address, void* end_address) { 
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 256) {
		UNROLL256(*wordptr = val; wordptr -= 4;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Write_Word64(void* start_address, void* end_address) { 
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr -= 4;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr -= 4;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride4Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr -= 4;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 8 READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride8Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr += 8;) 
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr += 8;) 
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr += 8;) 
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
		UNROLL16(val = *wordptr; wordptr += 8;) 
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
		UNROLL128(val = *wordptr; wordptr -= 8;) 
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr -= 8;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr -= 8;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
		UNROLL16(val = *wordptr; wordptr -= 8;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 8 WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride8Write_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr += 8;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Write_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr += 8;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr += 8;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride8Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 16) {
		UNROLL16(*wordptr = val; wordptr += 8;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Write_Word32(void* start_address, void* end_address) { 
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 128) {
		UNROLL128(*wordptr = val; wordptr -= 8;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Write_Word64(void* start_address, void* end_address) { 
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr -= 8;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr -= 8;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride8Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 16) {
		UNROLL16(*wordptr = val; wordptr -= 8;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 16 READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride16Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr += 16;) 
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr += 16;) 
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
		UNROLL16(val = *wordptr; wordptr += 16;) 
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 8) {
		UNROLL8(val = *wordptr; wordptr += 16;) 
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Read_Word32(void* start_address, void* end_address) {
	register Word32_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
		UNROLL64(val = *wordptr; wordptr -= 16;) 
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Read_Word64(void* start_address, void* end_address) { 
	register Word64_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
		UNROLL32(val = *wordptr; wordptr -= 16;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Read_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
		UNROLL16(val = *wordptr; wordptr -= 16;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Read_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word256_t val; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
		UNROLL8(val = *wordptr; wordptr -= 16;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}
	return 0;
#endif
}

/* ------------ STRIDE 16 WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::forwStride16Write_Word32(void* start_address, void* end_address) {
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(start_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr += 16;)
		if (wordptr >= static_cast<Word32_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Write_Word64(void* start_address, void* end_address) {
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word64_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr += 16;)
		if (wordptr >= static_cast<Word64_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0;
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word128_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(start_address); i < len; i += 16) {
		UNROLL16(*wordptr = val; wordptr += 16;)
		if (wordptr >= static_cast<Word128_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::forwStride16Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0;
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address); i < len; i += 8) {
		UNROLL8(*wordptr = val; wordptr += 16;)
		if (wordptr >= static_cast<Word256_t*>(end_address)) //end, modulo
			wordptr -= len;
	}
	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Write_Word32(void* start_address, void* end_address) { 
	register Word32_t val = 0xFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word32_t* wordptr = static_cast<Word32_t*>(end_address); i < len; i += 64) {
		UNROLL64(*wordptr = val; wordptr -= 16;)
		if (wordptr <= static_cast<Word32_t*>(start_address)) //end, modulo
			wordptr += len;
	}

	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Write_Word64(void* start_address, void* end_address) { 
	register Word64_t val = 0xFFFFFFFFFFFFFFFF; 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(end_address); i < len; i += 32) {
		UNROLL32(*wordptr = val; wordptr -= 16;)
		if (wordptr <= static_cast<Word64_t*>(start_address)) //end, modulo
			wordptr += len;
	}

	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Write_Word128(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	register Word128_t val;
	val = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word32_t);
	for (volatile Word128_t* wordptr = static_cast<Word128_t*>(end_address); i < len; i += 16) {
		UNROLL16(*wordptr = val; wordptr -= 16;)
		if (wordptr <= static_cast<Word128_t*>(start_address)) //end, modulo
			wordptr += len;
	}

	return 0;
#endif
}

int32_t xmem::benchmark::benchmark_kernels::revStride16Write_Word256(void* start_address, void* end_address) { 
#ifdef _WIN32
	return 0; //TODO
#endif
#ifdef __gnu_linux__
	//FIXME: segfaults?
	register Word256_t val;
	val = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF); 
	register uint64_t i = 0;
	register uint64_t len = (reinterpret_cast<uint64_t>(end_address)-reinterpret_cast<uint64_t>(start_address)) / sizeof(Word256_t);
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(end_address); i < len; i += 8) {
		UNROLL8(*wordptr = val; wordptr -= 16;)
		if (wordptr <= static_cast<Word256_t*>(start_address)) //end, modulo
			wordptr += len;
	}

	return 0;
#endif
}

/* ------------ RANDOM READ --------------*/

int32_t xmem::benchmark::benchmark_kernels::randomRead_Word32(void* start_address, void* end_address) {
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomRead_Word64(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomRead_Word128(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomRead_Word256(void* start_address, void* end_address) { 
	return 0;
}

/* ------------ RANDOM WRITE --------------*/

int32_t xmem::benchmark::benchmark_kernels::randomWrite_Word32(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomWrite_Word64(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomWrite_Word128(void* start_address, void* end_address) { 
	return 0;
}

int32_t xmem::benchmark::benchmark_kernels::randomWrite_Word256(void* start_address, void* end_address) { 
	return 0;
}
