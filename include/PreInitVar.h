/*
 *  PreInitVar.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.05.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__PREINITVAR_H__
#define __OLX__PREINITVAR_H__

#define PIVar(T, def) \
struct { \
	typedef T type; \
	template<typename _T> \
	struct Data { \
		_T var; \
		Data() : var(def) {} \
	}; \
	Data<T> data; \
	T& operator=(const T& d) { return data.var = d; } \
	operator const T&() const { return data.var; } \
	operator T&() { return data.var; } \
}

#endif
