/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// 2D Vector class
// Created 20/11/01
// By Jason Boettcher
// enhanced by Albert Zeyer

#ifndef __CVEC_H__
#define __CVEC_H__

#include <math.h>


template<typename _T>
class VectorD2 {
public:
	// Constructor
	VectorD2() : x(0), y(0) {}
	VectorD2(_T _x, _T _y) : x(_x), y(_y) {}
	
	
	// Attributes
	_T	x, y;


public:
	// Methods

	template<typename _T2>
	VectorD2(const _T2& cp) {
		x = (_T)cp.x; y = (_T)cp.y;
	}

	inline float GetLength() const { return (float)sqrt((float)x*x + (float)y*y); }
	inline _T GetLength2() const { return x*x + y*y; };

	inline float GetAngle() const { return (float)atan2((float)y,(float)x); }

	inline VectorD2 Normalize() { return *this/GetLength(); }

	inline _T Scalar(const VectorD2 vec) { return x*vec.x + y*vec.y; }

	// Overloads
	inline VectorD2 operator*(const float scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	inline VectorD2 operator*(const int scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	inline VectorD2 operator*(const VectorD2 vec) const {
		// WARNING: this doesn't make any sense (in most 'mathematical' cases)
		return VectorD2(x*vec.x,y*vec.y);
	}
	inline VectorD2 operator/(const float scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	inline VectorD2 operator/(const int scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	inline VectorD2 operator+(const VectorD2 vec) const {
		return VectorD2(x+vec.x,y+vec.y);
	}
	inline VectorD2 operator-(const VectorD2 vec) const {
		return VectorD2(x-vec.x,y-vec.y);
	}
	inline VectorD2 operator-() const {
		return VectorD2(-x,-y);
	}
    inline VectorD2& operator+=(const VectorD2 vec) {
		x+=vec.x; y+=vec.y;
		return *this;
	}
	inline VectorD2& operator-=(const VectorD2 vec) {
		x-=vec.x; y-=vec.y;
		return *this;
	}
    inline VectorD2& operator*=(const float scalar) {
		x*=scalar; y*=scalar;
		return *this;
	}
    inline VectorD2& operator*=(const int scalar) {
		x*=scalar; y*=scalar;
		return *this;
	}

	template<typename _T2>
	inline bool operator<(const VectorD2<_T2> op) const {
		return ((y == op.y && (x < op.x))
				|| y < op.y);
	}
	
	template<typename _T2>
	inline bool operator==(const VectorD2<_T2> op) const {
		return (x==op.x && y==op.y);
	}

	template<typename _T2>
	inline bool operator!=(const VectorD2<_T2> op) const {
		return (x!=op.x || y!=op.y);
	}
	
	template<typename _T2>
	inline bool operator<=(const VectorD2<_T2> op) const {
		return ((*this < op) || (*this == op));
	}
				
};

template<typename _T>
class VectorD2__absolute_less {
public:
	VectorD2<_T> zero;
	VectorD2__absolute_less(VectorD2<_T> z = VectorD2<_T>(0,0)) : zero(z) {}
	
	inline bool operator()(const VectorD2<_T> v1, const VectorD2<_T> v2) const {
		return (v1-zero).GetLength2() < (v2-zero).GetLength2();
	}
};



typedef VectorD2<float> CVec;






#endif  //  __CVEC_H__
