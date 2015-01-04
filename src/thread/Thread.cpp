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
 * @brief Implementation file for the Thread class.
 */

//Libraries
#include <stdlib.h>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#else
#error Windows is the only supported OS at this time.
#endif

//Headers
#include "Thread.h"

using namespace xmem::thread;

Thread::Thread(Runnable* target) :
	__target(target),
	__created(false),
	__started(false),
	__completed(false),
	__suspended(false),
	__running(false),
	__thread_exit_code(0),
#ifdef _WIN32
	__thread_handle(0),
#else
#error Windows is the only supported OS at this time.
#endif
	__thread_id(0)
{
}

Thread::~Thread() {
	if (!cancel())
		std::cerr << "WARNING: Failed to forcefully kill a thread!" << std::endl;
}

bool Thread::create() {
	if (__target != NULL) {
#ifdef _WIN32
		DWORD temp;
		__thread_handle = CreateThread(NULL, 0, &Thread::__run_launchpad, __target, CREATE_SUSPENDED, &temp);
		if (__thread_handle == NULL)
			return false;
		__thread_id = static_cast<uint32_t>(temp);
#else
#error Windows is the only supported OS at this time.
#endif
		__created = true;
		__suspended = true;

		return true;
	}
	return false;
}

bool Thread::start() {
	if (__created) {
#ifdef _WIN32
		if (ResumeThread(__thread_handle) == (DWORD)-1) //error condition check
			return false;

		__started = true;
		__running = true;
		__suspended = false;
		return true;
#else
#error Windows is the only supported OS at this time.
#endif
	}
	return false;
}

bool Thread::create_and_start() {
	if (create())
		return start();
	else
		return false;
}

bool Thread::join(int32_t timeout) {
#ifdef _WIN32
	DWORD reason;
	if (timeout < 0)
		reason = WaitForSingleObject(__thread_handle, INFINITE);
	else 
		reason = WaitForSingleObject(__thread_handle, static_cast<DWORD>(timeout));

	if (reason == WAIT_OBJECT_0) { //only succeed if the object was signaled. If timeout elapsed, or some error occurred, we will consider that a failure.
		__running = false;
		__suspended = false;
		__completed = true;
		return true;
	}

	return false;
#else
#error Windows is the only supported OS at this time.
#endif
}

bool Thread::cancel() {
	if (__created) {
		if (TerminateThread(__thread_handle, -1)) { //This can be unsafe! Use with caution.
			__suspended = false;
			__running = false;
			__completed = true;
			return true;
		}
	}
	return false;
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
DWORD Thread::__run_launchpad(void* target_runnable_object) {
	if (target_runnable_object != NULL) {
		Runnable* target = static_cast<Runnable*>(target_runnable_object);
		target->run();
	}
	return 0;
}
#else
#error Windows is the only supported OS at this time.
#endif
