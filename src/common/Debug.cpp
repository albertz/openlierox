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

static void PrettyPrintLn(const std::string& prefix, const std::string& msg, std::ostream& out) {
	out << prefix << msg << "\n";
}

static void PrettyPrint(const std::string& prefix, const std::string& buf, std::ostream& out) {
	std::string::const_iterator it = buf.begin();
	while(true) {
		std::string tmp = ReadUntil(buf, it, '\n');		
		if(it == buf.end()) {
			if(tmp != "") PrettyPrintLn(prefix, tmp, out);
			break;
		}
		++it;
		PrettyPrintLn(prefix, tmp, out);
	}
	out.flush();
}

Logger& Logger::flush() {
	PrettyPrint(prefix, buffer, cout);
	buffer = "";
	return *this;
}
