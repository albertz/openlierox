/*
 *  Debug.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#include "Debug.h"

#if defined(__GLIBCXX__) || defined(__GLIBC__) || !defined(WIN32)

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void DumpCallstack() {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printf("%s\n", strs[i]);
		else
			break;
	}
	free(strs);
}

#else

void DumpCallstack() {}

#endif

Logger notes(0,1000, "n: ");
Logger hints(0,100, "H: ");
Logger warnings(0,10, "W: ");
Logger errors(0,1, "E: ");

#include <iostream>

using namespace std;

std::string name;

// simple dummy for now
Logger& Logger::flush() {
	cout << buffer;
	buffer = "";
	return *this;
}
