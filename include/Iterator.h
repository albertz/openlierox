/*
	OpenLieroX

	general iterator interface

	created by Albert Zeyer on 27-05-2008
	code under LGPL
*/

#ifndef __OLX_ITERATOR_H__
#define __OLX_ITERATOR_H__

#include <set>
#include <map>
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
	virtual _Obj get() = 0; // this has to return a valid obj if valid == true
	_Obj* operator->() { return &get(); }

	typedef ::Ref< Iterator > Ref;
};


template<typename _STLT, typename _T>
class STLIterator : public Iterator<_T> {
private:
	typename _STLT::iterator i;
	_STLT& obj;
public:
	STLIterator(_STLT& o) : obj(o) { i = o.begin(); }
	STLIterator(const STLIterator& it) : obj(it.obj), i(it.i) {}	
	virtual Iterator<_T>* copy() const { return new STLIterator(*this); }
	virtual bool isValid() { return i != obj.end(); }
	virtual void next() { ++i; }
	virtual bool operator==(const Iterator<_T>& other) const { const STLIterator* ot = dynamic_cast< const STLIterator* > (&other); return ot && ot->i == i; }
	virtual _T get() { return *i; }
};


template< typename _T, typename _S >
typename Iterator<_T>::Ref GetIterator(_S s) { return s.iterator(); }

template< typename _T >
typename Iterator<_T>::Ref GetIterator(std::set<_T>& s) { return new STLIterator<std::set<_T>,_T>(s); }

template< typename _I, typename _T >
typename Iterator< std::pair<_I,_T> >::Ref GetIterator(std::map<_I,_T>& s) { return s.begin(); }

#define foreach( t, el, s )	for( Iterator<t>::Ref el = GetIterator(s); el->isValid(); el->next() )

#endif
