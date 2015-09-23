/*
 * Copyright 2015 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HEADER__TINYTASK_H
#define HEADER__TINYTASK_H

#include <vector>

namespace tinytask {

	namespace internal {

		struct BaseTask {
			virtual ~BaseTask() = 0;
			virtual void run() = 0;
		};

		template<typename T, typename Context>
		struct Task : BaseTask {
			T* entries;
			int nentries;
			void (*func)(int threadIndex, T*, int, Context*);
			Context* context;
			int threadIndex;

			virtual void run() {
				func(threadIndex, entries, nentries, context);
			}
		};

	} // namespace internal

	namespace platform {
		extern void* thread_start(internal::BaseTask* task);
		extern void  thread_join(void* thread);
	} // namespace platform

	template<typename T, typename Context>
	static void parallel_for(T* entries, int nentries, Context* context,
		void (*func)(int threadIndex, T*, int, Context*), int nthreads) {

		std::vector<void*> threads(nthreads);
		std::vector<internal::Task<T, Context> > tasks(nthreads);

		const int entriesPerThread = (nentries+nthreads-1)/nthreads;
		for (int ii = 0; ii < nthreads; ++ii) {
			tasks[ii].nentries = entriesPerThread;
			tasks[ii].entries = entries;
			tasks[ii].func = func;
			tasks[ii].context = context;
			tasks[ii].threadIndex = ii;

			if (tasks[ii].nentries >= nentries) {
				tasks[ii].nentries = nentries;
			}
			nentries -= tasks[ii].nentries;
			entries += tasks[ii].nentries;
		}

		for (int ii = 0; ii < nthreads; ++ii) {
			threads[ii] = platform::thread_start(&tasks[ii]);
		}

		// sync tasks
		for (int ii = 0; ii < nthreads; ++ii) {
			platform::thread_join(threads[ii]);
		}
	}

	template<typename T>
	static void parallel_for(T* entries, int nentries,
		void (*func)(int threadIndex, T*, int, void*), int nthreads) {
		parallel_for<T, void>(entries, nentries, 0, func, nthreads);
	}

} // namespace tinytask

#endif // HEADER__TINYTASK_H
