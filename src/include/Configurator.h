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
#include <include/common.h>
#include <include/optionparser.h>
#include <include/MyArg.h>

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
			HELP,
			MEAS_LATENCY,
			MEAS_THROUGHPUT,
			WORKING_SET_SIZE,
			ITERATIONS,
			BASE_TEST_INDEX,
			OUTPUT_FILE
		};

		/**
		 * @brief Command-line option descriptors as needed by stuff in <config/third_party/optionparser.h>. This is basically the help message content.
		 */
		const third_party::Descriptor usage[] = {
			{ UNKNOWN, 0, "", "", third_party::Arg::None, "USAGE: xmem [options]\n\n"
			"Options:" },
			{ HELP, 0, "h", "help", third_party::Arg::None, "    -h, --help    \tPrint usage and exit." },
			{ MEAS_LATENCY, 0, "l", "latency", third_party::Arg::None, "    -l, --latency    \tMeasure memory latency" },
			{ MEAS_THROUGHPUT, 0, "t", "throughput", third_party::Arg::None, "    -t, --throughput    \tMeasure memory throughput" },
			{ WORKING_SET_SIZE, 0, "w", "working_set_size", third_party::MyArg::PositiveInteger, "    -w, --working_set_size    \tWorking set size in KB. This must be a multiple of 4KB." },
			{ ITERATIONS, 0, "n", "iterations", third_party::MyArg::PositiveInteger, "    -n, --iterations    \tIterations per benchmark test" },
			{ BASE_TEST_INDEX, 0, "i", "base_test_index", third_party::MyArg::NonnegativeInteger, "    -i, --base_test_index    \tNumerical index of the first benchmark, for tracking unique test IDs." },
			{ OUTPUT_FILE, 0, "f", "output_file", third_party::MyArg::Required, "    -f, --output_file    \tOutput filename to use. If not specified, no output file generated." },
			{ UNKNOWN, 0, "", "", third_party::Arg::None, "\nExamples:\n"
			"    xmem --help\n"
			"    xmem -h\n"
			"    xmem -t\n"
			"    xmem -t --latency -n10 -w524288 -f results.csv -i 101\n"
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
			 * @param working_set_size The total size of memory to test in all benchmarks, in bytes. This MUST be a multiple of 4KB pages.
			 * @param iterations_per_test For each unique benchmark test, this is the number of times to repeat it.
			 * @param filename Output filename to use.
			 * @param use_output_file If true, use the provided output filename.
			 */
			Configurator(bool runLatency, bool runThroughput, size_t working_set_size, uint32_t iterations_per_test, std::string filename, bool use_output_file);

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
			bool latencyTestSelected() { return __runLatency; }

			/**
			 * @brief Indicates if the throughput test has been selected.
			 * @returns True if the throughput test has been selected to run.
			 */
			bool throughputTestSelected() { return __runThroughput; }

			/**
			 * @brief Gets the working set size in bytes for each worker thread, if applicable.
			 * @returns The working set size in bytes.
			 */
			size_t getWorkingSetSize() { return __working_set_size; }

			/**
			 * @brief Gets the number of iterations that should be run of each benchmark.
			 * @returns The iterations for each test.
			 */
			uint32_t getIterationsPerTest() { return __iterations; }

			/**
			 * @brief Gets the output filename to use, if applicable.
			 * @returns The output filename to use if useOutputFile() returns true. Otherwise return value is "".
			 */
			std::string getOutputFilename() { return __filename; }

			/**
			 * @brief Determines whether to generate an output CSV file.
			 * @returns True if an output file should be used.
			 */
			bool useOutputFile() { return __use_output_file; }

		private:
			/**
			 * @brief Inspects a command line option (switch) to see if it occurred more than once, and warns the user if this is the case. The program only uses the first occurrence of any switch.
			 * @param opt The option to inspect.
			 * @returns True if the option only occurred once.
			 */
			bool __checkSingleOptionOccurrence(third_party::Option* opt);

			bool __runLatency; /**< True if latency tests should be run. */
			bool __runThroughput; /**< True if throughput tests should be run. */
			size_t __working_set_size; /**< Working set size in bytes for each thread, if applicable. */
			uint32_t __iterations; /**< Number of iterations to run for each benchmark test. */
			bool __use_output_file; /**< If true, generate a CSV output file for results. */
			std::string __filename; /**< The output filename if applicable. */
		};
	};
};

#endif
