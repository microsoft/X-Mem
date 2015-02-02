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

Configurator::Configurator() :
					__runLatency(false),
					__runThroughput(false),
					__working_set_size_per_thread(DEFAULT_WORKING_SET_SIZE_PER_THREAD),
					__num_worker_threads(DEFAULT_NUM_WORKER_THREADS),
					__use_chunk_32b(false),
					__use_chunk_64b(false),
					__use_chunk_128b(false),
					__use_chunk_256b(false),
					__numa_enabled(true),
					__iterations(1),
					__filename(),
					__use_output_file(false)
					{
}

Configurator::Configurator(bool runLatency, bool runThroughput, size_t working_set_size_per_thread, uint32_t num_worker_threads, bool use_chunk_32b, bool use_chunk_64b, bool use_chunk_128b, bool use_chunk_256b, bool numa_enabled, uint32_t iterations_per_test, std::string filename, bool use_output_file) :
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
					__filename(filename),
					__use_output_file(use_output_file)
					{
}

int Configurator::configureFromInput(int argc, char* argv[]) {
	//Throw out first argument which is usually the program name.
	argc -= (argc > 0);
	argv += (argc > 0);

	//Set up optionparser
	third_party::Stats stats(usage, argc, argv);
	third_party::Option* options = new third_party::Option[stats.options_max];
	third_party::Option* buffer = new third_party::Option[stats.buffer_max];
	third_party::Parser parse(usage, argc, argv, options, buffer); //Parse input

	//Need some arguments.
	if (argc == 0)
		goto error;

	//Check for parser error
	if (parse.error())
		goto error;

	//Check for any unknown options
	for (third_party::Option* unknown_opt = options[UNKNOWN]; unknown_opt != NULL; unknown_opt = unknown_opt->next()) {
		std::cerr << "ERROR: Unknown option: " << std::string(unknown_opt->name, unknown_opt->namelen) << std::endl;
		goto error;
	}

	//Check runtime modes
	if (options[MEAS_LATENCY])
		__runLatency = true;

	if (options[MEAS_THROUGHPUT])
		__runThroughput = true;

	//Check working set size
	if (options[WORKING_SET_SIZE_PER_THREAD]) {
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
	if (options[NUM_WORKER_THREADS]) {
		if (!__checkSingleOptionOccurrence(&options[NUM_WORKER_THREADS]))
			goto error;

		char* endptr = NULL;
		__num_worker_threads = static_cast<uint32_t>(strtoul(options[NUM_WORKER_THREADS].arg, &endptr, 10));
	}
	
	//Check chunk sizes
	if (options[CHUNK_SIZE]) {
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
	} else
		__use_chunk_64b = true;


	//Check NUMA selection
	if (options[NUMA_DISABLE])
		__numa_enabled = false;

	//Check iterations
	if (options[ITERATIONS]) {
		if (!__checkSingleOptionOccurrence(&options[ITERATIONS]))
			goto error;

		char *endptr = NULL;
		__iterations = static_cast<uint32_t>(strtoul(options[ITERATIONS].arg, &endptr, 10));
	}

	//Check starting test index
	if (options[BASE_TEST_INDEX]) {
		if (!__checkSingleOptionOccurrence(&options[BASE_TEST_INDEX]))
			goto error;

		char *endptr = NULL;
		xmem::common::g_starting_test_index = static_cast<uint32_t>(strtoul(options[BASE_TEST_INDEX].arg, &endptr, 10));
	}
	else
		xmem::common::g_starting_test_index = 1;
	xmem::common::g_test_index = xmem::common::g_starting_test_index;

	//Check filename
	if (options[OUTPUT_FILE]) {
		if (!__checkSingleOptionOccurrence(&options[OUTPUT_FILE]))
			goto error;

		__filename = options[OUTPUT_FILE].arg;
		__use_output_file = true;
	}
	
	//Make sure at least one mode is available
	if (!__runLatency && !__runThroughput) {
		std::cerr << "ERROR: At least one benchmark type must be selected." << std::endl;
		goto error;
	}
	
	if (options[HELP] || options[UNKNOWN] != NULL)
		goto error;

	//Echo user settings if we are verbose
#ifdef VERBOSE
	if (__runLatency)
		std::cout << "Latency test selected." << std::endl;
	if (__runThroughput)
		std::cout << "Latency test selected." << std::endl;
	std::cout << std::endl << "Working set:  \t\t\t";
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
#endif

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
