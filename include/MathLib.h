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


float	CalculateDistance(CVec p1, CVec p2);
float	NormalizeVector(CVec *vec);
CVec	GetRandomVec(void);
void	GetAngles(int yaw,CVec *forward, CVec *right);
float	VectorAngle(CVec vec1, CVec vec2);
float	VectorLength(CVec vec);
float   fastSQRT(float x);

#define SIGN(x) (((x) > 0)?1:(((x) < 0)?-1:0))
#define SQR(x) ((x)*(x))










#endif  //  __MATHLIB_H__
