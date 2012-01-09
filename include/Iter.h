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
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "Ref.h"
#include "Functors.h"
#include "CodeAttributes.h"


template < typename _Obj >
class Iterator {
public:	
	virtual ~Iterator() {}
	virtual Iterator* copy() const = 0;
	virtual bool isValid() = 0;
	virtual void reset() = 0;
	virtual void next() = 0;
	virtual void nextn(size_t num) { while(num > 0) { next(); if(!isValid()) return; --num; } }
	virtual size_t size() {
		Ref i(copy());
		i->reset();
		size_t c = 0;
		for(; i->isValid(); i->next())
			++c;
		return c;
	}
	
	virtual bool operator==(const Iterator& other) const = 0;
	bool operator!=(const Iterator& other) const { return ! ((*this) == other); }
	
	virtual _Obj get() = 0; // this has to return a valid obj if valid == true
	//_Obj* operator->() { return &get(); }
	_Obj tryGet(_Obj zero = _Obj()) { if(isValid()) return get(); return zero; }

	typedef ::Ref< Iterator > Ref;
	typedef _Obj value_type;	
};

#define for_each_iterator( t, el, s )	for( ::Ref< Iterator<t> > el = GetIterator(s); el->isValid(); el->next() )

template<typename _STLT, typename _T>
class STLIteratorBase : public Iterator<_T> {
protected:
	typename _STLT::iterator i;
	_STLT& obj;
public:
	STLIteratorBase(_STLT& o) : i(o.begin()), obj(o) {}
	STLIteratorBase(const STLIteratorBase& it) : i(it.i), obj(it.obj) {}	
	virtual bool isValid() { return i != obj.end(); }
	virtual void reset() { i = obj.begin(); }
	virtual void next() { ++i; }
	virtual size_t size() { return obj.size(); }
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



template < typename _Type, typename _SpecificInitFunctor >
class SmartPointer;

template<typename _STLT, typename _T>
struct STLIteratorToPtr : Iterator<_T> {
	typename _STLT::value_type::iterator i;
	_STLT obj;
	STLIteratorToPtr(const _STLT& o) : i(o->begin()), obj(o) {}
	virtual ~STLIteratorToPtr() {}
	virtual Iterator<_T>* copy() const { return new STLIteratorToPtr(*this); }
	virtual bool isValid() { return i != obj->end(); }
	virtual void reset() { i = obj->begin(); }
	virtual void next() { ++i; }
	virtual size_t size() { return obj->size(); }
	virtual bool operator==(const Iterator<_T>& other) const { const STLIteratorToPtr* ot = dynamic_cast< const STLIteratorToPtr* > (&other); return (ot != NULL) && (i == ot->i) && (obj.get() == ot->obj.get()); }
	virtual _T get() { return * i; }
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
	virtual void reset() { i = 0; }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual size_t size() { return len; }
	virtual bool operator==(const Iterator<_T*>& other) const { const CArrayIterator* ot = dynamic_cast< const CArrayIterator* > (&other); return ot && ot->array == array && ot->i == i; }
	virtual _T* get() { return &array[i]; }
};

template<typename _T>
class CArrayConstIterator : public Iterator<_T> {
private:
	unsigned long i, len;
	_T* array;
public:
	CArrayConstIterator(_T* _arr, size_t _len) : i(0), len(_len), array(_arr) {}
	CArrayConstIterator(const CArray<_T>& a) : i(0), len(a.len), array(a.array) {}
	CArrayConstIterator(const CArrayConstIterator& it) : i(it.i), array(it.array) {}
	virtual Iterator<_T>* copy() const { return new CArrayConstIterator(*this); }
	virtual bool isValid() { return i < len; }
	virtual void reset() { i = 0; }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual size_t size() { return len; }
	virtual bool operator==(const Iterator<_T>& other) const { const CArrayConstIterator* ot = dynamic_cast< const CArrayConstIterator* > (&other); return ot && ot->array == array && ot->i == i; }
	virtual _T get() { return array[i]; }
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
	virtual void reset() { i = 0; }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual size_t size() { return str.size(); }
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
	virtual void reset() { i = 0; }
	virtual void next() { ++i; }
	virtual void nextn(size_t n) { i += n; }
	virtual size_t size() { return str.size(); }
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
typename Iterator<_T&>::Ref GetIteratorRef_second(std::map<_KT, _T>& s) { return new STL_MapIterator<std::map< _KT, _T >, _T& >(s); }

template< typename _T, typename _KT >
typename Iterator<_T>::Ref GetIterator_second(std::map<_KT, _T>& s) { return new STL_MapIterator<std::map< _KT, _T >, _T >(s); }

template< typename _T, typename _I >
typename Iterator< typename _T::value_type >::Ref GetIterator(const SmartPointer<_T,_I>& s) { return new STLIteratorToPtr<SmartPointer<_T,_I>,typename _T::value_type>(s); }

INLINE
Iterator<char&>::Ref GetIterator(std::string& s) { return new StringIterator(s); }

INLINE
Iterator<char>::Ref GetConstIterator(const std::string& s) { return new ConstStringIterator(s); }

template< typename _T >
typename Iterator<_T const&>::Ref GetConstIterator(std::vector<_T>& s) { return new STLIterator<std::vector<_T>,_T const&>(s); }

//template< typename _I, typename _T >
//typename Iterator< std::pair<_I,_T> >::Ref GetIterator(std::map<_I,_T>& s) { return s.begin(); }

template< typename _T >
typename Iterator<_T*>::Ref GetIterator(const CArray<_T>& s) { return new CArrayIterator<_T>(s); }

template< typename _T >
typename Iterator<_T>::Ref GetConstIterator(const CArray<_T>& s) { return new CArrayConstIterator<_T>(s); }

template< typename T >
typename Iterator<T>::Ref GetIterator(::Ref< Iterator<T> > i) { return i; }

template < typename T >
struct FilterIterator : public Iterator<T> {
	typename Iterator<T>::Ref baseIter;
	boost::function<bool(T)> predicate;
	FilterIterator(typename Iterator<T>::Ref _i, boost::function<bool(T)> _pred) : baseIter(_i), predicate(_pred) {}
	virtual Iterator<T>* copy() const { return new FilterIterator(*this); }
	virtual bool isValid() { return baseIter->isValid(); }
	virtual void reset() { baseIter->reset(); }
	virtual void next() {
		if(!isValid()) return;
		while(true) {
			baseIter->next();
			if(!isValid()) return;
			if(predicate(get())) return;
		}
	}
	virtual bool operator==(const Iterator<T>& other) const {
		if(FilterIterator* o2 = dynamic_cast<FilterIterator*>(&other))
			return baseIter.get() == o2->baseIter.get();
		return baseIter.get() == other;
	}
	virtual T get() { return baseIter->get(); }
};

template<typename T>
typename Iterator<T>::Ref GetFilterIterator(typename Iterator<T>::Ref i, boost::function<bool(T)> pred) {
	return new FilterIterator<T>(i, pred);
}

template < typename T, typename DataT >
struct ProxyIterator : public Iterator<T> {
	typename Iterator<T>::Ref baseIter;
	DataT data;
	ProxyIterator(typename Iterator<T>::Ref _i, const DataT& _data) : baseIter(_i), data(_data) {}
	virtual Iterator<T>* copy() const { return new ProxyIterator(*this); }
	virtual bool isValid() { return baseIter->isValid(); }
	virtual void reset() { baseIter->reset(); }
	virtual void next() { baseIter->next(); }
	virtual void nextn(size_t num) { baseIter->nextn(num); }
	virtual size_t size() { return baseIter->size(); }
	virtual bool operator==(const Iterator<T>& other) const { return baseIter.get() == other; }
	virtual T get() { return baseIter->get(); }
};

template<typename T>
typename Iterator<T>::Ref FullCopyIterator(::Ref<Iterator<T> > i) {
	boost::shared_ptr< std::vector<T> > copy( new std::vector<T>() );
	copy->reserve(i->size());
	for_each_iterator(T, x, i)
		copy->push_back(x->get());
	return ProxyIterator<T, boost::shared_ptr< std::vector<T> > >(GetIterator(*copy), copy);
}

#endif
