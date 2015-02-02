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
#include <Configurator.h>
#include <BenchmarkManager.h>

//Libraries
#include <iostream>

using namespace xmem;

/**
 *	@brief The main entry point to the program.
 */
int main(int argc, char* argv[]) {
	common::init_globals();
	common::print_welcome_message();
		
	config::Configurator config;
	bool configSuccess = !config.configureFromInput(argc, argv);

	if (configSuccess) {
		//Display useful info
#ifdef VERBOSE
		common::print_compile_time_options();
#endif

		if (common::query_sys_info()) {
			std::cerr << "ERROR occurred while querying CPU information." << std::endl;
			return -1;
		}

#ifdef VERBOSE
		common::test_thread_affinities();
		common::test_timers();
#endif

		benchmark::BenchmarkManager benchmgr(config.getWorkingSetSizePerThread(), config.getNumWorkerThreads(), config.useChunk32b(), config.useChunk64b(), config.useChunk128b(), config.useChunk256b(), config.isNUMAEnabled(), config.getIterationsPerTest(), config.useOutputFile(), config.getOutputFilename());
		if (config.throughputTestSelected()) {
			benchmgr.runThroughputBenchmarks();
		}

		if (config.latencyTestSelected()) {
			benchmgr.runLatencyBenchmarks();
		}
	}

	return !configSuccess;
}
