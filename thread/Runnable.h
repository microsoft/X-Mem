/**
 * @file
 * 
 * @brief Header file for the Runnable class.
 * 
 * (C) 2014 Microsoft Corporation
 */

#ifndef __RUNNABLE_H
#define __RUNNABLE_H

//Libraries
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#else
#error Windows is the only supported OS at this time.
#endif

namespace xmem {
	namespace thread {
		/**
		 * @brief A base class for any object that implements a thread-safe run() function for use by Thread objects.
		 */
		class Runnable {
			public:
				/**
				 * @brief Constructor.
				 */
				Runnable();

				/**
				 * @brief Destructor.
				 */
				~Runnable();

				/**
				 * @brief Does some "work". Pure virtual method that any derived class must implement in a thread-safe manner.
				 */
				virtual void run() = 0;

			protected:
				/** 
				 * @brief Acquires the object lock to access all object state in thread-safe manner.
				 * @param timeout timeout in milliseconds to acquire the lock. If 0, does not wait at all. If negative, waits indefinitely.
				 * @returns true on success. If not successful, the lock was not acquired, possibly due to a timeout, or the lock might already be held.
				 */
				bool _acquireLock(int32_t timeout);

				/** 
				 * @brief Releases the object lock to access all object state in thread-safe manner.
				 * @returns true on success. If not successful, the lock is either still held or the call was illegal (e.g., releasing a lock that was never acquired).
				 */
				bool _releaseLock();

#ifdef _WIN32
				HANDLE _mutex; /**< A handle to the OS mutex, i.e., the locking mechanism. Outside the constructor, this should only be accessed via _acquireLock() and _releaseLock(). */
#else
#error Windows is the only supported OS at this time.
#endif
		};
	};
};

#endif
