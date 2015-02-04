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
 * @brief Implementation file for the Configurator class and some helper data structures.
 */

//Headers
#include <Configurator.h>
#include <common.h>
#include <optionparser.h>
#include <MyArg.h>
#include <common.h>

//Libraries
#include <cstdint>
#include <iostream>
#include <string>

using namespace xmem::config;

Configurator::Configurator(
	) :
	__configured(false),
	__runLatency(true),
	__runThroughput(true),
	__working_set_size_per_thread(DEFAULT_WORKING_SET_SIZE_PER_THREAD),
	__num_worker_threads(DEFAULT_NUM_WORKER_THREADS),
	__use_chunk_32b(false),
	__use_chunk_64b(true),
	__use_chunk_128b(false),
	__use_chunk_256b(false),
	__numa_enabled(true),
	__iterations(1),
	__use_random_access_pattern(false),
	__use_sequential_access_pattern(true),
	__starting_test_index(1),
	__filename(),
	__use_output_file(false),
	__verbose(false),
	__use_reads(true),
	__use_writes(true)
	{
}

Configurator::Configurator(
	bool runLatency,
	bool runThroughput,
	size_t working_set_size_per_thread,
	uint32_t num_worker_threads,
	bool use_chunk_32b,
	bool use_chunk_64b,
	bool use_chunk_128b,
	bool use_chunk_256b,
	bool numa_enabled,
	uint32_t iterations_per_test,
	bool use_random_access_pattern,
	bool use_sequential_access_pattern,
	uint32_t starting_test_index,
	std::string filename,
	bool use_output_file,
	bool verbose,
	bool use_reads,
	bool use_writes
	) :
	__configured(true),
	__runLatency(runLatency),
	__runThroughput(runThroughput),
	__working_set_size_per_thread(working_set_size_per_thread),
	__num_worker_threads(num_worker_threads),
	__use_chunk_32b(use_chunk_32b),
	__use_chunk_64b(use_chunk_64b),
	__use_chunk_128b(use_chunk_128b),
	__use_chunk_256b(use_chunk_256b),
	__numa_enabled(numa_enabled),
	__iterations(iterations_per_test),
	__use_random_access_pattern(use_random_access_pattern),
	__use_sequential_access_pattern(use_sequential_access_pattern),
	__starting_test_index(starting_test_index),
	__filename(filename),
	__use_output_file(use_output_file),
	__verbose(verbose),
	__use_reads(use_reads),
	__use_writes(use_writes)
	{
}

int32_t Configurator::configureFromInput(int argc, char* argv[]) {
	if (__configured) { //If this object was already configured, cannot override from user inputs. This is to prevent an invalid state.
		std::cerr << "WARNING: Something bad happened when configuring X-Mem. This is probably not your fault." << std::endl;
		return -2;
	}

	//Throw out first argument which is usually the program name.
	argc -= (argc > 0);
	argv += (argc > 0);

	//Set up optionparser
	third_party::Stats stats(usage, argc, argv);
	third_party::Option* options = new third_party::Option[stats.options_max];
	third_party::Option* buffer = new third_party::Option[stats.buffer_max];
	third_party::Parser parse(usage, argc, argv, options, buffer); //Parse input

	//Check for parser error
	if (parse.error())
		goto error;

	//X-Mem doesn't have any non-option arguments, so we will presume the user wants a help message.
	if (parse.nonOptionsCount() > 0) {
		std::cerr << "ERROR: X-Mem does not support any non-option arguments." << std::endl;
		goto error;
	}

	//Check for any unknown options
	for (third_party::Option* unknown_opt = options[UNKNOWN]; unknown_opt != NULL; unknown_opt = unknown_opt->next()) {
		std::cerr << "ERROR: Unknown option: " << std::string(unknown_opt->name, unknown_opt->namelen) << std::endl;
		goto error;
	}

	//Check runtime modes
	if (options[MEAS_LATENCY] || options[MEAS_THROUGHPUT]) { //User explicitly picked a mode, so override default selection
		__runLatency = false;
		__runThroughput = false;
	}

	if (options[MEAS_LATENCY])
		__runLatency = true;

	if (options[MEAS_THROUGHPUT])
		__runThroughput = true;

	//Check working set size
	if (options[WORKING_SET_SIZE_PER_THREAD]) { //Override default value with user-specified value
		if (!__checkSingleOptionOccurrence(&options[WORKING_SET_SIZE_PER_THREAD]))
			goto error;

		char* endptr = NULL;
		size_t working_set_size_KB = strtoul(options[WORKING_SET_SIZE_PER_THREAD].arg, &endptr, 10);	
		if ((working_set_size_KB % 4) != 0) {
			std::cerr << "ERROR: Working set size must be specified in KB and be a multiple of 4 KB." << std::endl; 
			goto error;
		}

		__working_set_size_per_thread = working_set_size_KB * KB; //convert to bytes
	}

#ifdef USE_LARGE_PAGES
	size_t num_large_pages = 0;
	if (__working_set_size_per_thread <= xmem::common::g_large_page_size) //sub one large page, round up to one
		num_large_pages = 1;
	else if (__working_set_size_per_thread % xmem::common::g_large_page_size == 0) //multiple of large page
		num_large_pages = __working_set_size_per_thread / xmem::common::g_large_page_size;
	else //larger than one large page but not a multiple of large page
		num_large_pages = __working_set_size_per_thread / xmem::common::g_large_page_size + 1;
#endif

	//Check number of worker threads
	if (options[NUM_WORKER_THREADS]) { //Override default value
		if (!__checkSingleOptionOccurrence(&options[NUM_WORKER_THREADS]))
			goto error;

		char* endptr = NULL;
		__num_worker_threads = static_cast<uint32_t>(strtoul(options[NUM_WORKER_THREADS].arg, &endptr, 10));
	}
	
	//Check chunk sizes
	if (options[CHUNK_SIZE]) {
		//Init... override default values
		__use_chunk_32b = false;
		__use_chunk_64b = false;
		__use_chunk_128b = false;
		__use_chunk_256b = false;

		third_party::Option* curr = options[CHUNK_SIZE];
		while (curr) { //CHUNK_SIZE may occur more than once, this is perfectly OK.
			char* endptr = NULL;
			uint32_t chunk_size = static_cast<uint32_t>(strtoul(curr->arg, &endptr, 10));
			switch (chunk_size) {
				case 32:
					__use_chunk_32b = true;
					break;
				case 64:
					__use_chunk_64b = true;
					break;
				case 128: 
					__use_chunk_128b = true;
					break;
				case 256:
					__use_chunk_256b = true;
					break;
				default:
					std::cerr << "ERROR: Invalid chunk size " << chunk_size << ". Chunk sizes can be 32, 64, 128, or 256 bits." << std::endl;
					goto error;
			}
			curr = curr->next();
		}
	}

	//Check NUMA selection
	if (options[NUMA_DISABLE])
		__numa_enabled = false;

	if (options[VERBOSE]) {
		__verbose = true; //What the user configuration is.
		common::g_verbose = true; //What rest of X-Mem actually uses.
	}

	//Check iterations
	if (options[ITERATIONS]) { //Override default value
		if (!__checkSingleOptionOccurrence(&options[ITERATIONS]))
			goto error;

		char *endptr = NULL;
		__iterations = static_cast<uint32_t>(strtoul(options[ITERATIONS].arg, &endptr, 10));
	}

	//Check throughput benchmark access patterns
	if (options[RANDOM_ACCESS_PATTERN] || options[SEQUENTIAL_ACCESS_PATTERN]) { //override defaults
		if (!__runThroughput) //These options only make sense for throughput benchmarks, but are otherwise harmless
			std::cerr << "WARNING: Ignoring specified access patterns. These only apply to throughput benchmarks." << std::endl;

		__use_random_access_pattern = false;
		__use_sequential_access_pattern = false;
	}

	if (options[RANDOM_ACCESS_PATTERN]) {
		__use_random_access_pattern = true;
	}
	
	if (options[SEQUENTIAL_ACCESS_PATTERN]) {
		__use_sequential_access_pattern = true;
	}

	//Check starting test index
	if (options[BASE_TEST_INDEX]) { //override defaults
		if (!__checkSingleOptionOccurrence(&options[BASE_TEST_INDEX]))
			goto error;

		char *endptr = NULL;
		__starting_test_index = static_cast<uint32_t>(strtoul(options[BASE_TEST_INDEX].arg, &endptr, 10)); //What the user specified
	}
	xmem::common::g_starting_test_index = __starting_test_index; //What rest of X-Mem uses
	xmem::common::g_test_index = xmem::common::g_starting_test_index; //What rest of X-Mem uses. The current test index.

	//Check filename
	if (options[OUTPUT_FILE]) { //override defaults
		if (!__checkSingleOptionOccurrence(&options[OUTPUT_FILE]))
			goto error;

		__filename = options[OUTPUT_FILE].arg;
		__use_output_file = true;
	}

	//Check if reads should be used in throughput benchmarks
	if (options[USE_READS] || options[USE_WRITES]) { //override defaults
		if (!__runThroughput) //These options only make sense for throughput benchmarks, but are otherwise harmless
			std::cerr << "WARNING: Ignoring specified read/write patterns. These only apply to throughput benchmarks." << std::endl;

		__use_reads = false;
		__use_writes = false;
	}

	if (options[USE_READS])
		__use_reads = true;

	if (options[USE_WRITES])
		__use_writes = true;
	
	//Make sure at least one mode is available
	if (!__runLatency && !__runThroughput) {
		std::cerr << "ERROR: At least one benchmark type must be selected." << std::endl;
		goto error;
	}

	//Make sure at least one access pattern is selected if in throughput mode
	if (__runThroughput && !__use_random_access_pattern && !__use_sequential_access_pattern) { //This should never be triggered
		std::cerr << "ERROR: Throughput benchmark was selected, but no access pattern was specified!" << std::endl;	
		goto error;
	}
	
	//Make sure at least one access pattern is selected if in throughput mode
	if (__runThroughput && !__use_reads && !__use_writes) { //This should never be triggered
		std::cerr << "ERROR: Throughput benchmark was selected, but no read/write pattern was specified!" << std::endl;	
		goto error;
	}

	//Check for help or bad options
	if (options[HELP] || options[UNKNOWN] != NULL)
		goto error;
	
	//Echo user settings
	std::cout << std::endl;
	if (__verbose) 
		std::cout << "Verbose mode enabled." << std::endl;
	if (__runLatency)
		std::cout << "Latency test selected." << std::endl;
	if (__runThroughput) {
		std::cout << "Throughput test selected." << std::endl;
		std::cout << "---> Random access: \t\t";
		if (__use_random_access_pattern)
			std::cout << "yes";
		else
			std::cout << "no";
		std::cout << std::endl;
		std::cout << "---> Sequential access: \t";
		if (__use_sequential_access_pattern)
			std::cout << "yes";
		else
			std::cout << "no";
		std::cout << std::endl;
		std::cout << "---> Use memory reads: \t\t";
		if (__use_reads)
			std::cout << "yes";
		else
			std::cout << "no";
		std::cout << std::endl;
		std::cout << "---> Use memory writes: \t";
		if (__use_writes)
			std::cout << "yes";
		else
			std::cout << "no";
		std::cout << std::endl;
	}
	std::cout << "Working set:  \t\t\t";
#ifndef USE_LARGE_PAGES
	std::cout << __working_set_size_per_thread << " B == " << __working_set_size_per_thread / KB  << " KB == " << __working_set_size_per_thread / MB << " MB (" << __working_set_size_per_thread/(xmem::common::g_page_size) << " pages)" << std::endl;	
#else
	std::cout << __working_set_size_per_thread << " B == " << __working_set_size_per_thread / KB  << " KB == " << __working_set_size_per_thread / MB << " MB (fits in " << num_large_pages << " large pages)" << std::endl;	
#endif
	std::cout << "Number of worker threads:  \t";
	std::cout << __num_worker_threads << std::endl;
	std::cout << "Chunk sizes:  \t\t\t";
	if (__use_chunk_32b)
		std::cout << "32 ";
	if (__use_chunk_64b)
		std::cout << "64 ";
	if (__use_chunk_128b)
		std::cout << "128 ";
	if (__use_chunk_256b)
		std::cout << "256 ";
	std::cout << std::endl;
	if (!__numa_enabled)
		std::cout << "NUMA enabled:   \t\tno" << std::endl;
	else
		std::cout << "NUMA enabled:   \t\tyes" << std::endl;
	std::cout << "Iterations:  \t\t\t";
	std::cout << __iterations << std::endl;
	std::cout << "Starting test index:  \t\t";
	std::cout << __starting_test_index << std::endl;

	//Free up options memory
	if (options)
		delete[] options;
	if (buffer)
		delete[] buffer;

	return 0;

	error:
		third_party::printUsage(std::cerr, usage); //Display help message

		//Free up options memory
		if (options)
			delete[] options;
		if (buffer)
			delete[] buffer;

		return -1;
}

bool Configurator::__checkSingleOptionOccurrence(third_party::Option* opt) const {
	if (opt->count() > 1) {
		std::cerr << "ERROR: " << opt->name << " option can only be specified once." << std::endl;
		return false;
	}
	return true;
}
