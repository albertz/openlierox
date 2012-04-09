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
// Gusanos BaseVec merged in (thus a few double funcs).

#ifndef __CVEC_H__
#define __CVEC_H__

#include <string>
#include <cmath>
#include "CodeAttributes.h"
#include "util/angle.h"
#include "util/StringConv.h"


template<typename _T>
struct VectorD2 {
	typedef _T manip_t; // Gus BaseVec compatibility
	typedef _T Type;

	// Constructor
	VectorD2() : x(0), y(0) {}
	VectorD2(_T _x, _T _y) : x(_x), y(_y) {}

	template<typename _T2>
	VectorD2(const VectorD2<_T2>& cp) {
		x = (_T)cp.x; y = (_T)cp.y;
	}

	// clockwise direction. starting at top (12 o'clock), i.e. (0,-1)
	VectorD2(Angle angle, double length = 1)
	{
		double radians = angle.toRad();
		x = (_T)(sin(radians) * length);
		y = (_T)(-cos(radians) * length);
	}

	
	// Attributes
	_T	x, y;


	// Methods

	float GetLength() const { return sqrtf((float)x*x + (float)y*y); }
	_T GetLength2() const { return x*x + y*y; }

	float GetAngle() const { return (float)atan2((float)y,(float)x); }

	VectorD2 Normalize() { return *this/GetLength(); }

	_T Scalar(const VectorD2& vec) const { return x*vec.x + y*vec.y; }

	VectorD2 orthogonal() const { return VectorD2(y, -x); }

	_T Cross(const VectorD2& oth) const { return x * oth.y - y * oth.x; }
	
	_T dotProduct(VectorD2 const& A) const
	{
		return x * A.x + y * A.y;
	}

	_T perpDotProduct(VectorD2 const& A) const
	{
		return x * A.y - y * A.x;
	}

	VectorD2 perp() const
	{
		return VectorD2(-y, x);
	}

	double length() const
	{
		return sqrt((double)(x*x + y*y));
	}

	VectorD2 normal() const
	{
		double invLength = 1.0 / length();
		return VectorD2((_T)(invLength*x), (_T)(invLength*y));
	}

	_T lengthSqr() const
	{
		return x*x + y*y;
	}

	// equivalent to Angle-constructor.
	Angle getAngle() const
	{
		return Angle::fromRad( atan2(double(x), double(-y)) );
	}

	// Overloads
	template<typename _T2>
	VectorD2 operator*(_T2 scalar) const {
		return VectorD2(_T(x*scalar),_T(y*scalar));
	}
	VectorD2 multPairwise(_T _x, _T _y) const {
		return VectorD2(x*_x, y*_y);
	}
	template<typename _T2>
	VectorD2 operator/(_T2 scalar) const {
		return VectorD2(_T(x/scalar),_T(y/scalar));
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
		*this = *this + vec;
		return *this;
	}
	VectorD2& operator-=(const VectorD2& vec) {
		*this = *this - vec;
		return *this;
	}
	template<typename _T2>
	VectorD2& operator*=(_T2 scalar) {
		*this = *this * scalar;
		return *this;
	}
	template<typename _T2>
	VectorD2& operator/=(_T2 scalar) {
		*this = *this / scalar;
		return *this;
	}
	VectorD2& multPairwiseInplace(_T _x, _T _y) {
		x *= _x;
		y *= _y;
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

template<> VectorD2<int> from_string< VectorD2<int> >(const std::string& s, bool& fail);
template<> VectorD2<float> from_string< VectorD2<float> >(const std::string& s, bool& fail);
template<> INLINE std::string to_string< VectorD2<int> >(VectorD2<int> v) { return "(" + to_string(v.x) + "," + to_string(v.y) + ")"; }
template<> INLINE std::string to_string< VectorD2<float> >(VectorD2<float> v) { return "(" + to_string(v.x) + "," + to_string(v.y) + ")"; }


typedef VectorD2<float> CVec;
typedef VectorD2<float> Vec;
typedef VectorD2<int> IVec;


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
