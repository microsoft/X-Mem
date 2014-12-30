/**
 * @file
 * 
 * @brief Implementation file for the Configurator class and some helper data structures.
 * 
 * (C) 2014 Microsoft Corporation
 */

//Headers
#include "Configurator.h"
#include <common/common.h>
#include "third_party/optionparser.h"
#include "third_party/MyArg.h"

//Libraries
#include <cstdint>
#include <iostream>
#include <string>

using namespace xmem::config;

Configurator::Configurator() :
					__runLatency(false),
					__runThroughput(false),
					__working_set_size(DEFAULT_WORKING_SET_SIZE),
					__iterations(1),
					__filename(),
					__use_output_file(false)
					{
}

Configurator::Configurator(bool runLatency, bool runThroughput, size_t working_set_size, uint32_t iterations_per_test, std::string filename, bool use_output_file) :
					__runLatency(runLatency),
					__runThroughput(runThroughput),
					__working_set_size(working_set_size),
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

	//Check for parser error
	if (parse.error()) {
		//Free up options memory
		delete[] options;
		delete[] buffer;
	
		return -1;
	}

	//Report any unknown options
	for (third_party::Option* unknown_opt = options[UNKNOWN]; unknown_opt != NULL; unknown_opt = unknown_opt->next())
		std::cerr << "Unknown option: " << std::string(unknown_opt->name, unknown_opt->namelen) << std::endl;

	std::cout << std::endl;

	//Help message condition and exit (help option specified, no arguments, or one or more unknown options)
	if (options[HELP] || argc == 0 || options[UNKNOWN] != NULL) {
		third_party::printUsage(std::cerr, usage);

		//Free up options memory
		delete[] options;
		delete[] buffer;
	
		return -1;
	}

	//Check which options user provided, if any, and override mode defaults as necessary

	//Check runtime mode
	if (options[MEAS_LATENCY]) {
		__runLatency = true;
#ifdef VERBOSE
		std::cout << "Latency test selected." << std::endl;
#endif
	}
	if (options[MEAS_THROUGHPUT]) {
		__runThroughput = true;
#ifdef VERBOSE
		std::cout << "Throughput test selected." << std::endl;
#endif
	}
	//Make sure at least one mode is available
	if (!__runLatency && !__runThroughput) {
		std::cerr << "ERROR: At least one test must be selected." << std::endl;

		//Free up options memory
		delete[] options;
		delete[] buffer;

		return -1;
	}


	//Check working set size
	if (options[WORKING_SET_SIZE]) {
		__checkSingleOptionOccurrence(&options[WORKING_SET_SIZE]);
		char* endptr = NULL;
		size_t working_set_size_KB = strtoul(options[WORKING_SET_SIZE].arg, &endptr, 10);	
		if ((working_set_size_KB % 4) != 0) {
			std::cerr << "ERROR: Working set size must be specified in KB and be a multiple of 4 KB." << std::endl; 
			
			//Free up options memory
			delete[] options;
			delete[] buffer;

			return -1;
		}

		__working_set_size = working_set_size_KB * KB; //convert to bytes
	}

#ifdef VERBOSE
	std::cout << std::endl << "Working set:  \t\t\t";
#endif
#ifndef USE_LARGE_PAGES
#ifdef VERBOSE
	std::cout << __working_set_size << " B == " << __working_set_size / KB  << " KB == " << __working_set_size / MB << " MB (" << __working_set_size/(PAGE_SIZE) << " pages)" << std::endl;	
#endif
#else
	size_t num_large_pages = 0;
	if (__working_set_size <= xmem::common::g_large_page_size) //sub one large page, round up to one
		num_large_pages = 1;
	else if (__working_set_size % xmem::common::g_large_page_size == 0) //multiple of large page
		num_large_pages = __working_set_size / xmem::common::g_large_page_size;
	else //larger than one large page but not a multiple of large page
		num_large_pages = __working_set_size / xmem::common::g_large_page_size + 1;

#ifdef VERBOSE
	std::cout << __working_set_size << " B == " << __working_set_size / KB  << " KB == " << __working_set_size / MB << " MB (fits in " << num_large_pages << " large pages)" << std::endl;	
#endif

#endif

	//Check iterations
	if (options[ITERATIONS]) {
		__checkSingleOptionOccurrence(&options[ITERATIONS]);
		char *endptr = NULL;
		__iterations = static_cast<uint32_t>(strtoul(options[ITERATIONS].arg, &endptr, 10));
	}

#ifdef VERBOSE
	std::cout << "Iterations:  \t\t\t";
	std::cout << __iterations << std::endl;
#endif
	
	//Check starting test index
	if (options[BASE_TEST_INDEX]) {
		__checkSingleOptionOccurrence(&options[BASE_TEST_INDEX]);
		char *endptr = NULL;
		xmem::common::g_starting_test_index = static_cast<uint32_t>(strtoul(options[BASE_TEST_INDEX].arg, &endptr, 10));
	}
	else
		xmem::common::g_starting_test_index = 1;
	xmem::common::g_test_index = xmem::common::g_starting_test_index;

	//Check filename
	if (options[OUTPUT_FILE]) {
		__checkSingleOptionOccurrence(&options[OUTPUT_FILE]);
		__filename = options[OUTPUT_FILE].arg;
		__use_output_file = true;
	}

	//Free up options memory
	delete[] options;
	delete[] buffer;

	return 0;
}

bool Configurator::__checkSingleOptionOccurrence(third_party::Option* opt) {
	if (opt->count() > 1) {
		std::cerr << "WARNING: " << opt->name << " option can only be specified once. Anything but the first specified value will be ignored." << std::endl;
		return false;
	}
	return true;
}
