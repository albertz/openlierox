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

#include "util/macros.h"

/* HACK: use _LINENAME, workaround for a buggy MSVC compiler (http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=360628)*/
#define PIVar(T, def) \
struct _LINENAME(__predef, __LINE__) { \
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
