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
 * @brief Header file for the Configurator class and some helper data structures.
 */

#ifndef __CONFIGURATOR_H
#define __CONFIGURATOR_H

//Headers
#include <common.h>
#include <optionparser.h>
#include <MyArg.h>

//Libraries
#include <cstdint>
#include <string>

namespace xmem {
	namespace config {
		/**
		 * @brief Enumerates all possible types of command-line options
		 */
		enum optionIndex {
			UNKNOWN,
			CHUNK_SIZE,
			OUTPUT_FILE,
			HELP,
			BASE_TEST_INDEX,
			NUM_WORKER_THREADS,
			MEAS_LATENCY,
			ITERATIONS,
			MEAS_THROUGHPUT,
			NUMA_DISABLE,
			WORKING_SET_SIZE_PER_THREAD
		};

		/**
		 * @brief Command-line option descriptors as needed by stuff in <config/third_party/optionparser.h>. This is basically the help message content.
		 */
		const third_party::Descriptor usage[] = {
			{ UNKNOWN, 0, "", "", third_party::Arg::None, "\nUSAGE: xmem-<OS> [options]\n\n"
			"Options:" },
			{ CHUNK_SIZE, 0, "c", "chunk_size", third_party::MyArg::PositiveInteger, "    -c, --chunk_size    \tA chunk size to use for throughput benchmarks, specified in bits. Allowed values: 32, 64, 128, and 256. If no chunk sizes specified, use 64-bit chunks by default. NOTE: Some chunk sizes may not be supported on your hardware."},
			{ OUTPUT_FILE, 0, "f", "output_file", third_party::MyArg::Required, "    -f, --output_file    \tOutput filename to use. If not specified, no output file generated." },
			{ HELP, 0, "h", "help", third_party::Arg::None, "    -h, --help    \tPrint usage and exit." },
			{ BASE_TEST_INDEX, 0, "i", "base_test_index", third_party::MyArg::NonnegativeInteger, "    -i, --base_test_index    \tNumerical index of the first benchmark, for tracking unique test IDs." },
			{ NUM_WORKER_THREADS, 1, "j", "num_worker_threads", third_party::MyArg::PositiveInteger, "    -j, --num_worker_threads    \tNumber of worker threads to use in benchmarks." },
			{ MEAS_LATENCY, 0, "l", "latency", third_party::Arg::None, "    -l, --latency    \tMeasure memory latency" },
			{ ITERATIONS, 0, "n", "iterations", third_party::MyArg::PositiveInteger, "    -n, --iterations    \tIterations per benchmark test" },
			{ MEAS_THROUGHPUT, 0, "t", "throughput", third_party::Arg::None, "    -t, --throughput    \tMeasure memory throughput" },
			{ NUMA_DISABLE, 1, "u", "force_uma", third_party::Arg::None, "    -u, --force_uma    \tTest only CPU/memory NUMA node 0 instead of all combinations." },
			{ WORKING_SET_SIZE_PER_THREAD, 0, "w", "working_set_size_per_thread", third_party::MyArg::PositiveInteger, "    -w, --working_set_size    \tWorking set size per thread in KB. This must be a multiple of 4KB." },
			{ UNKNOWN, 0, "", "", third_party::Arg::None, "\nExamples:\n"
			"    xmem --help\n"
			"    xmem -h\n"
			"    xmem -t\n"
			"    xmem -t --latency -n10 -w524288 -f results.csv -c32 -c256 -i 101 -u -j2\n"
			},
			{ 0, 0, 0, 0, 0, 0 }
		};

		/**
		 * @brief Handles all user input interpretation and generates the necessary flags for running benchmarks.
		 */
		class Configurator {
		public:
			/**
			 * @brief Default constructor. A default configuration is set. You will want to run configureFromInput() most likely.
			 */
			Configurator();

			/**
			 * @brief Specialized constructor for when you don't want to get config from input, and you want to pass it in directly.
			 * @param runLatency Indicates latency benchmarks should be run.
			 * @param runThroughput Indicates throughput benchmarks should be run.
			 * @param working_set_size_per_thread The total size of memory to test in all benchmarks, in bytes, per thread. This MUST be a multiple of 4KB pages.
			 * @param num_worker_threads The number of threads to use in throughput benchmarks, loaded latency benchmarks, and stress tests.
			 * @param use_chunk_32b If true, include 32-bit chunks for relevant benchmarks.
			 * @param use_chunk_64b If true, include 64-bit chunks for relevant benchmarks.
			 * @param use_chunk_128b If true, include 128-bit chunks for relevant benchmarks.
			 * @param use_chunk_256b If true, include 256-bit chunks for relevant benchmarks.
			 * @param numa_enable If true, then test all combinations of CPU/memory NUMA nodes.
			 * @param iterations_per_test For each unique benchmark test, this is the number of times to repeat it.
			 * @param filename Output filename to use.
			 * @param use_output_file If true, use the provided output filename.
			 */
			Configurator(bool runLatency, bool runThroughput, size_t working_set_size_per_thread, uint32_t num_worker_threads, bool use_chunk_32b, bool use_chunk_64b, bool use_chunk_128b, bool use_chunk_256b, bool numa_enable, uint32_t iterations_per_test, std::string filename, bool use_output_file);

			/**
			 * @brief Configures the tool based on user's command-line inputs.
			 * @param argc The argc from main().
			 * @param argv The argv from main().
			 * @returns 0 on success.
			 */
			int configureFromInput(int argc, char* argv[]);

			/**
			 * @brief Indicates if the latency test has been selected.
			 * @returns True if the latency test has been selected to run.
			 */
			bool latencyTestSelected() const { return __runLatency; }

			/**
			 * @brief Indicates if the throughput test has been selected.
			 * @returns True if the throughput test has been selected to run.
			 */
			bool throughputTestSelected() const { return __runThroughput; }

			/**
			 * @brief Gets the working set size in bytes for each worker thread, if applicable.
			 * @returns The working set size in bytes.
			 */
			size_t getWorkingSetSizePerThread() const { return __working_set_size_per_thread; }

			/**
			 * @brief Determines if chunk size of 32 bits should be used in relevant benchmarks.
			 * @returns True if 32-bit chunks should be used.
			 */
			bool useChunk32b() const { return __use_chunk_32b; }

			/**
			 * @brief Determines if chunk size of 64 bits should be used in relevant benchmarks.
			 * @returns True if 64-bit chunks should be used.
			 */
			bool useChunk64b() const { return __use_chunk_64b; }

			/**
			 * @brief Determines if chunk size of 128 bits should be used in relevant benchmarks.
			 * @returns True if 128-bit chunks should be used.
			 */
			bool useChunk128b() const { return __use_chunk_128b; }

			/**
			 * @brief Determines if chunk size of 256 bits should be used in relevant benchmarks.
			 * @returns True if 256-bit chunks should be used.
			 */
			bool useChunk256b() const { return __use_chunk_256b; }
	
			/**
			 * @brief Determines if the benchmarks should test for all CPU/memory NUMA combinations.
			 * @returns True if all NUMA nodes should be tested.
			 */
			bool isNUMAEnabled() const { return __numa_enabled; }

			/**
			 * @brief Gets the number of iterations that should be run of each benchmark.
			 * @returns The iterations for each test.
			 */
			uint32_t getIterationsPerTest() const { return __iterations; }

			/** 
			 * @brief Gets the number of worker threads to use.
			 * @returns The number of worker threads.
			 */
			uint32_t getNumWorkerThreads() const { return __num_worker_threads; }

			/**
			 * @brief Gets the output filename to use, if applicable.
			 * @returns The output filename to use if useOutputFile() returns true. Otherwise return value is "".
			 */
			std::string getOutputFilename() const { return __filename; }

			/**
			 * @brief Determines whether to generate an output CSV file.
			 * @returns True if an output file should be used.
			 */
			bool useOutputFile() const { return __use_output_file; }

		private:
			/**
			 * @brief Inspects a command line option (switch) to see if it occurred more than once, and warns the user if this is the case. The program only uses the first occurrence of any switch.
			 * @param opt The option to inspect.
			 * @returns True if the option only occurred once.
			 */
			bool __checkSingleOptionOccurrence(third_party::Option* opt) const;

			bool __runLatency; /**< True if latency tests should be run. */
			bool __runThroughput; /**< True if throughput tests should be run. */
			size_t __working_set_size_per_thread; /**< Working set size in bytes for each thread, if applicable. */
			uint32_t __num_worker_threads; /**< Number of load threads to use for throughput benchmarks, loaded latency benchmarks, and stress tests. */
			bool __use_chunk_32b; /**< If true, use chunk sizes of 32-bits where applicable. */
			bool __use_chunk_64b; /**< If true, use chunk sizes of 64-bits where applicable. */
			bool __use_chunk_128b; /**< If true, use chunk sizes of 128-bits where applicable. */
			bool __use_chunk_256b; /**< If true, use chunk sizes of 256-bits where applicable. */
			bool __numa_enabled; /**< If true, test all combinations of CPU/memory NUMA nodes. Otherwise, just use node 0. */
			uint32_t __iterations; /**< Number of iterations to run for each benchmark test. */
			std::string __filename; /**< The output filename if applicable. */
			bool __use_output_file; /**< If true, generate a CSV output file for results. */
		};
	};
};

#endif
