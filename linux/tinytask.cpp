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

#include "../tinytask.h"

#include <pthread.h>
#include <stdint.h>

#define TINYSTACK_ALLOCATE_PTHREAD 0

#if !TINYSTACK_ALLOCATE_PTHREAD
static_assert(sizeof(pthread_t) <= sizeof(void*), "If this assert fails you must define TINYSTACK_ALLOCATE_PTHREAD to 1");
#endif // !TINYSTACK_ALLOCATE_PTHREAD

using namespace tinytask;
using namespace tinytask::internal;

BaseTask::~BaseTask() {
}

static void* thread_entry(void* threadContext) {
	BaseTask* t = static_cast<BaseTask*>(threadContext);
	t->run();
	return 0;
}

void* tinytask::platform::thread_start(BaseTask* task) {
#if TINYSTACK_ALLOCATE_PTHREAD
		pthread_t* pt = new pthread_t;
		pthread_create(pt, NULL, thread_entry, task);
		return pt;
#else
		pthread_t pt;
		pthread_create(&pt, NULL, thread_entry, task);
		return reinterpret_cast<void*>(static_cast<uintptr_t>(pt));
#endif // TINYSTACK_ALLOCATE_PTHREAD
}

void tinytask::platform::thread_join(void* thread) {
#if TINYSTACK_ALLOCATE_PTHREAD
	pthread_t* pt = static_cast<pthread_t*>(thread);
	pthread_join(*pt, 0);
	delete pt;
#else
	pthread_t pt = static_cast<pthread_t>(reinterpret_cast<uintptr_t>(thread));
	pthread_join(pt, 0);
#endif // TINYSTACK_ALLOCATE_PTHREAD
}
