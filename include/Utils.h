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

template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class iterator {
protected:
	_RandomAccessType& base;
	_PosType pos;
public:
	iterator(_RandomAccessType& b, _PosType p) : base(b), pos(p) {}
	static iterator end(_RandomAccessType& b) {
		return iterator<_RandomAccessType, _ValueType>(b, -1); }

	template<typename _ValueType2, typename _PosType2>
	void operator==(const iterator<_RandomAccessType, _ValueType2, _PosType2>& it) {
		return base == it->base && (pos == it->pos || MIN(pos,it->pos) >= base.size()); }
	
	void operator++() { pos++; }
	void operator--() { pos--; }
	_ValueType operator*() { return base[pos]; }
};

template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class reverse_iterator : public iterator<_RandomAccessType, _ValueType, _PosType> {
public:
	reverse_iterator(_RandomAccessType& b, _PosType p) : iterator<_RandomAccessType, _ValueType, _PosType>(b, p) {}
	void operator++() { this->pos--; }
	void operator--() { this->pos++; }
};

#endif
