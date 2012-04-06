/*
 *  Debug_GetCallstack.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.12.
 *  code under LGPL
 *
 */

#include "Debug.h"

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

int GetCallstack(ThreadId threadId, void **buffer, int size) {
#ifdef HAVE_EXECINFO
	if(threadId == 0)
		return backtrace(buffer, size);
#endif

	// stub
	return 0;
}
