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
 * @brief Header file for the Thread class.
 */

#ifndef THREAD_H
#define THREAD_H

//Headers
#include <Runnable.h>

//Libraries
#include <cstdint>

#ifdef __gnu_linux__
#include <pthread.h>
#endif

namespace xmem {

    /**
     * @brief a nice wrapped thread interface independent of particular OS API
     */
    class Thread {
        public:
            /**
             * Constructor. Does not actually create the real thread or run it.
             * @param target The target object to do some work with in a new thread.
             */
            Thread(Runnable* target);
            
            /** 
             * Destructor. Immediately cancels the thread if it exists. This can be unsafe!
             */
            ~Thread();

            /**
             * Creates and starts the thread immediately if the target Runnable is valid. This invokes the run() method in the Runnable target that was passed in the constructor.
             * @returns true if the thread was successfully created and started.
             */
            bool create_and_start();

            /**
             * Blocks the calling thread until the worker thread managed by this object terminates. For simplicity, this does not support a timeout due to pthreads incompatibility with the Windows threading API.
             * If the worker thread has already terminated, returns immediately.
             * If the worker has not yet started, returns immediately.
             * @returns true if the worker thread terminated successfully, false otherwise.
             */
            bool join();

            /**
             * Cancels the worker thread immediately. This should only be done in emergencies, as it is effectively killed and undefined behavior might occur.
             * @returns true if the worker thread was successfully killed.
             */
            bool cancel();

            /**
             * @returns the exit code of the worker thread if it completed. If it did not complete or has not started, returns 0.
             */
            int32_t getExitCode();

            /**
             * @returns true if the thread has been started, regardless if has completed or not.
             */
            bool started();

            /**
             * @returns true if the thread completed, regardless of the manner in which it terminated. Returns false if it has not been started.
             */
            bool completed();

            /**
             * @returns true if the Runnable target is valid.
             */
            bool validTarget();

            /**
             * @returns true if the thread has been created successfully.
             */
            bool created();

            /**
             * @returns true if the thread is suspended. Returns false if the thread has not been created.
             */
            bool isThreadSuspended();

            /**
             * @returns true if the thread is running. Returns false if the thread has not been created.
             */
            bool isThreadRunning();

            /**
             * @returns a pointer to the target Runnable object
             */
            Runnable* getTarget();

        private:
#ifdef _WIN32
            /**
             * Creates the thread if the target Runnable is valid, but does not start running it.
             * @returns true if the thread was successfully created.
             */
            bool create();
#endif

#ifdef _WIN32
            /**
             * Starts the thread immediately if the thread has been created. This invokes the run() method in the Runnable target that was passed in the constructor.
             * @returns true if the thread was successfully started.
             */
            bool start();
#endif
            

            /**
             * Invokes the run() method on the target Runnable object.
             * @param target_runnable_object pointer to the target Runnable object. This needs to be a generic pointer to keep APIs happy.
             */
#ifdef _WIN32
            static DWORD WINAPI runlaunchpad(void* target_runnable_object);
#endif
#ifdef __gnu_linux__
            static void* runLaunchpad(void* target_runnable_object);
#endif

            Runnable* target_; /**< The object connecting a run() method which operates in a thread-safe manner. */
            bool created_; /**< If true, the OS thread has been created at some point, but does not indicate its current state. */
            bool started_; /**< If true, the OS thread has been started at some point, but does not indicate its current state. */
            bool completed_; /**< If true, the OS thread completed. */
            bool suspended_; /**< If true, the OS thread is suspended. */
            bool running_; /**< If true, the OS thread is running. */
            int32_t thread_exit_code_; /**< Contains the thread's exit status code. If it has not yet exited, this should be 0 (normal condition). */

#ifdef _WIN32
            uint32_t thread_id_; /**< The OS thread ID. */
#endif

            /** A handle to the OS thread. */
#ifdef _WIN32
            HANDLE thread_handle_;
#endif
#ifdef __gnu_linux__
            pthread_t thread_handle_;
#endif
    };
};

#endif
