/*
 *  Debug_RaiseDebugger.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.12.
 *  code under LGPL
 *
 */

#include "Debug.h"
#include "CrashHandler.h"

#ifdef WIN32

#include <windows.h>

void RaiseDebugger() {
#ifdef DEBUG
#ifndef __MINGW32_VERSION
	// HINT: ignored when not in debugger
	// If it just does nothing then, remove the surrounding #ifdef DEBUG
	// I read about a Win32's IsDebuggerPresent() function, perhaps you should use that one here.
	__asm  { int 3 };
#endif
#endif
}

#else

#include <signal.h>

void RaiseDebugger() {
	if(AmIBeingDebugged()) {
		printf("I am being debugged, raising debugger ...\n");
		if(CrashHandler::get()) CrashHandler::get()->disable();
		// TODO: We need a way to set another ucontext here. (And that should be specified via a parameter
		// to RaiseDebugger().) E.g. when we use this function in the debugger thread, we want to set the
		// ucontext of the main loop thread.
		raise(SIGABRT);
		if(CrashHandler::get()) CrashHandler::get()->enable();
	} else
		printf("I am not being debugged, ignoring debugger raise.\n");
}

#endif
