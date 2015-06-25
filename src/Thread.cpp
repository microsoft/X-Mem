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
 * @brief Implementation file for the Thread class.
 */

//Headers
#include <Thread.h>

//Libraries
#include <stdlib.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __gnu_linux__
#include <pthread.h>
#endif

using namespace xmem;

Thread::Thread(Runnable* target) :
    __target(target),
    __created(false),
    __started(false),
    __completed(false),
    __suspended(false),
    __running(false),
    __thread_exit_code(0),
#ifdef _WIN32
    __thread_id(0),
#endif
    __thread_handle(0)
{
}

Thread::~Thread() {
    if (!cancel())
        std::cerr << "WARNING: Failed to forcefully kill a thread!" << std::endl;
}


bool Thread::create_and_start() {
#ifdef _WIN32
    if (__create())
        return __start();
    else
        return false;
#endif

#ifdef __gnu_linux__
    //We cannot use create() and start() on Linux because pthreads API does not allow for a thread created in the suspended state. So we just do it in one shot.
    if (__target != NULL) {
        pthread_attr_t attr;
        if (pthread_attr_init(&attr)) //Currently using just default flags. This may not be the optimal choice in general.
            return false;
        if (pthread_create(&__thread_handle, &attr, &Thread::__run_launchpad, __target))
            return false;
        __created = true;
        __started = true;
        __suspended = false;
        __running = true;

        return true;
    }
    return false;
#endif
}

bool Thread::join() {
    if (!__created || !__started)
        return false;

#ifdef _WIN32
    DWORD reason = WaitForSingleObject(__thread_handle, INFINITE);

    if (reason == WAIT_OBJECT_0) { //only succeed if the object was signaled. If timeout elapsed, or some error occurred, we will consider that a failure.
        __running = false;
        __suspended = false;
        __completed = true;
        return true;
    }

    return false;
#endif

#ifdef __gnu_linux__
    void* exit_pointer = NULL;
    int32_t failure = pthread_join(__thread_handle, &exit_pointer);
    if (exit_pointer)
        __thread_exit_code = *(static_cast<int32_t*>(exit_pointer));

    if (!failure) {
        __running = false;
        __suspended = false;
        __completed = true;
        return true;
    }
    return false;
#endif
}

bool Thread::cancel() {
#ifdef _WIN32
    if (__created) {
        if (TerminateThread(__thread_handle, -1)) { //This can be unsafe! Use with caution.
#endif
#ifdef __gnu_linux__
    if (__created && !__completed) {
        if (pthread_cancel(__thread_handle)) {
#endif
            __suspended = false;
            __running = false;
            __completed = true;
            return true;
        }
        return false;
    }
    return true;
}

int32_t Thread::getExitCode() {
    return __thread_exit_code;
}

bool Thread::started() {
    return __started;
}

bool Thread::completed() {
    return __completed;
}

bool Thread::validTarget() {
    return (__target != NULL);
}

bool Thread::created() {
    return __created;
}

bool Thread::isThreadSuspended() {
    return __suspended;
}

bool Thread::isThreadRunning() {
    return __running;
}

Runnable* Thread::getTarget() {
    return __target;
}

#ifdef _WIN32
bool Thread::__create() {
    if (__target != NULL) {
        DWORD temp;
        __thread_handle = CreateThread(NULL, 0, &Thread::__run_launchpad, __target, CREATE_SUSPENDED, &temp);
        if (__thread_handle == NULL)
            return false;
        __thread_id = static_cast<uint32_t>(temp);
        __created = true;
        __suspended = true;

        return true;
    }
    return false;
}
#endif

#ifdef _WIN32
bool Thread::__start() {
    if (__created) {
        if (ResumeThread(__thread_handle) == (DWORD)-1) //error condition check
            return false;

        __started = true;
        __running = true;
        __suspended = false;
        return true;
    }
    return false;
}
#endif

#ifdef _WIN32
DWORD Thread::__run_launchpad(void* target_runnable_object) {
    if (target_runnable_object != NULL) {
        Runnable* target = static_cast<Runnable*>(target_runnable_object);
        target->run();
        return 0;
    }
    return 1;
}
#endif

#ifdef __gnu_linux__
void* Thread::__run_launchpad(void* target_runnable_object) {
    int32_t* thread_retval = new int32_t;
    *thread_retval = 1;
    if (target_runnable_object != NULL) {
        Runnable* target = static_cast<Runnable*>(target_runnable_object);
        target->run();
        *thread_retval = 0;
        return static_cast<void*>(thread_retval);
    }
    return static_cast<void*>(thread_retval);
}
#endif
