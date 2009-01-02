/*
 *  Debug.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#include "Debug.h"

#ifdef __GLIBCXX__
#include <execinfo.h>
#include <stdio.h>
#endif

void DumpBacktrace(std::ostream& out) {
#ifdef __GLIBCXX__
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		printf("%s\n", strs[i]);
	}
	free(strs);
#endif
}

