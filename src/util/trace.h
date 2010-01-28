#ifndef OMFGUTIL_DETAIL_TRACE_H
#define OMFGUTIL_DETAIL_TRACE_H

#include <algorithm>
using std::min;
using std::max;

struct Fixed
{
	Fixed fromInt(int v)
	{
		return Fixed(static_cast<long>(v) << 16);
	}
	
	Fixed floor()
	{
		return Fixed(v & ~0xFFFF);
	}
	
	Fixed ceil()
	{
		return Fixed((v + 0xFFFF) & ~0xFFFF);
	}
	
	unsigned long frac()
	{
		return v & 0xFFFF;
	}
	
	Fixed operator+(Fixed rhs)
	{
		return Fixed(*this) += rhs;
	}
	
	Fixed& operator+=(Fixed rhs)
	{
		v += rhs.v;
		return *this;
	}
	
	Fixed operator-(Fixed rhs)
	{
		return Fixed(*this) -= rhs;
	}
	
	Fixed& operator-=(Fixed rhs)
	{
		v -= rhs.v;
		return *this;
	}
	
	Fixed operator-()
	{
		return Fixed(-v);
	}
	
	bool operator<(int rhs)
	{
		return (v >> 16) < rhs;
	}
	
	bool operator>(int rhs)
	{
		return (v >> 16) > rhs;
	}
	
	bool operator<=(int rhs)
	{
		return (v >> 16) <= rhs;
	}
	
	bool operator>=(int rhs)
	{
		return (v >> 16) >= rhs;
	}
	
	friend Fixed abs(Fixed b)
	{
		return Fixed(abs(b.v));
	}
	
	Fixed operator/(Fixed rhs)
	{
		long long ve = v;
		return Fixed((ve << 16) / rhs.v);
	}
	
	long v;
private:

	explicit Fixed(long v_)
	: v(v_)
	{
	}
};



template<class BaseT>
struct Tracer
{
	
	
	void trace(BaseVec<Fixed> from, BaseVec<Fixed> to)
	{
		BaseVec<Fixed> diff(from, to);
		
		if(diff.y < 0)
		{
			if(abs(diff.x).v < abs(diff.y).v)
			{
				// \/
				Fixed d(diff.x / -diff.y);
				
				
			}
		}
	}
};

#endif //OMFGUTIL_DETAIL_TRACE_H
