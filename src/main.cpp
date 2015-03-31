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
 * @brief main entry point to the tool
 * 
 * This tool is designed to measure bandwidth and latency
 * of the memory system using several access patterns, strides,
 * and working set sizes. The primary goal is to measure DRAM performance,
 * although it can also measure cache performance depending on the configuration.
 */


//Headers
#include <common.h>
#include <build_datetime.h>
#include <Configurator.h>
#include <BenchmarkManager.h>

//Libraries
#include <iostream>
#include <string>

using namespace xmem;

void xmem::print_welcome_message() {
	//Greetings!
	std::cout << "--------------------------------------------------------------------" << std::endl;
	std::cout << "Extensible Memory Benchmarking Tool (X-Mem) v" << VERSION << " for";
#ifdef _WIN32
	std::cout << " Windows";
#endif
#ifdef __gnu_linux__
	std::cout << " GNU/Linux";
#endif
	std::cout << std::endl;
	std::cout << "Build date: " << BUILD_DATETIME << std::endl;
	std::cout << "(C) Microsoft Corporation 2014" << std::endl;
	std::cout << "Originally authored by Mark Gottscho <mgottscho@ucla.edu>" << std::endl;
	std::cout << "--------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}

/**
 *	@brief The main entry point to the program.
 */
int main(int argc, char* argv[]) {
	init_globals();
	print_welcome_message();
	
	//Get info about the runtime system
	if (query_sys_info()) {
		std::cerr << "ERROR occurred while querying CPU information." << std::endl;
		return -1;
	}
	
	//Configure runtime based on user inputs
	Configurator config;
	bool configSuccess = !config.configureFromInput(argc, argv);
		
	if (configSuccess) {

		if (g_verbose) {
			print_compile_time_options();
			test_thread_affinities();
		}
		
		setup_timer();
		if (g_verbose)
			report_timer();

		BenchmarkManager benchmgr(config);
		if (config.throughputTestSelected()) {
			benchmgr.runThroughputBenchmarks();
		}

		if (config.latencyTestSelected()) {
			benchmgr.runLatencyBenchmarks();
		}

		if (config.customExtensionsEnabled()) {
			/***** USER-DEFINED FUNCTIONAL EXTENSIONS ******/
			std::cout << std::endl;
			std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
			std::cout << "++++++++++++ Starting custom X-Mem extensions ++++++++++++" << std::endl;
			std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
			std::cout << std::endl;
#ifdef EXT_LATENCY_DELAY_INJECTED_BENCHMARK
			std::cout << "EXTENSION DESCRIPTION: " << EXTENSION_DESCRIPTION << std::endl;
			if (config.getNumWorkerThreads() > 1)
				benchmgr.runCustomExtensions();	
			else {
				std::cerr << "ERROR: Number of worker threads must be at least 1." << std::endl;
				return false;
			}
#else
			std::cerr << "ERROR: No custom extensions are implemented in this build of X-Mem." << std::endl;
			return false;
#endif
			/***********************************************/
		}
	}

	return !configSuccess;
}
