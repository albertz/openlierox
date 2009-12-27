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
	FixedPointNumber(Sint64 n) : number(n * factor) {}
	FixedPointNumber(double n) : number(n * factor) {}
	FixedPointNumber& operator=(Sint64 n) { number = n * factor; return *this; }
	FixedPointNumber& operator=(double n) { number = n * factor; return *this; }
	
	FixedPointNumber& operator++/* prefix */(int) { number += factor; }
	
	bool operator==(const FixedPointNumber& n) const { return number == n.number; }
	bool operator!=(const FixedPointNumber& n) const { return !(*this == n); }
	
	
};

#endif
