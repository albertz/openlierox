// OpenLieroX
// code under LGPL
// created 2012-02-27, by Albert Zeyer

#ifndef OLX_STATICVAR_H
#define OLX_STATICVAR_H

#include "PODForClass.h"

// This is a POD type.
// It can be used for static variables
// when you must ensure that there are
// initialized when you use them, no
// matter how early that is.
template<typename T>
struct StaticVar {
	bool inited;
	PODForClass<T> var;

	void init() {
		if(inited) return;
		var.init();
		inited = true;
	}

	T& get() {
		init();
		return *(T*)&var;
	}

	T* operator->() { return &get(); }

};

#endif // STATICVAR_H
