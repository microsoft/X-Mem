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
 * @brief Implementation file for the LatencyBenchmark class.
 */

//Headers
#include <LatencyBenchmark.h>
#include <common.h>
#include <benchmark_kernels.h>
#include <MemoryWorker.h>
#include <LatencyWorker.h>
#include <LoadWorker.h>

//Libraries
#include <iostream>
#include <random>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

using namespace xmem;
        
LatencyBenchmark::LatencyBenchmark(
        void* mem_array,
        size_t len,
        uint32_t iterations,
        uint32_t num_worker_threads,
        uint32_t mem_node,
        uint32_t cpu_node,
        pattern_mode_t pattern_mode,
        rw_mode_t rw_mode,
        chunk_size_t chunk_size,
        int32_t stride_size,
        std::vector<PowerReader*> dram_power_readers,
        std::string name
    ) :
        Benchmark(
            mem_array,
            len,
            iterations,
            num_worker_threads,
            mem_node,
            cpu_node,
            pattern_mode,
            rw_mode,
            chunk_size,
            stride_size,
            dram_power_readers,
            "ns/access",
            name
        ),
        load_metric_on_iter_(),
        mean_load_metric_(0)
    { 

    for (uint32_t i = 0; i < iterations_; i++) 
        load_metric_on_iter_.push_back(0);
}

void LatencyBenchmark::reportBenchmarkInfo() const {
    std::cout << "CPU NUMA Node: " << cpu_node_ << std::endl;
    std::cout << "Memory NUMA Node: " << mem_node_ << std::endl;
    std::cout << "Latency measurement chunk size: ";
    std::cout << sizeof(uintptr_t)*8 << "-bit" << std::endl;
    std::cout << "Latency measurement access pattern: random read (pointer-chasing)" << std::endl;

    if (num_worker_threads_ > 1) {
        std::cout << "Load Chunk Size: ";
        switch (chunk_size_) {
            case CHUNK_32b:
                std::cout << "32-bit";
                break;
#ifdef HAS_WORD_64
            case CHUNK_64b:
                std::cout << "64-bit";
                break;
#endif
#ifdef HAS_WORD_128
            case CHUNK_128b:
                std::cout << "128-bit";
                break;
#endif
#ifdef HAS_WORD_256
            case CHUNK_256b:
                std::cout << "256-bit";
                break;
#endif
            default:
                std::cout << "UNKNOWN";
                break;
        }
        std::cout << std::endl;

        std::cout << "Load Access Pattern: ";
        switch (pattern_mode_) {
            case SEQUENTIAL:
                if (stride_size_ > 0)
                    std::cout << "forward ";
                else if (stride_size_ < 0)
                    std::cout << "reverse ";
                else 
                    std::cout << "UNKNOWN ";

                if (stride_size_ == 1 || stride_size_ == -1)
                    std::cout << "sequential";
                else 
                    std::cout << "strides of " << stride_size_ << " chunks";
                break;
            case RANDOM:
                std::cout << "random";
                break;
            default:
                std::cout << "UNKNOWN";
                break;
        }
        std::cout << std::endl;


        std::cout << "Load Read/Write Mode: ";
        switch (rw_mode_) {
            case READ:
                std::cout << "read";
                break;
            case WRITE:
                std::cout << "write";
                break;
            default:
                std::cout << "UNKNOWN";
                break;
        }
        std::cout << std::endl;

        std::cout << "Load number of worker threads: " << num_worker_threads_-1;
        std::cout << std::endl;
    }

    std::cout << std::endl;
}


void LatencyBenchmark::reportResults() const {
    std::cout << std::endl;
    std::cout << "*** RESULTS";
    std::cout << "***" << std::endl;
    std::cout << std::endl;
 
    if (has_run_) {
        for (uint32_t i = 0; i < iterations_; i++) {
            std::printf("Iter #%4d:    %0.3f %s @    %0.3f MB/s mean self-imposed load", i, metric_on_iter_[i], metric_units_.c_str(), load_metric_on_iter_[i]);
            //std::cout << "Iter #" << i << ": " << metric_on_iter_[i] << " " << metric_units_ << " @ " << load_metric_on_iter_[i] << " MB/s mean imposed load";
            if (warning_)
                std::cout << " (WARNING)";
            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << std::endl;

        std::cout << "Mean: " << mean_metric_ << " " << metric_units_ << " and " << mean_load_metric_ << " MB/s mean imposed load (not necessarily matched)";
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;

        std::cout << "Min: " << min_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "25th Percentile: " << percentile_25_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Median: " << median_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "75th Percentile: " << percentile_75_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "95th Percentile: " << percentile_95_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "99th Percentile: " << percentile_99_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << "Max: " << max_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
         
        std::cout << "Mode: " << mode_metric_ << " " << metric_units_;
        if (warning_)
            std::cout << " (WARNING)";
        std::cout << std::endl;
        
        std::cout << std::endl;
        std::cout << std::endl;
        
        for (uint32_t i = 0; i < dram_power_readers_.size(); i++) {
            if (dram_power_readers_[i] != NULL) {
                std::cout << dram_power_readers_[i]->name() << " Power Statistics..." << std::endl;
                std::cout << "...Mean Power: " << dram_power_readers_[i]->getMeanPower() * dram_power_readers_[i]->getPowerUnits() << " W" << std::endl;
                std::cout << "...Peak Power: " << dram_power_readers_[i]->getPeakPower() * dram_power_readers_[i]->getPowerUnits() << " W" << std::endl;
            }
        }
    }
    else
        std::cerr << "WARNING: Benchmark has not run yet. No reported results." << std::endl;
}

double LatencyBenchmark::getLoadMetricOnIter(uint32_t iter) const {
    if (has_run_ && iter - 1 <= iterations_)
        return load_metric_on_iter_[iter - 1];
    else //bad call
        return -1;
}

double LatencyBenchmark::getMeanLoadMetric() const {     
    if (has_run_)
        return mean_load_metric_;
    else //bad call
        return -1;
}

bool LatencyBenchmark::runCore() {
    size_t len_per_thread = len_ / num_worker_threads_; //Carve up memory space so each worker has its own area to play in

    //Set up latency measurement kernel function pointers
    RandomFunction lat_kernel_fptr = &chasePointers;
    RandomFunction lat_kernel_dummy_fptr = &dummy_chasePointers;

    //Initialize memory regions for all threads by writing to them, causing the memory to be physically resident.
    forwSequentialWrite_Word32(mem_array_,
                               reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_)+len_)); //static casts to silence compiler warnings

    //Build pointer indices for random-access latency thread. We assume that latency thread is the first one, so we use beginning of memory region.
    if (!buildRandomPointerPermutation(mem_array_,
                                       reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_)+len_per_thread), //static casts to silence compiler warnings
#ifndef HAS_WORD_64 //special case: 32-bit architectures
                                       CHUNK_32b)) { 
#endif
#ifdef HAS_WORD_64
                                       CHUNK_64b)) { 
#endif
        std::cerr << "ERROR: Failed to build a random pointer permutation for the latency measurement thread!" << std::endl;
        return false;
    }

    //Set up load generation kernel function pointers
    SequentialFunction load_kernel_fptr_seq = NULL;
    SequentialFunction load_kernel_dummy_fptr_seq = NULL; 
    RandomFunction load_kernel_fptr_ran = NULL;
    RandomFunction load_kernel_dummy_fptr_ran = NULL; 
    if (num_worker_threads_ > 1) { //If we only have one worker thread, it is used for latency measurement only, and no load threads will be used.
        if (pattern_mode_ == SEQUENTIAL) {
            if (!determineSequentialKernel(rw_mode_, chunk_size_, stride_size_, &load_kernel_fptr_seq, &load_kernel_dummy_fptr_seq)) {
                std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
                return false;
            }
        } else if (pattern_mode_ == RANDOM) {
            if (!determineRandomKernel(rw_mode_, chunk_size_, &load_kernel_fptr_ran, &load_kernel_dummy_fptr_ran)) {
                std::cerr << "ERROR: Failed to find appropriate benchmark kernel." << std::endl;
                return false;
            }

            //Build pointer indices for random-access load threads. Note that the pointers for each load thread must stay within its respective region, otherwise sharing may occur. 
            for (uint32_t i = 1; i < num_worker_threads_; i++) {
                if (!buildRandomPointerPermutation(reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_) + i*len_per_thread), //static casts to silence compiler warnings
                                                   reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_) + (i+1)*len_per_thread), //static casts to silence compiler warnings
                                                   chunk_size_)) {
                    std::cerr << "ERROR: Failed to build a random pointer permutation for a load generation thread!" << std::endl;
                    return false;
                }
            }
        } else {
            std::cerr << "ERROR: Got an invalid pattern mode." << std::endl;
            return false;
        }
    }

    //Set up some stuff for worker threads
    std::vector<MemoryWorker*> workers;
    std::vector<Thread*> worker_threads;
    
    //Start power measurement
    if (g_verbose)
        std::cout << "Starting power measurement threads...";
    
    if (!startPowerThreads()) {
        if (g_verbose)
            std::cout << "FAIL" << std::endl;
        std::cerr << "WARNING: Failed to start power threads." << std::endl;
    } else if (g_verbose)
        std::cout << "done" << std::endl;
    
    //Run benchmark
    if (g_verbose)
        std::cout << "Running benchmark." << std::endl << std::endl;

    //Do a bunch of iterations of the core benchmark routine
    for (uint32_t i = 0; i < iterations_; i++) {

        //Create load workers and load worker threads
        for (uint32_t t = 0; t < num_worker_threads_; t++) {
            void* thread_mem_array = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(mem_array_) + t*len_per_thread);
            int32_t cpu_id = cpu_id_in_numa_node(cpu_node_, t);
            if (cpu_id < 0)
                std::cerr << "WARNING: Failed to find logical CPU " << t << " in NUMA node " << cpu_node_ << std::endl;
            if (t == 0) { //special case: thread 0 is always latency thread
                workers.push_back(new LatencyWorker(thread_mem_array,
                                                    len_per_thread,
                                                    lat_kernel_fptr,
                                                    lat_kernel_dummy_fptr,
                                                    cpu_id));
            } else {
                if (pattern_mode_ == SEQUENTIAL)
                    workers.push_back(new LoadWorker(thread_mem_array,
                                                     len_per_thread,
                                                     load_kernel_fptr_seq,
                                                     load_kernel_dummy_fptr_seq,
                                                     cpu_id));
                else if (pattern_mode_ == RANDOM)
                    workers.push_back(new LoadWorker(thread_mem_array,
                                                     len_per_thread,
                                                     load_kernel_fptr_ran,
                                                     load_kernel_dummy_fptr_ran,
                                                     cpu_id));
                else
                    std::cerr << "WARNING: Invalid benchmark pattern mode." << std::endl;
            }
            worker_threads.push_back(new Thread(workers[t]));
        }

        //Start worker threads! gogogo
        for (uint32_t t = 0; t < num_worker_threads_; t++)
            worker_threads[t]->create_and_start();

        //Wait for all threads to complete
        for (uint32_t t = 0; t < num_worker_threads_; t++)
            if (!worker_threads[t]->join())
                std::cerr << "WARNING: A worker thread failed to complete correctly!" << std::endl;
        
        //Compute metrics for this iteration
        bool iterwarning = false;

        //Compute latency metric
        uint32_t lat_passes = workers[0]->getPasses();  
        tick_t lat_adjusted_ticks = workers[0]->getAdjustedTicks();
        tick_t lat_elapsed_dummy_ticks = workers[0]->getElapsedDummyTicks();
        uint32_t lat_bytes_per_pass = workers[0]->getBytesPerPass();
        uint32_t lat_accesses_per_pass = lat_bytes_per_pass / 8;
        iterwarning |= workers[0]->hadWarning();
        
        //Compute throughput generated by load threads
        uint32_t load_total_passes = 0;
        tick_t load_total_adjusted_ticks = 0;
        tick_t load_total_elapsed_dummy_ticks = 0;
        uint32_t load_bytes_per_pass = 0;
        double load_avg_adjusted_ticks = 0;
        for (uint32_t t = 1; t < num_worker_threads_; t++) {
            load_total_passes += workers[t]->getPasses();
            load_total_adjusted_ticks += workers[t]->getAdjustedTicks();
            load_total_elapsed_dummy_ticks += workers[t]->getElapsedDummyTicks();
            load_bytes_per_pass = workers[t]->getBytesPerPass(); //all should be the same.
            iterwarning |= workers[t]->hadWarning();
        }

        //Compute load metrics for this iteration
        load_avg_adjusted_ticks = static_cast<double>(load_total_adjusted_ticks) / (num_worker_threads_-1);
        if (num_worker_threads_ > 1)
            load_metric_on_iter_[i] = (((static_cast<double>(load_total_passes) * static_cast<double>(load_bytes_per_pass)) / static_cast<double>(MB)))   /  ((load_avg_adjusted_ticks * g_ns_per_tick) / 1e9);

        if (iterwarning)
            warning_ = true;
    
        if (g_verbose) { //Report metrics for this iteration
            //Latency thread
            std::cout << "Iter " << i+1 << " had " << lat_passes << " latency measurement passes, with " << lat_accesses_per_pass << " accesses per pass:";
            if (iterwarning) std::cout << " -- WARNING";
            std::cout << std::endl;

            std::cout << "...lat clock ticks == " << lat_adjusted_ticks << " (adjusted by -" << lat_elapsed_dummy_ticks << ")";
            if (iterwarning) std::cout << " -- WARNING";
            std::cout << std::endl;

            std::cout << "...lat ns == " << lat_adjusted_ticks * g_ns_per_tick << " (adjusted by -" << lat_elapsed_dummy_ticks * g_ns_per_tick << ")";
            if (iterwarning) std::cout << " -- WARNING";
            std::cout << std::endl;

            std::cout << "...lat sec == " << lat_adjusted_ticks * g_ns_per_tick / 1e9 << " (adjusted by -" << lat_elapsed_dummy_ticks * g_ns_per_tick / 1e9 << ")";
            if (iterwarning) std::cout << " -- WARNING";
            std::cout << std::endl;

            //Load threads
            if (num_worker_threads_ > 1) {
                std::cout << "Iter " << i+1 << " had " << load_total_passes << " total load generation passes, with " << load_bytes_per_pass << " bytes per pass:";
                if (iterwarning) std::cout << " -- WARNING";
                std::cout << std::endl;

                std::cout << "...load total clock ticks across " << num_worker_threads_-1 << " threads == " << load_total_adjusted_ticks << " (adjusted by -" << load_total_elapsed_dummy_ticks << ")";
                if (iterwarning) std::cout << " -- WARNING";
                std::cout << std::endl;

                std::cout << "...load total ns across " << num_worker_threads_-1 << " threads == " << load_total_adjusted_ticks * g_ns_per_tick << " (adjusted by -" << load_total_elapsed_dummy_ticks * g_ns_per_tick << ")";
                if (iterwarning) std::cout << " -- WARNING";
                std::cout << std::endl;

                std::cout << "...load total sec across " << num_worker_threads_-1 << " threads == " << load_total_adjusted_ticks * g_ns_per_tick / 1e9 << " (adjusted by -" << load_total_elapsed_dummy_ticks * g_ns_per_tick / 1e9 << ")";
                if (iterwarning) std::cout << " -- WARNING";
                std::cout << std::endl;
            }

        }
        
        //Compute overall metrics for this iteration
        metric_on_iter_[i] = static_cast<double>(lat_adjusted_ticks * g_ns_per_tick)  /  static_cast<double>(lat_accesses_per_pass * lat_passes);
        
        //Clean up workers and threads for this iteration
        for (uint32_t t = 0; t < num_worker_threads_; t++) {
            delete worker_threads[t];
            delete workers[t];
        }
        worker_threads.clear();
        workers.clear();
    }

    //Stop power measurement
    if (g_verbose) {
        std::cout << std::endl;
        std::cout << "Stopping power measurement threads...";
    }
    
    if (!stopPowerThreads()) {
        if (g_verbose)
            std::cout << "FAIL" << std::endl;
        std::cerr << "WARNING: Failed to stop power measurement threads." << std::endl;
    } else if (g_verbose)
        std::cout << "done" << std::endl;
    
    //Run metadata
    has_run_ = true;
    computeMetrics();

    //Get mean load metrics -- these aren't part of Benchmark class thus not covered by computeMetrics()
    for (uint32_t i = 0; i < iterations_; i++)
        mean_load_metric_ += load_metric_on_iter_[i];
    mean_load_metric_ /= static_cast<double>(iterations_);

    return true;
}
