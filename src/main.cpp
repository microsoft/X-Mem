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
 *
 * Author: Mark Gottscho <mgottscho@ucla.edu>
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
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Extensible Memory Benchmarking Tool (X-Mem) v" << VERSION << " for";
#ifdef _WIN32
    std::cout << " Windows";
#endif
#ifdef __gnu_linux__
    std::cout << " GNU/Linux";
#endif
    std::cout << " on";
#ifdef ARCH_INTEL_X86_64
    std::cout << " Intel x86-64";
#endif
#ifdef ARCH_INTEL_AVX
    std::cout << " (AVX)";
#endif
#ifdef ARCH_INTEL_X86
    std::cout << " Intel x86 (32-bit)";
#endif
#ifdef ARCH_INTEL_MIC
    std::cout << " Intel Xeon Phi MIC (64-bit)";
#endif
#ifdef ARCH_ARM
    std::cout << " ARM (32-bit)";
#endif
#ifdef ARCH_ARM_64
    std::cout << " ARMv8 (64-bit)";
#endif
#ifdef ARCH_ARM_NEON
    std::cout << " ARM (32-bit) with NEON";
#endif
    std::cout << std::endl;
    std::cout << "Build date: " << BUILD_DATETIME << std::endl;
    std::cout << "Indicated compiler(s): ";
#ifdef COMPILER_MS_VS
    std::cout << "Microsoft Visual C++ ";
#endif
#ifdef COMPILER_GCC_GXX
    std::cout << "GNU C/C++ (gcc/g++) ";
#endif
#ifdef COMPILER_ICC
    std::cout << "Intel C/C++ (icc) ";
#endif
#ifdef COMPILER_CLANG_LLVM
    std::cout << "Clang/LLVM (llvm) ";
#endif
    std::cout << std::endl;
    std::cout << "(C) Microsoft Corporation 2015" << std::endl;
    std::cout << "Originally authored by Mark Gottscho <mgottscho@ucla.edu>" << std::endl;
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
}

/**
 *  @brief The main entry point to the program.
 */
int main(int argc, char* argv[]) {
    bool config_success = false;
    try {
        init_globals();
        print_welcome_message();
        
        //Get info about the runtime system
        if (query_sys_info()) {
            std::cerr << "ERROR occurred while querying CPU information." << std::endl;
            return -1;
        }
        
        //Configure runtime based on user inputs
        Configurator config;
        config_success = !config.configureFromInput(argc, argv);
            
        if (config_success) {
            if (g_verbose) {
                print_compile_time_options();
                print_types_report();
                report_sys_info();
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

            if (config.extensionsEnabled()) {
                std::cout << std::endl;
                std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                std::cout << "++++++++++++ Starting custom X-Mem extensions ++++++++++++" << std::endl;
                std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                std::cout << std::endl;

                /***** USER-DEFINED FUNCTIONAL EXTENSIONS ******/
#ifdef EXT_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK
                if (config.runExtDelayInjectedLoadedLatencyBenchmark()) {
                    std::cout << "EXTENSION " << EXT_NUM_DELAY_INJECTED_LOADED_LATENCY_BENCHMARK << ": Loaded latency benchmarks with delay injected kernels on load threads." << std::endl;
                    benchmgr.runExtDelayInjectedLoadedLatencyBenchmark();   
                }
#endif

#ifdef EXT_STREAM_BENCHMARK
                if (config.runExtStreamBenchmark()) {
                    std::cout << "EXTENSION " << EXT_NUM_STREAM_BENCHMARK << ": STREAM-like throughput benchmark using stream copy, add, and triad kernels." << std::endl;
                    benchmgr.runExtStreamBenchmark();
                }
#endif
                /***********************************************/

                std::cout << std::endl;
                std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                std::cout << "++++++++++++ Finished custom X-Mem extensions ++++++++++++" << std::endl;
                std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
                std::cout << std::endl;
            }
        }
    }
    catch (...) {
        std::cerr << "Uncaught exception in main(), terminating." << std::endl;
        return EXIT_FAILURE;
    }

    if (config_success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
