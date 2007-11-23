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

#include "LieroX.h"
#include <stdlib.h>
#include <math.h>
#include "Utils.h" // for MIN
#include "MathLib.h"


const int HALF_RAND = (RAND_MAX / 2);


//////////////////////////////////////
//				Generic maths
//////////////////////////////////////


///////////////////
// Faster SQRT function
float fastSQRT(float x)
{
    
    /*if(x <= 0) return 0;
    
	float tmp = x / 2;
	for(short i = 0; i<=10; i++) {
		tmp = tmp + x / tmp;
		tmp = tmp / 2;
	}
	return tmp;*/

	
	return (float)sqrt(x);
}


///////////////////
// Get a random number
float GetRandomNum(void)
{
	int rn;
	rn = rand();
	return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}


///////////////////
// Get a random integer with a max value
int GetRandomInt(int max)
{
	float f = GetRandomNum()*(float)(max+1);

	return MIN(max, abs((int)f) );
}

//////////////////
// Round the number
int Round(float x)
{
        return (int) ceil((double)x-0.5); 
}


//#ifdef VEC2D

//////////////////////////////////////
//				2D Section
//////////////////////////////////////


///////////////////
// Calculate the distance between 2 vectors
float CalculateDistance(CVec p1, CVec p2)
{
	CVec dist = p2-p1;
	return fastSQRT( (dist.x*dist.x) + (dist.y*dist.y) );
}


///////////////////
// Normalize a vector
float NormalizeVector(CVec *vec)
{
	float length;

	length = (float)fastSQRT( vec->x*vec->x + vec->y*vec->y );

	if(length) {
		vec->x /= length;
		vec->y /= length;
	}

	return length;
}


///////////////////
// Get a random normalized vector
CVec GetRandomVec(void)
{
	return CVec(GetRandomNum(),GetRandomNum());
}


///////////////////
// Get forward, and right vectors from a yaw angle
void GetAngles(int yaw,CVec *forward, CVec *right)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	
	angle = yaw * (float)(PI / 180);
	sy = (float)sin(angle);
	cy = (float)cos(angle);	
	sp = 0; //(float)sin(0);
	cp = 1; //(float)cos(0);	
	sr = sp;
	cr = cp;

	if(forward)
		*forward = CVec(cp*cy,cp*sy);
	if(right)
		*right = CVec((-1*sr*sp*cy+-1*cr*-sy),(-1*sr*sp*sy+-1*cr*cy));
}

//////////////////
// Get the angle (in radians) of two vectors
float VectorAngle(CVec vec1, CVec vec2)
{
	return (float)atan2(vec1.y-vec2.y,vec1.x-vec2.x);
	
/*	//return (float)acos(vec1.x*vec2.x+vec1.y*vec2.y)/(VectorLength(vec1)*VectorLength(vec2));
	float scalar = vec1.x*vec2.x + vec1.y*vec2.y;
	float len1 = vec1.GetLength();
	float len2 = vec2.GetLength();
	float result = (float)acos(scalar/(len1*len2));
	return result; */
}


///////////////////
// Get the length of a vector
float VectorLength(CVec vec)
{
	return (float)fastSQRT( vec.x*vec.x + vec.y*vec.y );
}




//#endif  //  VEC2D
