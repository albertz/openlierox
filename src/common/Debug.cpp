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

void DumpCallstackPrintf() {
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

void DumpCallstack(void (*LineOutFct) (const std::string&)) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	(*LineOutFct) ("DumpCallstack: " + itoa(framesC) + " addresses:");
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			(*LineOutFct) (std::string(" ") + strs[i]);
		else
			break;
	}
	free(strs);
}

#else

void DumpCallstackPrintf() { printf("DumpCallstackPrintf() not implemented in this version.\n"); }
void DumpCallstack(void (*LineOutFct) (const std::string&)) { (*LineOutFct) ("DumpCallstack() not implemented in this version."); }

#endif

Logger notes(1,2,1000, "n: ");
Logger hints(0,1,100, "H: ");
Logger warnings(0,0,10, "W: ");
Logger errors(-1,-1,1, "E: ");

#include <iostream>
#include <sstream>
#include "Options.h"
#include "console.h"
#include "StringUtils.h"

using namespace std;

static void PrettyPrint(const std::string& prefix, const std::string& buf, void (*LineOutFct) (const std::string&)) {
	std::string::const_iterator it = buf.begin();
	while(true) {
		std::string tmp = ReadUntil(buf, it, '\n');		
		if(it == buf.end()) {
			if(tmp != "") (*LineOutFct) (prefix + tmp);
			break;
		}
		++it;
		(*LineOutFct) (prefix + tmp);
	}
}

static void CoutPrintLn(const std::string& str) {
	cout << str << "\n";
}

template<int col> void ConPrintLn(const std::string& str) {
	Con_AddText(col, str, false);
}

Logger& Logger::flush() {
	if(!tLXOptions || tLXOptions->iVerbosity >= minCoutVerb) {
		PrettyPrint(prefix, buffer, CoutPrintLn);
		cout.flush();
	}
	if(tLXOptions && tLXOptions->iVerbosity >= minCallstackVerb) {
		DumpCallstackPrintf();
	}
	if(tLXOptions && Con_IsInited() && tLXOptions->iVerbosity >= minIngameConVerb) {
		// the check is a bit hacky (see Con_AddText) but I really dont want to overcomplicate this
		if(!strStartsWith(buffer, "Ingame console: ")) {
			// we are not safing explicitly a color in the Logger, thus we try to assume a good color from the verbosity level
			if(minIngameConVerb < 0)
				PrettyPrint(prefix, buffer, ConPrintLn<CNC_ERROR>);
			else if(minIngameConVerb == 0)
				PrettyPrint(prefix, buffer, ConPrintLn<CNC_WARNING>);
			else if(minIngameConVerb == 1)
				PrettyPrint(prefix, buffer, ConPrintLn<CNC_NOTIFY>);
			else if(minIngameConVerb < 5)
				PrettyPrint(prefix, buffer, ConPrintLn<CNC_NORMAL>);
			else // >=5
				PrettyPrint(prefix, buffer, ConPrintLn<CNC_DEV>);
		}
		if(tLXOptions->iVerbosity >= minCallstackVerb) {
			DumpCallstack(ConPrintLn<CNC_DEV>);
		}
	}
	buffer = "";
	return *this;
}
