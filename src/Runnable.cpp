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
 * @brief Implementation file for the Runnable class.
 */

//Headers
#include <Runnable.h>

//Libraries
#include <iostream>

using namespace xmem::thread;

Runnable::Runnable() :
	_mutex(0) {
#ifdef _WIN32
	if ((_mutex = CreateMutex(NULL, false, NULL)) == NULL)
		std::cerr << "WARNING: Failed to create mutex for a Runnable object! Thread-safe operation may not be possible." << std::endl;
#else
#error Windows is the only supported OS at this time.
#endif
}

Runnable::~Runnable() {
	if (_mutex != NULL)
		ReleaseMutex(_mutex); //Don't need to check return code. If it fails, the lock might not have been held anyway.
}

bool Runnable::_acquireLock(int32_t timeout) {
	if (_mutex == NULL)
		return false;

#ifdef _WIN32
	DWORD reason;

	if (timeout < 0) {
		reason = WaitForSingleObject(_mutex, INFINITE);
		if (reason == WAIT_OBJECT_0) //acquired mutex successfully
			return true;
		else {
			std::cerr << "WARNING: Failed to acquire lock in a Runnable object! Error code " << GetLastError() << std::endl;
			return false;
		}
	} else {
		reason = WaitForSingleObject(_mutex, timeout);
		if (reason == WAIT_OBJECT_0) //acquired mutex successfully
			return true;
		else if (reason != WAIT_TIMEOUT) {
			std::cerr << "WARNING: Failed to acquire lock in a Runnable object! Error code " << GetLastError() << std::endl;
			return false;
		}
	}
	
	return false;
#else
#error Windows is the only supported OS at this time.
#endif
}

bool Runnable::_releaseLock() {
#ifdef _WIN32
	if (ReleaseMutex(_mutex))
		return true;
	std::cerr << "WARNING: Failed to release lock in a Runnable object! Error code " << GetLastError() << std::endl;
	return false;
#else
#error Windows is the only supported OS at this time.
#endif
}
