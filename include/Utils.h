/*
	OpenLieroX

	various utilities

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h> // for size_t

template <typename _dst, typename _src>
bool isSameType(const _src& obj1, const _dst& obj2) {
	if(sizeof(_dst) < sizeof(_src)) return isSameType(obj2, obj1);
	return dynamic_cast<const _dst*>(&obj1) != NULL;
}


/*
	some very basic math functions
*/


template <class Iter> void SafeAdvance(Iter& it, size_t count, const Iter& end)  {
	for (size_t i=0; i < count && it != end; i++, it++)  {}
}

#endif

