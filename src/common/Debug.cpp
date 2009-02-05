/*
 *  Debug.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#include "Debug.h"

#if (defined(__GLIBCXX__) || defined(__GLIBC__) || !defined(WIN32)) && !defined(__MINGW32_VERSION)

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void DumpCallstackPrintf(void* callpnt) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	if(framesC > 3) callstack[3] = callpnt; // expected to be called from signal handler
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printf("%s\n", strs[i]);
		else
			break;
	}
	free(strs);
}

void DumpCallstack(void (*PrintOutFct) (const std::string&)) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	(*PrintOutFct) ("DumpCallstack: " + itoa(framesC) + " addresses:");
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			(*PrintOutFct) (std::string(" ") + strs[i] + "\n");
		else
			break;
	}
	free(strs);
}

#else

void DumpCallstackPrintf() { printf("DumpCallstackPrintf() not implemented in this version.\n"); }
void DumpCallstack(void (*LineOutFct) (const std::string&)) { (*LineOutFct) ("DumpCallstack() not implemented in this version."); }

#endif

Logger notes(0,2,1000, "n: ");
Logger hints(0,1,100, "H: ");
Logger warnings(0,0,10, "W: ");
Logger errors(-1,-1,1, "E: ");

#include <iostream>
#include <sstream>
#include <SDL_thread.h>
#include "Options.h"
#include "console.h"
#include "StringUtils.h"


Logger::Logger(int o, int ingame, int callst, const std::string& p)
: minCoutVerb(o), minIngameConVerb(ingame), minCallstackVerb(callst), prefix(p), lastWasNewline(true), mutex(NULL) {
	mutex = SDL_CreateMutex();
}

Logger::~Logger() {
	SDL_DestroyMutex(mutex); mutex = NULL;
}

void Logger::lock() {
	SDL_mutexP(mutex);
}

void Logger::unlock() {
	SDL_mutexV(mutex);
}

static void CoutPrint(const std::string& str) {
	std::cout << str;
}

template<int col> void ConPrint(const std::string& str) {
	// TODO: Con_AddText adds a line but we only want to add str
	std::string buf = str;
	if(buf.size() > 0 && buf[buf.size()-1] == '\n') buf.erase(buf.size()-1);
	Con_AddText(col, buf, false);
}

// true if last was newline
static bool logger_output(Logger& log, const std::string& buf) {
	bool ret = true;
	if(!tLXOptions || tLXOptions->iVerbosity >= log.minCoutVerb) {
		ret = PrettyPrint(log.prefix, buf, CoutPrint, log.lastWasNewline);
		std::cout.flush();
	}
	if(tLXOptions && tLXOptions->iVerbosity >= log.minCallstackVerb) {
		DumpCallstackPrintf();
	}
	if(tLXOptions && Con_IsInited() && tLXOptions->iVerbosity >= log.minIngameConVerb) {
		// the check is a bit hacky (see Con_AddText) but I really dont want to overcomplicate this
		if(!strStartsWith(buf, "Ingame console: ")) {
			// we are not safing explicitly a color in the Logger, thus we try to assume a good color from the verbosity level
			if(log.minIngameConVerb < 0)
				ret = PrettyPrint(log.prefix, buf, ConPrint<CNC_ERROR>, log.lastWasNewline);
			else if(log.minIngameConVerb == 0)
				ret = PrettyPrint(log.prefix, buf, ConPrint<CNC_WARNING>, log.lastWasNewline);
			else if(log.minIngameConVerb == 1)
				ret = PrettyPrint(log.prefix, buf, ConPrint<CNC_NOTIFY>, log.lastWasNewline);
			else if(log.minIngameConVerb < 5)
				ret = PrettyPrint(log.prefix, buf, ConPrint<CNC_NORMAL>, log.lastWasNewline);
			else // >=5
				ret = PrettyPrint(log.prefix, buf, ConPrint<CNC_DEV>, log.lastWasNewline);
		}
		if(tLXOptions->iVerbosity >= log.minCallstackVerb) {
			DumpCallstack(ConPrint<CNC_DEV>);
		}
	}
	return ret;
}

Logger& Logger::flush() {
	lock();
	lastWasNewline = logger_output(*this, buffer);
	buffer = "";
	unlock();
	return *this;
}
