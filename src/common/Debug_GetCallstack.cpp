/*
 *  Debug_GetCallstack.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.12.
 *  code under LGPL
 *
 */

#include "Debug.h"
#include "Mutex.h"
#include "util/StaticVar.h"

// When implementing iterating over threads on Mac, this might be useful:
// http://llvm.org/viewvc/llvm-project/lldb/trunk/tools/darwin-threads/examine-threads.c?view=markup

// For getting the callback, maybe libunwind can be useful: http://www.nongnu.org/libunwind/


#ifndef HAVE_EXECINFO
#	if defined(__linux__)
#		define HAVE_EXECINFO 1
#	elif defined(__DARWIN_VERS_1050)
#		define HAVE_EXECINFO 1
#	else
#		define HAVE_EXECINFO 0
#	endif
#endif

#if HAVE_EXECINFO
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef HAVE_EXECINFO

#include <signal.h>
#include <pthread.h>

static StaticVar<Mutex> callstackMutex;
static ThreadId callingThread = 0;
static ThreadId targetThread = 0;
static void** threadCallstackBuffer = NULL;
static int threadCallstackBufferSize = 0;
static int threadCallstackCount = 0;

#define CALLSTACK_SIG SIGUSR2

static void _callstack_signal_handler(int) {	
	ThreadId myThread = (ThreadId)pthread_self();
	//notes << "_callstack_signal_handler, self: " << myThread << ", target: " << targetThread << ", caller: " << callingThread << endl;
	if(myThread != targetThread) return;
	
	threadCallstackCount = backtrace(threadCallstackBuffer, threadCallstackBufferSize);

	// continue calling thread
	pthread_kill((pthread_t)callingThread, CALLSTACK_SIG);
}

static void _setup_callstack_signal_handler() {
	struct sigaction sa;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = _callstack_signal_handler;
	sigaction(CALLSTACK_SIG, &sa, NULL);	
}

int GetCallstack(ThreadId threadId, void **buffer, int size) {
	if(threadId == 0 || threadId == (ThreadId)pthread_self())
		return backtrace(buffer, size);
	
	Mutex::ScopedLock lock(callstackMutex.get());
	callingThread = (ThreadId)pthread_self();
	targetThread = threadId;
	threadCallstackBuffer = buffer;
	threadCallstackBufferSize = size;
	
	_setup_callstack_signal_handler();

	// call _callstack_signal_handler in target thread
	if(pthread_kill((pthread_t)threadId, CALLSTACK_SIG) != 0)
		// something failed ...
		return 0;

	{
		sigset_t mask;
		sigfillset(&mask);
		sigdelset(&mask, CALLSTACK_SIG);

		// wait for CALLSTACK_SIG on this thread
		sigsuspend(&mask);
	}
	
	threadCallstackBuffer = NULL;
	threadCallstackBufferSize = 0;
	return threadCallstackCount;
}

#else // !HAVE_EXECINFO

// TODO: win32 implementation
// This might be useful: http://stackwalker.codeplex.com/SourceControl/changeset/view/66907#604665
// Esp, SuspendThread, ResumeThread, GetThreadContext, STACKFRAME64, ...

int GetCallstack(ThreadId threadId, void **buffer, int size) {
	return 0;
}

#endif
