/*
	OpenLieroX

	file utils

	code under LGPL
	created 30-04-2009
*/

#ifndef __OLX__FILE_UTILS_H__
#define __OLX__FILE_UTILS_H__

#include <cstdio>

template<size_t C>
bool fread_fixedwidthstr(std::string& str, FILE* fp) {
	char tmp[C + 1]; tmp[0] = 0; tmp[C] = 0;
	if(fread(tmp, sizeof(char), C, fp) < C)
		return false;
	str = tmp; // we are handling tmp as a C-str
	return true;
}


#endif
