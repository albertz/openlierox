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

#include "CVec.h"

// Constants
#define PI		3.14159265358979323846




// Routines
float	GetRandomNum(void);
int		GetRandomInt(int max);
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
CVec	GetRandomVec(void);
void	GetAngles(int yaw,CVec *forward, CVec *right);
float	VectorAngle(CVec vec1, CVec vec2);
float	VectorLength(CVec vec);
float   fastSQRT(float x);

#define SIGN(x) (((x) > 0)?1:(((x) < 0)?-1:0))
#define SQR(x) ((x)*(x))


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
		m.v1.x = cos(angle);
		m.v1.y = sin(angle);
		m.v2.x = -m.v1.y;
		m.v2.y = m.v1.x;
		return m;	
	}
	
	inline CVec operator()(const VectorD2<_T> v) const {
		return CVec(v.x*v1.x + v.y*v2.x, v.x*v1.y + v.y*v2.y);
	}
	inline SquareMatrix operator*(const SquareMatrix m) const {
		return SquareMatrix((*this)(m.v1), (*this)(m.v2));
	}
	inline SquareMatrix operator*(const float m) const {
		return SquareMatrix(v1*m, v2*m);
	}
	inline SquareMatrix operator*(const int m) const {
		return SquareMatrix(v1*m, v2*m);
	}
	inline SquareMatrix operator/(const float m) const {
		return SquareMatrix(v1/m, v2/m);
	}
	inline SquareMatrix operator/(const int m) const {
		return SquareMatrix(v1/m, v2/m);
	}
	inline _T det() {
		return v1.x*v2.y - v1.y*v2.x;
	}
	inline SquareMatrix inverse() {
		_T tdet = det();
		if(tdet == 0)
			return SquareMatrix();
		else
			return SquareMatrix(VectorD2<_T>(v2.y,-v1.y),VectorD2<_T>(-v2.x,v1.x))/tdet;
	}
};



#endif  //  __MATHLIB_H__
