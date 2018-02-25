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


// Mathematics Library
// Created 20/12/01
// Jason Boettcher
// Albert Zeyer


#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#include <cmath>

#include "CVec.h"


// Constants
#define PI		3.14159265358979323846


const float	D2R(1.745329e-2f); // degrees to radians
const float	R2D(5.729578e+1f); // radians to degrees

#define DEG2RAD(a)  ((a) * D2R)
#define RAD2DEG(a)  ((a) * R2D)

// Routines
float	GetRandomNum(); // get a random float from [-1,1] 
float	GetRandomPosNum(); // get a random float from [0,1]
int		GetRandomInt(int max); // get a random int from [0,max] 
int		Round(float x);

/*#ifdef _MSC_VER
float	cos(float _v)  {return cosf(_v); }
float	sin(float _v)  {return sinf(_v); }
float	tan(float _v)  {return tanf(_v); }
float	atan(float _v)  {return atanf(_v); }
float	sqrt(float _v)  {return sqrtf(_v); }
#endif*/


float	CalculateDistance(CVec p1, CVec p2);
float	NormalizeVector(CVec *vec);
CVec	GetRandomVec();
void	GetVecsFromAngle(float yaw,CVec *forward, CVec *right);
CVec	GetVecFromAngle(float yaw);
float	VectorAngle(CVec vec1, CVec vec2);
float	VectorLength(CVec vec);
float   fastSQRT(float x);

#define SIGN(x) (((x) > 0)?1:(((x) < 0)?-1:0))
#define SQR(x) ((x)*(x))

#undef MIN
#undef MAX

template <typename T> T MIN(T a, T b) { return a < b ? a : b; }
template <typename T> T MAX(T a, T b) { return a > b ? a : b; }
inline unsigned long MIN(unsigned long a, unsigned int b) { return a < b ? a : b; }
template <typename T> T CLAMP(const T& num, const T& lower_bound, const T& upper_bound) {
	return num < lower_bound ? lower_bound : (num > upper_bound ? upper_bound : num); }
template <typename T> int CLAMP_DIRECT(T& num, const T& lower_bound, const T& upper_bound) {
	if(num < lower_bound) {
		num = lower_bound;
		return -1;
	} else if(num > upper_bound) {
	 	num = upper_bound;
	 	return 1;
	} else return 0;
}
template <typename T> void REDUCE_CONST(T& v, const T& red_const) {
	if(v > 0) v = MAX((T)0, v - red_const); else if(v < 0) v = MIN((T)0, v + red_const); }
template <typename T> void RESET_SMALL(T& v, const T& limit) {
	if((v > 0 && v < limit) || (v < 0 && v > -limit)) v = 0; }

template <typename T> void MOD(T& a, const T& b) { a %= b; if(a < 0) a += b; }
template <typename T> void FMOD(T& a, const T& b) { a -= b * floor(a / b); if(a < 0) a += b; }


template<typename _T>
class SquareMatrix {
public:
	VectorD2<_T> v1, v2;
	SquareMatrix(VectorD2<_T> _v1 = VectorD2<_T>(0,0), VectorD2<_T> _v2 = VectorD2<_T>(0,0)) {
		v1 = _v1; v2 = _v2;
	}

	static SquareMatrix Identity() {
		 return SquareMatrix(VectorD2<_T>(1,0), VectorD2<_T>(0,1));
	}

	static SquareMatrix RotateMatrix(float angle) {
		SquareMatrix m;
		m.v1.x = cosf(angle);
		m.v1.y = sinf(angle);
		m.v2.x = -m.v1.y;
		m.v2.y = m.v1.x;
		return m;
	}

	CVec operator()(const VectorD2<_T>& v) const {
		return CVec(v.x*v1.x + v.y*v2.x, v.x*v1.y + v.y*v2.y);
	}
	SquareMatrix operator*(const SquareMatrix& m) const {
		return SquareMatrix((*this)(m.v1), (*this)(m.v2));
	}
	SquareMatrix operator*(const float m) const {
		return SquareMatrix(v1*m, v2*m);
	}
	SquareMatrix operator*(const int m) const {
		return SquareMatrix(v1*m, v2*m);
	}
	SquareMatrix operator/(const float m) const {
		return SquareMatrix(v1/m, v2/m);
	}
	SquareMatrix operator/(const int m) const {
		return SquareMatrix(v1/m, v2/m);
	}
	_T det() const {
		return v1.x*v2.y - v1.y*v2.x;
	}
	SquareMatrix inverse() const {
		_T tdet = det();
		if(tdet == 0)
			return SquareMatrix();
		else
			return SquareMatrix(VectorD2<_T>(v2.y,-v1.y),VectorD2<_T>(-v2.x,v1.x))/tdet;
	}

	// v1 is the upper-left, v2 the right-bottom
	bool isInDefinedArea(const VectorD2<_T>& p) const {
		return v1.x <= p.x && p.x <= v2.x && v1.y <= p.y && p.y <= v2.y;
	}

	VectorD2<_T> getCenter() const {
		return (v1 + v2) / 2;
	}

	SquareMatrix getInsersectionWithArea(const SquareMatrix& a) const {
		return SquareMatrix(
			VectorD2<_T>( MAX(a.v1.x, v1.x), MAX(a.v1.y, v1.y) ),
			VectorD2<_T>( MIN(a.v2.x, v2.x), MIN(a.v2.y, v2.y) )
			);
	}

};


class Parabola  {
public:
	float a, b, c; // a*x^2 + b*x + c
public:
	Parabola() : a(0), b(0), c(0) {}
	Parabola(float pa, float pb, float pc) : a(pa), b(pb), c(pc) {}
	Parabola(const Parabola& p) { a = p.a; b = p.b; c = p.c; }
	Parabola(CVec p1, CVec p2, CVec p3);
	Parabola(CVec p1, float angleP1, CVec p2);

	inline bool operator==(const Parabola& p) const {
		return (a == p.a && b == p.b && c == p.c);
	}

	float getLength(CVec p1, CVec p2);
	float getLength(float pa, float pb);
	bool isPointAtParabola(CVec p1)  {
		return (p1.y == (a * p1.x * p1.x ) + (b * p1.x) + c);
	}
};


// Very fast 32-bit PRNG: https://en.wikipedia.org/wiki/Xorshift
// The parameter must be initialized to non-zero, otherwise it will return zero every time
static inline uint32_t xorshift32Random(uint32_t x)
{
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}


#endif  //  __MATHLIB_H__
