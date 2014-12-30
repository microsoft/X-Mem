/**
 * @file
 * 
 * @brief main entry point to the tool
 * 
 * This tool is designed to measure bandwidth and latency
 * of the memory system using several access patterns, strides,
 * and working set sizes. The primary goal is to measure DRAM performance,
 * although it can also measure cache performance depending on the configuration.
 * 
 * (C) 2014 Microsoft Corporation
 */

//Headers
#include <common/common.h>
#include <common/win/win_common.h>
#include <common/win/third_party/win_common_third_party.h>
#include <config/Configurator.h>
#include <benchmark/BenchmarkManager.h>

//Libraries
#include <iostream>

using namespace xmem;

/**
 *	@brief The main entry point to the program.
 */
int main(int argc, char* argv[]) {
	common::print_welcome_message();
		
	//Display useful info
#ifdef VERBOSE
	common::print_compile_time_options();
#endif
	if (common::win::third_party::query_sys_info()) {
		std::cerr << "ERROR occurred while querying CPU information." << std::endl;
		return -1;
	}

	config::Configurator config;
	bool configSuccess = !config.configureFromInput(argc, argv);

	if (configSuccess) {
#ifdef VERBOSE
		common::print_types_report();
		common::test_thread_affinities();
		common::test_timers();
#endif

		benchmark::BenchmarkManager benchmgr(config.getWorkingSetSize(), config.getIterationsPerTest(), config.useOutputFile(), config.getOutputFilename());
		if (config.throughputTestSelected()) {
			benchmgr.runThroughputBenchmarks();
		}

		if (config.latencyTestSelected()) {
			benchmgr.runLatencyBenchmarks();
		}
	}

	return !configSuccess;
}
