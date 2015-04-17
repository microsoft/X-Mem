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
 * @brief Implementation file for benchmark kernel functions for the delay-injected loaded latency benchmark.
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
#include <delay_injected_benchmark_kernels.h>
#include <common.h>

//Libraries
#include <iostream>

using namespace xmem;

/***********************************************************************
 ***********************************************************************
 ******************* THROUGHPUT-RELATED BENCHMARK KERNELS **************
 ***********************************************************************
 ***********************************************************************/

/* -------------------- DUMMY BENCHMARK ROUTINES ------------------------- */

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay1(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL256(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay2(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL128(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay4(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL64(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay8(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL32(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay16(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL16(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay32(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL8(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay64(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL4(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay128(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL2(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word64_Delay256plus(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		wordptr++;
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay1(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL64(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay2(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL32(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay4(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL16(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay8(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL8(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay16(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL4(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay32(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL2(wordptr++;) 
		placeholder = 0;
	}
	return placeholder;
}

int32_t xmem::dummy_forwSequentialLoop_Word256_Delay64plus(void* start_address, void* end_address) {
	volatile int32_t placeholder = 0; //Try our best to defeat compiler optimizations
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		wordptr++;
		placeholder = 0;
	}
	return placeholder;
}

/* -------------------- CORE BENCHMARK ROUTINES -------------------------- */

int32_t xmem::forwSequentialRead_Word64_Delay1(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL256(val = *wordptr++; my_nop();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay2(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL128(val = *wordptr++; my_nop2();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay4(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL64(val = *wordptr++; my_nop4();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay8(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL32(val = *wordptr++; my_nop8();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay16(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL16(val = *wordptr++; my_nop16();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay32(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL8(val = *wordptr++; my_nop32();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay64(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL4(val = *wordptr++; my_nop64();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay128(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		UNROLL2(val = *wordptr++; my_nop128();)
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay256(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop256();
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay512(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop512();
	}
	return 0;
}

int32_t xmem::forwSequentialRead_Word64_Delay1024(void* start_address, void* end_address) {
	register Word64_t val;
	for (volatile Word64_t* wordptr = static_cast<Word64_t*>(start_address), *endptr = static_cast<Word64_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop1024();
	}
	return 0;
}




int32_t xmem::forwSequentialRead_Word256_Delay1(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL64(val = *wordptr++; my_nop();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay2(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL32(val = *wordptr++; my_nop2();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay4(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL16(val = *wordptr++; my_nop4();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay8(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL8(val = *wordptr++; my_nop8();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay16(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL4(val = *wordptr++; my_nop16();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay32(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		UNROLL2(val = *wordptr++; my_nop32();)
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay64(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop64();
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay128(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop128();
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay256(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop256();
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay512(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop512();
	}
	return 0;
#endif
}

int32_t xmem::forwSequentialRead_Word256_Delay1024(void* start_address, void* end_address) {
#ifdef _WIN32
	return 0; //TODO: Not yet implemented for Windows.
#endif
#ifdef __gnu_linux__
	register Word256_t val;
	for (volatile Word256_t* wordptr = static_cast<Word256_t*>(start_address), *endptr = static_cast<Word256_t*>(end_address); wordptr < endptr;) {
		val = *wordptr++; my_nop1024();
	}
	return 0;
#endif
}
