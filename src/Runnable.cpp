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
 * @brief Implementation file for the Runnable class.
 */

//Headers
#include <Runnable.h>

//Libraries
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __gnu_linux__
#include <pthread.h>
#include <time.h>
#endif

using namespace xmem;

Runnable::Runnable() :
#ifdef _WIN32
    mutex(0) 
#endif
#ifdef __gnu_linux__
    mutex_(PTHREAD_MUTEX_INITIALIZER)
#endif
    {
#ifdef _WIN32
    if ((mutex = CreateMutex(NULL, false, NULL)) == NULL)
        std::cerr << "WARNING: Failed to create mutex for a Runnable object! Thread-safe operation may not be possible." << std::endl;
#endif
}

Runnable::~Runnable() {
#ifdef _WIN32
    if (mutex != NULL)
        ReleaseMutex(mutex); //Don't need to check return code. If it fails, the lock might not have been held anyway.
#endif

#ifdef __gnu_linux__
    int32_t retval = pthread_mutex_destroy(&mutex_); 
    if (retval) {
        if (retval == EBUSY)
            std::cerr << "WARNING: Failed to destroy a mutex, as it was busy!" << std::endl;
        else if (retval == EINVAL)
            std::cerr << "WARNING: Failed to destroy a mutex, as it was invalid!" << std::endl;
        else
            std::cerr << "WARNING: Failed to destroy a mutex for an unknown reason, error number " << retval << "!" << std::endl;
    }
#endif
}

bool Runnable::acquireLock(int32_t timeout) {
#ifdef _WIN32
    if (mutex == NULL)
        return false;
#endif

#ifdef _WIN32
    DWORD reason;
#endif

#ifdef __gnu_linux__
    int32_t reason;
#endif

    if (timeout < 0) {
#ifdef _WIN32
        reason = WaitForSingleObject(mutex, INFINITE);
        if (reason == WAIT_OBJECT_0) //success
#endif
#ifdef __gnu_linux__
        reason = pthread_mutex_lock(&mutex_);
        if (!reason) //success
#endif
            return true;
        else {
            std::cerr << "WARNING: Failed to acquire lock in a Runnable object! Error code " 
#ifdef _WIN32
            << GetLastError() 
#endif
#ifdef __gnu_linux__
            << reason
#endif
            << std::endl;
            return false;
        }
    } else {
#ifdef _WIN32
        reason = WaitForSingleObject(mutex, timeout);
        if (reason == WAIT_OBJECT_0) //success
#endif
#ifdef __gnu_linux__
        struct timespec t;
        t.tv_sec = static_cast<time_t>(timeout/1000);
        t.tv_nsec = static_cast<time_t>((timeout % 1000) * 1e6);
        reason = pthread_mutex_timedlock(&mutex_, &t);
        if (!reason) //success
#endif
            return true;
#ifdef _WIN32
        else if (reason != WAIT_TIMEOUT) {
#endif
#ifdef __gnu_linux__
        else if (reason != ETIMEDOUT) {
#endif
            std::cerr << "WARNING: Failed to acquire lock in a Runnable object! Error code "
#ifdef _WIN32
            << GetLastError() 
#endif
#ifdef __gnu_linux__        
            << reason
#endif
            << std::endl;
            return false;
        }
    }
    
    return false;
}

bool Runnable::releaseLock() {
#ifdef _WIN32
    if (ReleaseMutex(mutex))
#endif
#ifdef __gnu_linux__
    int32_t retval = pthread_mutex_unlock(&mutex_);
    if (!retval)
#endif
        return true;
    std::cerr << "WARNING: Failed to release lock in a Runnable object! Error code "
#ifdef _WIN32
    << GetLastError()
#endif
#ifdef __gnu_linux__
    << retval
#endif
    << std::endl;
    return false;
}
