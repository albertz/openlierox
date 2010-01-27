/*
 *  FixedPointNumber.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 27.12.09.
 *  code under LGPL
 *
 */

#ifndef __OLX_FIXEDPOINTNUMBER_H___
#define __OLX_FIXEDPOINTNUMBER_H___

#include <SDL.h> // Uint64

template< Uint64 _factor >
struct FixedPointNumber {
	static const Uint64 factor = _factor;
	Sint64 number;
	
	FixedPointNumber() : number(0) {}
	FixedPointNumber(int n) : number(n * factor) {}
	FixedPointNumber(Sint64 n) : number(n * factor) {}
	FixedPointNumber(double n) : number(Sint64(n * factor)) {}
	
	FixedPointNumber& operator++/* prefix */(int) { number += factor; return *this; }
	FixedPointNumber& operator--/* prefix */(int) { number -= factor; return *this; }
	FixedPointNumber& operator-=(const FixedPointNumber& n) { number -= n.number; return *this; }
	
	bool operator==(const FixedPointNumber& n) const { return number == n.number; }
	bool operator!=(const FixedPointNumber& n) const { return !(*this == n); }
	bool operator<(const FixedPointNumber& n) const { return number < n.number; }
	bool operator>(const FixedPointNumber& n) const { return number > n.number; }
	
	int asInt() { return (int)(number / factor); }
	
};

#endif
