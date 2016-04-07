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
    target_(target),
    created_(false),
    started_(false),
    completed_(false),
    suspended_(false),
    running_(false),
    thread_exit_code_(0),
#ifdef _WIN32
    thread_id_(0),
#endif
    thread_handle_(0)
{
}

Thread::~Thread() {
    if (!cancel())
        std::cerr << "WARNING: Failed to forcefully kill a thread!" << std::endl;
}


bool Thread::create_and_start() {
#ifdef _WIN32
    if (create())
        return start();
    else
        return false;
#endif

#ifdef __gnu_linux__
    //We cannot use create() and start() on Linux because pthreads API does not allow for a thread created in the suspended state. So we just do it in one shot.
    if (target_ != NULL) {
        pthread_attr_t attr;
        if (pthread_attr_init(&attr)) //Currently using just default flags. This may not be the optimal choice in general.
            return false;
        if (pthread_create(&thread_handle_, &attr, &Thread::runLaunchpad, target_))
            return false;
        created_ = true;
        started_ = true;
        suspended_ = false;
        running_ = true;

        return true;
    }
    return false;
#endif
}

bool Thread::join() {
    if (!created_ || !started_)
        return false;

#ifdef _WIN32
    DWORD reason = WaitForSingleObject(thread_handle_, INFINITE);

    if (reason == WAIT_OBJECT_0) { //only succeed if the object was signaled. If timeout elapsed, or some error occurred, we will consider that a failure.
        running_ = false;
        suspended_ = false;
        completed_ = true;
        return true;
    }

    return false;
#endif

#ifdef __gnu_linux__
    void* exit_pointer = NULL;
    int32_t failure = pthread_join(thread_handle_, &exit_pointer);
    if (exit_pointer)
        thread_exit_code_ = *(static_cast<int32_t*>(exit_pointer));

    if (!failure) {
        running_ = false;
        suspended_ = false;
        completed_ = true;
        return true;
    }
    return false;
#endif
}

bool Thread::cancel() {
#ifdef _WIN32
    if (created_) {
        if (TerminateThread(thread_handle_, -1)) { //This can be unsafe! Use with caution.
#endif
#ifdef __gnu_linux__
    if (created_ && !completed_) {
        if (pthread_cancel(thread_handle_)) {
#endif
            suspended_ = false;
            running_ = false;
            completed_ = true;
            return true;
        }
        return false;
    }
    return true;
}

int32_t Thread::getExitCode() {
    return thread_exit_code_;
}

bool Thread::started() {
    return started_;
}

bool Thread::completed() {
    return completed_;
}

bool Thread::validTarget() {
    return (target_ != NULL);
}

bool Thread::created() {
    return created_;
}

bool Thread::isThreadSuspended() {
    return suspended_;
}

bool Thread::isThreadRunning() {
    return running_;
}

Runnable* Thread::getTarget() {
    return target_;
}

#ifdef _WIN32
bool Thread::create() {
    if (target_ != NULL) {
        DWORD temp;
        thread_handle_ = CreateThread(NULL, 0, &Thread::runLaunchpad, target_, CREATE_SUSPENDED, &temp);
        if (thread_handle_ == NULL)
            return false;
        thread_id_ = static_cast<uint32_t>(temp);
        created_ = true;
        suspended_ = true;

        return true;
    }
    return false;
}
#endif

#ifdef _WIN32
bool Thread::start() {
    if (created_) {
        if (ResumeThread(thread_handle_) == (DWORD)-1) //error condition check
            return false;

        started_ = true;
        running_ = true;
        suspended_ = false;
        return true;
    }
    return false;
}
#endif

#ifdef _WIN32
DWORD Thread::runLaunchpad(void* target_runnable_object) {
    if (target_runnable_object != NULL) {
        Runnable* target = static_cast<Runnable*>(target_runnable_object);
        target->run();
        return 0;
    }
    return 1;
}
#endif

#ifdef __gnu_linux__
void* Thread::runLaunchpad(void* target_runnable_object) {
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
