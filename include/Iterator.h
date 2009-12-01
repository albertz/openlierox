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
#include <vector>
#include <list>
#include <string>
#include "Ref.h"


template < typename _Obj >
class Iterator {
public:	
	virtual ~Iterator() {}
	virtual Iterator* copy() const = 0;
	virtual bool isValid() = 0;
	virtual void next() = 0;
	virtual void nextn(size_t num) { while(num > 0) { next(); if(!isValid()) return; --num; } }
	
	virtual bool operator==(const Iterator& other) const = 0;
	bool operator!=(const Iterator& other) const { return ! ((*this) == other); }
	
	virtual _Obj get() = 0; // this has to return a valid obj if valid == true
	//_Obj* operator->() { return &get(); }

	typedef ::Ref< Iterator > Ref;
	typedef _Obj value_type;	
};

template<typename _STLT, typename _T>
class STLIteratorBase : public Iterator<_T> {
protected:
	typename _STLT::iterator i;
	_STLT& obj;
public:
	STLIteratorBase(_STLT& o) : i(o.begin()), obj(o) {}
	STLIteratorBase(const STLIteratorBase& it) : i(it.i), obj(it.obj) {}	
	virtual bool isValid() { return i != obj.end(); }
	virtual void next() { ++i; }
	virtual bool operator==(const Iterator<_T>& other) const { const STLIteratorBase* ot = dynamic_cast< const STLIteratorBase* > (&other); return ot && &ot->obj == &obj && ot->i == i; }
};


template<typename _STLT, typename _T>
class STLIterator : public STLIteratorBase<_STLT, _T> {
public:
	STLIterator(_STLT& o) : STLIteratorBase<_STLT, _T>(o) {}
	STLIterator(const STLIterator& it) : STLIteratorBase<_STLT, _T>(it) {}	
	virtual Iterator<_T>* copy() const { return new STLIterator(*this); }
	virtual bool operator==(const Iterator<_T>& other) const { const STLIterator* ot = dynamic_cast< const STLIterator* > (&other); return ot != NULL && STLIteratorBase<_STLT, _T> :: operator == ( other ); }
	virtual _T get() { return * STLIteratorBase<_STLT, _T> :: i; }
};

template<typename _STLT, typename _T>
class STL_MapIterator : public STLIteratorBase<_STLT, _T> {
public:
	STL_MapIterator(_STLT& o) : STLIteratorBase<_STLT, _T> (o) {}
	STL_MapIterator(const STL_MapIterator& it) : STLIteratorBase<_STLT, _T> (it) {}	
	virtual Iterator<_T>* copy() const { return new STL_MapIterator(*this); }
	virtual bool operator==(const Iterator<_T>& other) const { const STL_MapIterator* ot = dynamic_cast< const STL_MapIterator* > (&other); return ot != NULL && STLIteratorBase<_STLT, _T> :: operator == ( other ); }
	virtual _T get() { return STLIteratorBase<_STLT, _T> :: i -> second; }
};

template<typename _T>
struct CArray {
	_T* array;
	unsigned long len;
	CArray(_T* a, unsigned long l) : array(a), len(l) {}
	typedef _T value_type;
};

template<typename _T>
CArray<_T> Array(_T* a, unsigned long l) { return CArray<_T>(a,l); }

template<typename _T>
class CArrayIterator : public Iterator<_T*> {
private:
	unsigned long i, len;
	_T* array;	
public:
	CArrayIterator(_T* _arr, size_t _len) : i(0), len(_len), array(_arr) {}		
	CArrayIterator(const CArray<_T>& a) : i(0), len(a.len), array(a.array) {}	
	CArrayIterator(const CArrayIterator& it) : i(it.i), array(it.array) {}	
	virtual Iterator<_T*>* copy() const { return new CArrayIterator(*this); }
	virtual bool isValid() { return i < len; }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual bool operator==(const Iterator<_T*>& other) const { const CArrayIterator* ot = dynamic_cast< const CArrayIterator* > (&other); return ot && ot->array == array && ot->i == i; }
	virtual _T* get() { return &array[i]; }
};

class StringIterator : public Iterator<char&> {
private:
	std::string& str;
	size_t i;
public:
	typedef StringIterator This;
	typedef char& _T;
	StringIterator(std::string& s) : str(s), i(0) {}
	virtual Iterator<_T>* copy() const { return new This(*this); }
	virtual bool isValid() { return i < str.size(); }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual bool operator==(const Iterator<_T>& other) const { const This* ot = dynamic_cast< const This* > (&other); return ot && &ot->str == &str && ot->i == i; }
	virtual _T get() { return str[i]; }
};

class ConstStringIterator : public Iterator<char> {
private:
	const std::string& str;
	size_t i;
public:
	typedef ConstStringIterator This;
	typedef char _T;
	ConstStringIterator(const std::string& s) : str(s), i(0) {}
	virtual Iterator<_T>* copy() const { return new This(*this); }
	virtual bool isValid() { return i < str.size(); }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual bool operator==(const Iterator<_T>& other) const { const This* ot = dynamic_cast< const This* > (&other); return ot && &ot->str == &str && ot->i == i; }
	virtual _T get() { return str[i]; }
};


template< typename __T, typename __C >
typename Iterator<__T>::Ref GetIterator(__C& s) { return s.iterator(); }

template< typename _T >
typename Iterator<_T>::Ref GetIterator(std::set<_T>& s) { return new STLIterator<std::set<_T>,_T>(s); }

template< typename _T >
typename Iterator<_T&>::Ref GetIterator(std::list<_T>& s) { return new STLIterator<std::list<_T>,_T&>(s); }

template< typename _T >
typename Iterator<_T&>::Ref GetIterator(std::vector<_T>& s) { return new STLIterator<std::vector<_T>,_T&>(s); }

template< typename _T, typename _KT >
typename Iterator<_T&>::Ref GetIterator(std::map<_KT, _T>& s) { return new STL_MapIterator<std::map< _KT, _T >, _T& >(s); }

inline
Iterator<char&>::Ref GetIterator(std::string& s) { return new StringIterator(s); }

inline
Iterator<char>::Ref GetConstIterator(std::string& s) { return new ConstStringIterator(s); }

template< typename _T >
typename Iterator<_T const&>::Ref GetConstIterator(std::vector<_T>& s) { return new STLIterator<std::vector<_T>,_T const&>(s); }

//template< typename _I, typename _T >
//typename Iterator< std::pair<_I,_T> >::Ref GetIterator(std::map<_I,_T>& s) { return s.begin(); }

template< typename _T >
typename Iterator<_T*>::Ref GetIterator(const CArray<_T>& s) { return new CArrayIterator<_T>(s); }

#ifndef foreach
#define foreach( t, el, s )	for( Iterator<t>::Ref el = GetIterator(s); el->isValid(); el->next() )
#endif

#endif
