#ifndef VERMES_MATH_H
#define VERMES_MATH_H

#include <cmath>
#include <boost/random.hpp>

/**
 * Magic number goodness!
 */

extern boost::mt19937 rndgen;
extern boost::uniform_01<boost::mt19937> rnd;
extern boost::variate_generator<boost::mt19937, boost::uniform_real<> > midrnd;

float const Pi = 3.14159265358979323846f;

inline float deg2rad( float Degrees )
{
	return (Degrees * Pi) / 180;
}

inline float rad2deg( float Radians )
{
	return (Radians * 180) / Pi;
}

inline unsigned long rndInt(unsigned long max)
{
	return static_cast<unsigned long>(
		(static_cast<unsigned long long>(rndgen()) * max) >> (CHAR_BIT * sizeof(boost::mt19937::result_type))
	);
}

template<unsigned int B, unsigned int E>
//struct ctpow { static unsigned int const value = ctpow<B*B, E>>1>::value * ((E & 1) ? B : 1); };
struct ctpow { static unsigned int const value = ctpow<B*B, E>::value * ((E & 1) ? B : 1); };

template<unsigned int B>
struct ctpow<B, 0> { static unsigned int const value = 1; };

// MSVC lrintf implementation, taken from http://www.dpvreony.co.uk/blog/post/63
#ifdef _MSC_VER
inline long lrintf(float f){

#ifdef _M_X64
	//x64 fix
	//http://groups.google.com/group/openjpeg/browse_thread/thread/64b89cdb1a44bb3b
	return (long)((f > 0.0f) ? (f + 0.5f) : (f - 0.5f)); 
#else 

	int i;
	_asm{
		fld f
		fistp i
	};
	return i;

#endif

}
#endif

inline long roundAny(float v)
{
	return lrintf(v);
}

#endif //VERMES_MATH_H
