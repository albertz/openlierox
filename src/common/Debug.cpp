/*
 *  Debug.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#include "Debug.h"

void DumpBacktrace(std::ostream& out) {
#ifdef __GLIBC__
	void *buffer[100];
	int nptrs = backtrace(buffer, 100);
	printf("backtrace() returned %d addresses\n", nptrs);
	backtrace_symbols_fd(buffer, nptrs, fileno(stdout));
	
#endif
}

