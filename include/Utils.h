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
#include <vector>
#include <list>
#include <cassert>
#include <set>
#include <boost/function.hpp>
#include "MathLib.h"

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

template <typename T>
T& randomChoiceFrom(std::vector<T>& data) {
	assert(data.size() > 0);
	size_t i = (size_t)GetRandomInt(data.size() - 1);
	return data[i];
}

template <typename T>
int highestBit(T d) {
	for(int i = sizeof(T) * 8 - 1; i >= 0; --i)
		if((1 << i) <= d) return i + 1;
	return 0;
}


class DontCopyTag {
public:
	DontCopyTag() {}
private:
	DontCopyTag(const DontCopyTag&) { assert(false); }
	DontCopyTag& operator=(const DontCopyTag&) { assert(false); return *this; }
};


template<typename T>
std::vector<T> ListAsVector(const std::list<T>& l) {
	return std::vector<T>(l.begin(), l.end());
}

template<typename T> std::set<T> Set(T v1) { std::set<T> ret; ret.insert(v1); return ret; }
template<typename T> std::set<T> Set(T v1, T v2) { std::set<T> ret; ret.insert(v1); ret.insert(v2); return ret; }
template<typename T> std::set<T> Set(T v1, T v2, T v3) { std::set<T> ret; ret.insert(v1); ret.insert(v2); ret.insert(v3); return ret; }

template<typename RetT, typename ArgT>
RetT ifTrue(ArgT v, boost::function<RetT (ArgT)> f, RetT fallback = RetT()) {
	if(v) return f(v);
	return fallback;
}

#endif

