#ifndef VEC_H
#define VEC_H

#include <cmath>
#include "angle.h"

template<class T>
struct BaseVec
{
	typedef T manip_t;
	
	BaseVec()
	: x(T(0)), y(T(0))
	{
		
	}
	
	BaseVec(Angle angle, double length)
	{
		double radians = angle.toRad();
		x = sin(radians) * length;
		y = -cos(radians) * length;
	}
	
	BaseVec(Angle angle)
	{
		double radians = angle.toRad();
		x = sin(radians);
		y = -cos(radians);
	}
	
	template<class T2>
	explicit BaseVec(BaseVec<T2> const& b)
	: x(static_cast<T>(b.x)), y(static_cast<T>(b.y))
	{
		
	}
	
	BaseVec(BaseVec const& a, BaseVec const& b)
	: x(b.x - a.x), y(b.y - a.y)
	{
		
	}

	BaseVec(T const& x_, T const& y_)
	: x(x_) , y(y_)
	{
	}
	
	void zero()
	{
		x = static_cast<T>(0);
		y = static_cast<T>(0);
	}
	
	BaseVec operator - (BaseVec const &A) const
	{
		return BaseVec(x - A.x, y - A.y);
	}
	
	BaseVec operator + (BaseVec const &A) const
	{
		return BaseVec(x + A.x, y + A.y);
	}
	
	friend BaseVec operator * (T const& A, BaseVec const& B)
	{
		return BaseVec(A * B.x, A * B.y);
	}
	
	BaseVec operator * (T const& A) const
	{
		return BaseVec(A * x, A * y);
	}
	
	BaseVec operator / (T const& A) const
	{
		return BaseVec(x / A, y / A);
	}
	
	BaseVec& operator += (BaseVec const& A)
	{
		x += A.x;
		y += A.y;
		return *this;
	}
	
	BaseVec& operator -= (BaseVec const& A)
	{
		x -= A.x;
		y -= A.y;
		return *this;
	}
	
	BaseVec& operator *= (T const& A)
	{
		x *= A;
		y *= A;
		return *this;
	}
	
	BaseVec& operator /= (T const& A)
	{
		x /= A;
		y /= A;
		return *this;
	}
	
	T dotProduct(BaseVec const& A) const
	{
		return x * A.x + y * A.y;
	}
	
	T perpDotProduct(BaseVec const& A) const
	{
		return x * A.y - y * A.x;
	}
	
	BaseVec perp() const
	{
		return BaseVec(-y, x);
	}
	
	double length() const
	{
		return sqrt(x*x + y*y);
	}
	
	BaseVec normal() const
	{
		double invLength = 1.0 / length();
		return BaseVec(invLength*x, invLength*y);
	}
	
	T lengthSqr() const
	{
		return x*x + y*y;
	}
	
	Angle getAngle() const
	{
		return Angle::fromRad( atan2(double(x), double(-y)) );
	}
	
	T x;
	T y;
	
};

typedef BaseVec<float> Vec;
typedef BaseVec<int> IVec;

#endif // _VEC_H_
