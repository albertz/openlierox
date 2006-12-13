/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Mathematics Library
// Created 20/12/01
// Jason Boettcher


#ifndef __MATHLIB_H__
#define __MATHLIB_H__


// Constants
#define PI		3.14159265358979323846




// Routines
float	GetRandomNum(void);
int		GetRandomInt(int max);
int		Round(float x);

/*#ifdef WIN32
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


class SquareMatrix {
public:	
	CVec v1, v2;
	SquareMatrix(CVec _v1=CVec(0,0), CVec _v2=CVec(0,0)) {
		v1 = _v1; v2 = _v2;
	}
	
	static SquareMatrix Identity() {
		 return SquareMatrix(CVec(1,0),CVec(0,1));
	}
	
	static SquareMatrix RotateMatrix(float angle) {
		SquareMatrix m;
		m.v1.x = cos(angle);
		m.v1.y = sin(angle);
		m.v2.x = -m.v1.y;
		m.v2.y = m.v1.x;
		return m;	
	}
	
	inline CVec operator()(const CVec v) const {
		return CVec(v.x*v1.x + v.y*v2.x, v.x*v1.y + v.y*v2.y);
	}
	inline SquareMatrix operator*(const SquareMatrix m) const {
		return SquareMatrix((*this)(m.v1), (*this)(m.v2));
	}
	inline SquareMatrix operator*(const float m) const {
		return SquareMatrix(v1*m, v2*m);
	}
	inline SquareMatrix operator/(const float m) const {
		return SquareMatrix(v1/m, v2/m);
	}
	inline float det() {
		return v1.x*v2.y - v1.y*v2.x;
	}
	inline SquareMatrix inverse() {
		float tdet = det();
		if(tdet == 0)
			return SquareMatrix();
		else
			return SquareMatrix(CVec(v2.y,-v1.y),CVec(-v2.x,v1.x))/tdet;
	}
};




#endif  //  __MATHLIB_H__
