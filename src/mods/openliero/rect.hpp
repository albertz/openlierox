#ifndef LIERO_RECT_HPP
#define LIERO_RECT_HPP

/*
#include "vec.hpp"
*/

template<class T>
class basic_rect
{
public:
	basic_rect()
	{
	}

	basic_rect(T x1_, T y1_, T x2_, T y2_)
	: x1(x1_), y1(y1_), x2(x2_), y2(y2_)
	{

	}
	/*
	basic_rect(basic_vec<T> const& center, T size)
	: x1(center.x - size), y1(center.y - size)
	, x2(center.x + size), y2(center.y + size)
	{

	}
	*/
	
	T x1;
	T y1;
	T x2;
	T y2;
	
	T center_x() const
	{
		return (x1 + x2) / T(2);
	}
	
	T center_y() const
	{
		return (y1 + y2) / T(2);
	}
	/*
	basic_vec<T> center() const
	{
		return basic_vec<T>(center_x(), center_y());
	}*/

	T width() const
	{
		return x2 - x1;
	}

	T height() const
	{
		return y2 - y1;
	}
	
	basic_rect flip() const
	{
		return basic_rect<T>(y1, x1, y2, x2);
	}

	bool valid()
	{
		return x1 <= x2 && y1 <= y2;
	}
	
	void join(basic_rect const& b)
	{
		if(b.x1 < x1)
			x1 = b.x1;
		if(b.y1 < y1)
			y1 = b.y1;
		if(b.x2 > x2)
			x2 = b.x2;
		if(b.y2 > y2)
			y2 = b.y2;
	}

	bool intersecting(basic_rect const& b) const
	{
		if(b.y2 < y1
		|| b.y1 > y2
		|| b.x2 < x1
		|| b.x1 > x2)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	void intersect(basic_rect const& b)
	{
	/*
		if(!intersecting(b))
		{	  
			return false;
		}*/

		if(b.x1 > x1)
			x1 = b.x1;
		if(b.y1 > y1)
			y1 = b.y1;
		if(b.x2 < x2)
			x2 = b.x2;
		if(b.y2 < y2)
			y2 = b.y2;

		//return true;
	}
/*
	bool inside(basic_vec<T> v) const
	{
		T diffX = v.x - x1;
		T diffY = v.y - y1;
		
		return diffX < width() && diffX >= T(0)
		    && diffY < height() && diffY >= T(0);
		
	}*/
	
	basic_rect operator&(basic_rect const& b) const
	{
		return basic_rect(*this) &= b;
	}
	
	basic_rect& operator&=(basic_rect const& b)
	{
		if(b.x1 > x1)
			x1 = b.x1;
		if(b.y1 > y1)
			y1 = b.y1;
		if(b.x2 < x2)
			x2 = b.x2;
		if(b.y2 < y2)
			y2 = b.y2;
		
		return *this;
	}
	/*
	basic_rect operator+(basic_vec<T> const& b)
	{
		return basic_rect(*this) += b;
	}
	
	basic_rect& operator+=(basic_vec<T> const& b)
	{
		x1 += b.x;
		x2 += b.x;
		y1 += b.y;
		y2 += b.y;
		
		return *this;
	}
	
	basic_rect operator-(basic_vec<T> const& b)
	{
		return basic_rect(*this) -= b;
	}
	
	basic_rect& operator-=(basic_vec<T> const& b)
	{
		x1 -= b.x;
		x2 -= b.x;
		y1 -= b.y;
		y2 -= b.y;
		
		return *this;
	}
*/
	
	basic_rect translate(T x, T y)
	{
		return basic_rect(x1 + x, y1 + y, x2 + x, y2 + y);
	}
	
/*
	basic_vec<T> ul()
	{
		return basic_vec<T>(x1, y1);
	}
	
	basic_vec<T> ur()
	{
		return basic_vec<T>(x2, y1);
	}
	
	basic_vec<T> ll()
	{
		return basic_vec<T>(x1, y2);
	}
	
	basic_vec<T> lr()
	{
		return basic_vec<T>(x2, y2);
	}
*/
};

typedef basic_rect<int> Rect;
typedef basic_rect<float> FRect;

#endif // LIERO_RECT_HPP
