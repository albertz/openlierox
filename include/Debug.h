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
void DumpCallstack(void (*LineOutFct) (const std::string&));

struct Logger {
	int minIngameConVerb;
	int minCoutVerb;
	int minCallstackVerb;
	std::string prefix;
	std::string buffer;

	Logger(int ingame, int o, int callst, const std::string& p) : minIngameConVerb(ingame), minCoutVerb(o), minCallstackVerb(callst), prefix(p) {}
	Logger& operator<<(const std::string& msg) { buffer += msg; return *this; }
	Logger& operator<<(Logger& (*__pf)(Logger&)) { return (*__pf)(*this); }
	template<typename _T> Logger& operator<<(_T v) { return operator<<(to_string(v)); }
	Logger& flush();
};

inline Logger& endl(Logger& __os) { return (__os << "\n").flush(); }


extern Logger notes;
extern Logger hints;
extern Logger warnings;
extern Logger errors;

#endif
