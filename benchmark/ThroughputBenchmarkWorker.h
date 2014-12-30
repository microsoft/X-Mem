/**
 * @file
 * 
 * @brief Header file for the ThroughputBenchmarkWorker class.
 * 
 * (C) 2014 Microsoft Corporation
 */

#ifndef __THROUGHPUT_BENCHMARK_WORKER_H
#define __THROUGHPUT_BENCHMARK_WORKER_H

//Headers
#include <thread/Runnable.h>
#include <common/common.h>

//Libraries
#include <cstdint>

namespace xmem {
	namespace benchmark {
		/** 
		 * @brief Helper multithreading-friendly class to do the core throughput benchmark.
		 */
		class ThroughputBenchmarkWorker : public thread::Runnable {
			public:
				typedef int32_t(*BenchFunction)(void*, void*); //Core benchmark function pointer

				/** 
				 * @brief Constructor.
				 * @param mem_array Pointer to the memory region to use by this worker.
				 * @param len Length of the memory region to use by this worker.
				 * @param bench_fptr Pointer to the core benchmark kernel to use.
				 * @param dummy_fptr Pointer to the dummy version of the core benchmark kernel to use.
				 * @param cpu_affinity Logical CPU identifier to lock this worker's thread to.
				 */
				ThroughputBenchmarkWorker(
					void* mem_array,
					size_t len,
#ifdef USE_SIZE_BASED_BENCHMARKS
					uint64_t passes_per_iteration,
#endif
					BenchFunction bench_fptr,
					BenchFunction dummy_fptr,
					uint32_t cpu_affinity
					);
				
				/**
				 * @brief Destructor.
				 */
				~ThroughputBenchmarkWorker();

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
				uint32_t __cpu_affinity; /**< The logical CPU affinity for this worker. */
				BenchFunction __bench_fptr; /**< Points to the memory test core routine to use. */
				BenchFunction __dummy_fptr; /**< Points to a dummy version of the memory test core routine to use. */
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
};

#endif
