#ifndef GUSANOS_MATH_H
#define GUSANOS_MATH_H

#include <cmath>
#include <boost/random.hpp>

extern boost::mt19937 rndgen;
extern boost::uniform_01<boost::mt19937> rnd;
extern boost::variate_generator<boost::mt19937, boost::uniform_real<> > midrnd;

double const PI = 3.14159265358979323846;

inline float deg2rad( float Degrees )
{
	return (Degrees * PI) / 180;
}

inline float rad2deg( float Radians )
{
	return (Radians * 180) / PI;
}

inline unsigned long rndInt(unsigned long max)
{
	return static_cast<unsigned long>(
		(static_cast<unsigned long long>(rndgen()) * max) >> (CHAR_BIT * sizeof(boost::mt19937::result_type))
	);
}

template<unsigned int B, unsigned int E>
struct ctpow { static unsigned int const value = ctpow<B*B, E>>1>::value * ((E & 1) ? B : 1); };

template<unsigned int B>
struct ctpow<B, 0> { static unsigned int const value = 1; };

inline long roundAny(float v)
{
	return lrintf(v);
}

#endif //GUSANOS_MATH_H
