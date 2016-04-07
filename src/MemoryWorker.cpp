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
 * @brief Implementation file for the MemoryWorker class.
 */

//Headers
#include <MemoryWorker.h>
#include <common.h>

using namespace xmem;

MemoryWorker::MemoryWorker(
        void* mem_array,
        size_t len,
        int32_t cpu_affinity
    ) :
        _mem_array(mem_array),
        _len(len),
        _cpu_affinity(cpu_affinity),
        _bytes_per_pass(0),
        _passes(0),
        _elapsed_ticks(0),
        _elapsed_dummy_ticks(0),
        _adjusted_ticks(0),
        _warning(false),
        _completed(false)
    {
}

MemoryWorker::~MemoryWorker() {
}

size_t MemoryWorker::getLen() {
    size_t retval = 0;
    if (acquireLock(-1)) {
        retval = _len;
        releaseLock();
    }

    return retval;
}

uint32_t MemoryWorker::getBytesPerPass() {
    uint32_t retval = 0;
    if (acquireLock(-1)) {
        retval = _bytes_per_pass;
        releaseLock();
    }

    return retval;
}

uint32_t MemoryWorker::getPasses() {
    uint32_t retval = 0;
    if (acquireLock(-1)) {
        retval = _passes;
        releaseLock();
    }

    return retval;
}
                
tick_t MemoryWorker::getElapsedTicks() {
    tick_t retval = 0;
    if (acquireLock(-1)) {
        retval = _elapsed_ticks;
        releaseLock();
    }

    return retval;
}

tick_t MemoryWorker::getElapsedDummyTicks() {
    tick_t retval = 0;
    if (acquireLock(-1)) {
        retval = _elapsed_dummy_ticks;
        releaseLock();
    }

    return retval;
}

tick_t MemoryWorker::getAdjustedTicks() {
    tick_t retval = 0;
    if (acquireLock(-1)) {
        retval = _adjusted_ticks;
        releaseLock();
    }

    return retval;
}

bool MemoryWorker::hadWarning() {
    bool retval = true;
    if (acquireLock(-1)) {
        retval = _warning;
        releaseLock();
    }

    return retval;
}
