/**
 * @file
 * 
 * @brief Header file for the LatencyBenchmark class.
 * 
 * (C) 2014 Microsoft Corporation
 */

#ifndef __LATENCY_BENCHMARK_H
#define __LATENCY_BENCHMARK_H

//Headers
#include "Benchmark.h"
#include <common/common.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {
	namespace benchmark {
		/**
		 * @brief A type of benchmark that measures memory latency via random pointer chasing.
		 * TODO: loaded latency tests
		 */
		class LatencyBenchmark : public Benchmark {
		public:
			typedef int32_t(*LatencyBenchFunction)(uintptr_t*, uintptr_t**, size_t); //Core benchmark function pointer

			/**
			 * Constructor.
			 * @param mem_array a pointer to a contiguous chunk of memory that has been allocated for benchmarking among the worker threads. This should be aligned to a 256-bit boundary and should be the working set size times number of threads large.
			 * @param len Length of the raw_mem_array in bytes. This should be a multiple of 4 KB pages.
			 * @param iterations Number of iterations (passes) to do of the complete benchmark.
			 * @param cpu_node the logical CPU NUMA node to use in the benchmark
			 * @param mem_node the logical memory NUMA node used in the benchmark
			 * @param num_worker_threads number of worker threads to use in the benchmark
			 * @param name name of the benchmark to use when reporting
			 * @param timer pointer to an existing Timer object
			 * @param dram_power_readers vector of pointers to PowerReader objects for measuring DRAM power
			 */
			LatencyBenchmark(
				void* mem_array,
				size_t len,
				uint32_t iterations,
#ifdef USE_SIZE_BASED_BENCHMARKS
				uint64_t passes_per_iteration,
#endif
				uint32_t cpu_node,
				uint32_t mem_node,
				uint32_t num_worker_threads,
				std::string name,
				xmem::timers::Timer *timer,
				std::vector<xmem::power::PowerReader*> dram_power_readers
			);

			/**
			 * @brief Runs the benchmark.
			 * @returns True on success.
			 */
			virtual bool run();

			/**
			 * @brief Outputs the benchmark configuration to the console.
			 */
			virtual void report_benchmark_info();

			/**
			 * @brief Outputs a report of the benchmark results to the console if run() returned true.
			 */
			virtual void report_results();

		private:
			/**
			 * @brief Primes the whole memory space to be used in the benchmark.
			 * This is to warm up the caches and make sure the OS has physical pages allocated as well.
			 * @param passes The number of priming passes to do.
			 */
			void __primeMemory(uint64_t passes);

			/**
			 * @brief The core benchmark method.
			 * @returns True on success.
			 */
			bool __run_core();

			/**
			 * @brief Constructs a cycle of pointers in the whole memory space, such that each word will be reached exactly once. This is essentially a random linked list.
			 * For successive iterations, the access pattern is identical.
			 * @returns True on success.
			 */
			bool __buildRandomPointerPermutation();

			/**
			 * @brief Outputs a report of the benchmark results to the console if they ran.
			 */
			void __internal_report_results();

			//////////////////////// VARIABLES //////////////////////////////

			LatencyBenchFunction __bench_fptr; /**< Points to the memory test core routine to use. */
			LatencyBenchFunction __dummy_fptr; /**< Points to a dummy version of the memory test core routine to use. */
		};
	};
};

#endif
