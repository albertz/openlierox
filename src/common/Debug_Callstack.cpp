/*
 *  Debug_Callstack.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.12.
 *  code under LGPL
 *
 */

#include "Debug.h"
#include "util/macros.h"
#include <vector>
#include <string>

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

#if HASBFD

std::vector<std::string> backtrace_symbols_str(void *const *buffer, int size);

void DumpCallstackPrintf(void* callpnt) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	if(callpnt != NULL && framesC > 3) callstack[3] = callpnt; // expected to be called from signal handler
	std::vector<std::string> strs = backtrace_symbols_str(callstack, framesC);
	foreach(s, strs)
	printf("%s\n", s->c_str());
}

void DumpCallstack(const PrintOutFct& printer) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printer.print("DumpCallstack: " + itoa(framesC) + " addresses:");
	std::vector<std::string> strs = backtrace_symbols_str(callstack, framesC);
	foreach(s, strs)
	printer.print(" " + *s + "\n");
}

#else

void DumpCallstackPrintf(void* callpnt) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	if(callpnt != NULL && framesC > 3) callstack[3] = callpnt; // expected to be called from signal handler
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printf("%s\n", strs[i]);
		else
			break;
	}
	free(strs);
}

void DumpCallstack(const PrintOutFct& printer) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printer.print("DumpCallstack: " + itoa(framesC) + " addresses:");
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printer.print(std::string(" ") + strs[i] + "\n");
		else
			break;
	}
	free(strs);
}

#endif // HASBFD

#elif defined(WIN32)

#include "StackWalker.h"  // Call Luke Stackwalker for help

typedef void (*DbgPrintOutFct) (const std::string&);

// Override the default stackwalker with our own print functions
class PrintStackWalker : public StackWalker  {
private:
	DbgPrintOutFct m_print;
	PrintOutFct *m_print2;
	
public:
	PrintStackWalker(DbgPrintOutFct fct = NULL) : StackWalker(RetrieveVerbose) { m_print = fct; m_print2 = NULL; }
	PrintStackWalker(PrintOutFct *fct) : StackWalker(RetrieveVerbose) { m_print2 = fct; m_print = NULL; }
	void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
	{
		
	}
	
	void OnOutput(LPCSTR szText)  
	{
		if (m_print)
			m_print(std::string(szText));
		else if (m_print2)
			m_print2->print(std::string(szText));
		else
			printf(szText);
		
		StackWalker::OnOutput(szText);
	}
};

void DumpCallstackPrintf(void* callpnt) 
{
	PrintStackWalker sw;
	sw.ShowCallstack();
	
}
void DumpCallstack(void (*LineOutFct) (const std::string&)) 
{  
	PrintStackWalker sw(LineOutFct);
	sw.ShowCallstack();
}

void DumpCallstack(const PrintOutFct& printer) {
	PrintStackWalker sw(const_cast<PrintOutFct *>(&printer));
	sw.ShowCallstack();
}

#else

#warning No DumpCallstack implementation for this arch/sys

void DumpCallstackPrintf(void* callpnt) {
	printf("DumpCallstackPrintf not implemented\n");
}

void DumpCallstack(const PrintOutFct& printer) {
	printf("DumpCallstack not implemented\n");
}

#endif
