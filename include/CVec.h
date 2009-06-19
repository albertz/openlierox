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


// 2D Vector / Matrix class
// Created 20/11/01
// By Jason Boettcher
// enhanced by Albert Zeyer

#ifndef __CVEC_H__
#define __CVEC_H__

#include <cmath>

template<typename _T>
struct VectorD2 {
	// Constructor
	VectorD2() : x(0), y(0) {}
	VectorD2(_T _x, _T _y) : x(_x), y(_y) {}
	
	
	// Attributes
	_T	x, y;


	// Methods

	template<typename _T2>
	VectorD2(const _T2& cp) {
		x = (_T)cp.x; y = (_T)cp.y;
	}

	float GetLength() const { return sqrtf((float)x*x + (float)y*y); }
	_T GetLength2() const { return x*x + y*y; };

	float GetAngle() const { return (float)atan2((float)y,(float)x); }

	VectorD2 Normalize() { return *this/GetLength(); }

	_T Scalar(const VectorD2& vec) const { return x*vec.x + y*vec.y; }

	VectorD2 orthogonal() const { return VectorD2(y, -x); }

	_T Cross(const VectorD2& oth) const { return x * oth.y - y * oth.x; }
	
	// Overloads
	VectorD2 operator*(const float scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	VectorD2 operator*(const int scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	/*
	VectorD2 operator*(const VectorD2& vec) const {
		// WARNING: this doesn't make any sense (in most 'mathematical' cases)
		// TODO: why is it here? I would expect dot product or cross product...
		return VectorD2(x*vec.x,y*vec.y);
	}
	*/
	VectorD2 operator/(const float scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	VectorD2 operator/(const int scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	VectorD2 operator+(const VectorD2& vec) const {
		return VectorD2(x+vec.x,y+vec.y);
	}
	VectorD2 operator-(const VectorD2& vec) const {
		return VectorD2(x-vec.x,y-vec.y);
	}
	VectorD2 operator-() const {
		return VectorD2(-x,-y);
	}
	VectorD2& operator+=(const VectorD2& vec) {
		x+=vec.x; y+=vec.y;
		return *this;
	}
	VectorD2& operator-=(const VectorD2& vec) {
		x-=vec.x; y-=vec.y;
		return *this;
	}
	VectorD2& operator*=(const float scalar) {
		x*=scalar; y*=scalar;
		return *this;
	}
	VectorD2& operator*=(const int scalar) {
		x*=scalar; y*=scalar;
		return *this;
	}

	template<typename _T2>
	bool operator<(const VectorD2<_T2> & op) const {
		return ((y == op.y && (x < op.x))
				|| y < op.y);
	}
	
	template<typename _T2>
	bool operator==(const VectorD2<_T2> & op) const {
		return (x==op.x && y==op.y);
	}

	template<typename _T2>
	bool operator!=(const VectorD2<_T2> & op) const {
		return (x!=op.x || y!=op.y);
	}
	
	template<typename _T2>
	bool operator<=(const VectorD2<_T2> & op) const {
		return ((*this < op) || (*this == op));
	}
				
};

template<typename _T>
struct VectorD2__absolute_less {
	VectorD2<_T> zero;
	VectorD2__absolute_less(VectorD2<_T> z = VectorD2<_T>(0,0)) : zero(z) {}
	
	bool operator()(const VectorD2<_T> v1, const VectorD2<_T> v2) const {
		return (v1-zero).GetLength2() < (v2-zero).GetLength2();
	}
};



typedef VectorD2<float> CVec;

template<typename _T>
struct MatrixD2 {
	VectorD2<_T> v1;
	VectorD2<_T> v2;
	
	MatrixD2() {}
	MatrixD2(_T f) { v1.x = f; v2.y = f; }
	MatrixD2(_T x1, _T y1, _T x2, _T y2) : v1(x1,y1), v2(x2,y2) {}
	MatrixD2(const VectorD2<_T>& _v1, const VectorD2<_T>& _v2) : v1(_v1), v2(_v2) {}
	static MatrixD2 Rotation(_T x, _T y) { return MatrixD2(x,y,-y,x); }
	
	template<typename _T2> bool operator==(const MatrixD2<_T2>& m) const { return v1 == m.v1 && v2 == m.v2; }
	template<typename _T2> bool operator!=(const MatrixD2<_T2>& m) const { return !(*this == m); }

	VectorD2<_T> operator*(const VectorD2<_T>& v) const {
		return VectorD2<_T>( v1.x * v.x + v2.x * v.y, v1.y * v.x + v2.y * v.y );
	}

	MatrixD2<_T> operator*(const MatrixD2<_T>& m) const {
		return MatrixD2<_T>( *this * m.v1, *this * m.v2 );
	}
	
	MatrixD2<_T>& operator*=(const MatrixD2<_T>& m) {
		return *this = *this * m;
	}
};



#endif  //  __CVEC_H__
