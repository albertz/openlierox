/*
 *  Debug.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#ifndef __OLXDEBUG_H__
#define __OLXDEBUG_H__

#include <string>
#include "StringUtils.h"

void DumpCallstackPrintf();
void DumpCallstack(void (*PrintOutFct) (const std::string&));

struct Logger {
	int minCoutVerb;
	int minIngameConVerb;
	int minCallstackVerb;
	std::string prefix;
	std::string buffer;
	bool lastWasNewline;

	Logger(int o, int ingame, int callst, const std::string& p) : minCoutVerb(o), minIngameConVerb(ingame), minCallstackVerb(callst), prefix(p), lastWasNewline(true) {}
	Logger& operator<<(const std::string& msg) { buffer += msg; return *this; }
	Logger& operator<<(Logger& (*__pf)(Logger&)) { return (*__pf)(*this); }
	template<typename _T> Logger& operator<<(_T v) { return operator<<(to_string(v)); }
	Logger& flush();
	
	// deprecated, only for easier find/replace with printf
	void operator()(const std::string& str) { (*this) << str; flush(); }
};

inline Logger& endl(Logger& __os) { return (__os << "\n").flush(); }
inline Logger& flush(Logger& __os) { return __os.flush(); }


extern Logger notes;
extern Logger hints;
extern Logger warnings;
extern Logger errors;

#endif
