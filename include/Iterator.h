/*
	OpenLieroX

	general iterator interface

	created by Albert Zeyer on 27-05-2008
	code under LGPL
*/

#ifndef __OLX_ITERATOR_H__
#define __OLX_ITERATOR_H__

#include "Utils.h" // for Ref


template < typename _Obj >
class Iterator {
public:
	virtual ~Iterator() {}
	virtual Iterator* copy() const = 0;
	virtual bool isValid() = 0;
	virtual void next() = 0;
	virtual bool operator==(const Iterator& other) const = 0;
	bool operator!=(const Iterator& other) const { return ! ((*this) == other); }
	virtual _Obj& get() = 0; // this has to return a valid obj if valid == true
	_Obj* operator->() { return &get(); }

	typedef ::Ref< Iterator > Ref;
};

#endif
