//
//  WeakRef.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 17.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_WeakRef_h
#define OpenLieroX_WeakRef_h

#include <boost/shared_ptr.hpp>

/*
This is basically `shared_ptr<T*>`, wrapped with some common functions.

This can be used as a base for a weak-pointer, but you need to do the logic
of resetting it somewhere else - it's not integraded automatically to `T`.

An example is `BaseObject` which has an attribute `WeakRef<BaseObject> thisRef`.
When you need a `WeakRef` to it, you would copy it from `thisRef`.
`BaseObject` will automatically reset it when it gets deleted.
*/
template<typename T>
struct WeakRef {
	typedef boost::shared_ptr<T*> SharedT;
	SharedT ref;
	
	WeakRef() {}
	WeakRef(const SharedT& _ref)
	: ref(_ref) {}
	
	T* get() const { return *ref; }
	void set(const SharedT& _ref) { ref = _ref; }
	void set(T* _ref) { ref = SharedT(new T*(_ref)); }
	void overwriteShared(T* _ref) { *ref = _ref; }
	
	operator bool() const { return ref && *ref; }
	bool operator==(const WeakRef& other) const { return ref == other.ref; }
	bool operator<(const WeakRef& other) const { return ref < other.ref; }
};

#endif
