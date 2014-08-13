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


#include <cassert>
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <SDL_timer.h>

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
// Get a random number between -1 and 1
float GetRandomNum()
{
	int rn = rand();
	return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}

float GetRandomPosNum() {
	int rn = rand();
	return (float)rn / (float)RAND_MAX;
}

///////////////////
// Get a random integer with a max value
int GetRandomInt(int max)
{
	assert(max >= 0);
	float f = GetRandomPosNum()*(float(max)+1);
	return CLAMP((int)f, 0, max);
}

unsigned long GetRandomLong(unsigned long max) {
	float f = GetRandomPosNum()*(float(max)+1);
	return MIN(f, max);
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
CVec GetRandomVec()
{
	float angle = GetRandomPosNum() * 2.0f * (float)PI;
	return CVec( cosf(angle), sinf(angle) ) * GetRandomNum();
}


///////////////////
// Get forward, and right vectors from a yaw angle
void GetVecsFromAngle(float yaw,CVec *forward, CVec *right)
{
	const float angle = yaw * (float)(PI / 180);
	const float sy = sinf(angle);
	const float cy = cosf(angle);	
	const float sp = 0; //sinf(0);
	const float cp = 1; //cosf(0);	
	const float sr = sp;
	const float cr = cp;

	if(forward)
		*forward = CVec(cp*cy,cp*sy);
	if(right)
		*right = CVec((-1*sr*sp*cy+-1*cr*-sy),(-1*sr*sp*sy+-1*cr*cy));
}

CVec GetVecFromAngle(float yaw) {
	CVec ret;
	GetVecsFromAngle(yaw, &ret, NULL);
	return ret;
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

////////////////////
// Create a parabola from three points
Parabola::Parabola(CVec p1, CVec p2, CVec p3)
{
	const float m = p1.x;
	const float n = p1.y;
	const float o = p2.x;
	const float p = p2.y;
	const float q = p3.x;
	const float r = p3.y;

	const float denom = (pow(m,2)-m*(o+q)+o*q)*(o-q);
	if (denom == 0)  {
		a = b = c = 0;
		return;
	}

	a = -(m*(p-r)+n*(q-o)+o*r-p*q)/denom;
	b = (m*m*(p-r)+n*(o+q)*(q-o)+o*o*r-p*q*q)/denom;
	c = (m*m*(o*r-p*q)+m*(p*q*q-o*o*r)+n*o*q*(o-q))/denom;
}

/////////////////////
// Create a parabola from two points and a tangent angle in point p1
Parabola::Parabola(CVec p1, float angleP1, CVec p2)
{
	const float p = p1.x;
	const float r = p1.y;
	const float s = p2.x;
	const float t = p2.y;
	const float cos_f = cosf(angleP1);
	const float sin_f = sinf(angleP1);

	const float denom = (p-s) * (p-s) * cos_f;
	if (denom == 0)  {
		a = b = c = 0;
		return;
	}

	a = -((r-t)*cos_f+(s-p)*sin_f)/denom;
	b = (2*p*(r-t)*cos_f+(p+s)*(s-p)*sin_f)/denom;
	c = ((p*p*t-2*p*r*s+r*s*s)*cos_f+p*s*(p-s)*sin_f)/denom;	
}

/////////////////////
// Get length of a parabola segment between points p1 and p2, p1 and p2 must be part of the parabola
float Parabola::getLength(CVec p1, CVec p2)
{
	if (isPointAtParabola(p1) && isPointAtParabola(p2))
		return getLength(p1.x, p2.x);
	else
		return -1;
}

float Parabola::getLength(float pa, float pb)
{
	// Check if we have a line
	if (a == 0)  {
		CVec x1(pa, b * pa + c);
		CVec x2(pb, b * pb + c);
		return (x2 - x1).GetLength();
	}

	float upper_u = 2*a*pa + b;
	float lower_u = 2*a*pb + b;

	float up = (log(sqrt(upper_u*upper_u + 1) + upper_u) + upper_u*sqrt(upper_u*upper_u + 1))/(4*a);
	float low = (log(sqrt(lower_u*lower_u + 1) + lower_u) + lower_u*sqrt(lower_u*lower_u + 1))/(4*a);

	return fabs(up - low);
}

unsigned long SyncedRandom::getRandomSeed()
{
	return (unsigned long)time(NULL) + ~ SDL_GetTicks();
}
