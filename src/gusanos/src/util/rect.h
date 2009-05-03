#ifndef OMFGUTIL_DETAIL_RECT_H
#define OMFGUTIL_DETAIL_RECT_H

#include "vec.h"
#include <allegro.h>

template<class T>
class BasicRect
{
public:
	BasicRect(void)
	{
	}

	BasicRect(T x1_, T y1_, T x2_, T y2_)
	: x1(x1_), y1(y1_), x2(x2_), y2(y2_)
	{

	}
	
	BasicRect(BITMAP* b)
	: x1(0), y1(0), x2(b->w - 1), y2(b->h - 1)
	{

	}

	T x1;
	T y1;
	T x2;
	T y2;
	
	T centerX() const
	{
		return (x1 + x2) / T(2);
	}
	
	T centerY() const
	{
		return (y1 + y2) / T(2);
	}

	T getWidth() const
	{
		return x2 - x1;
	}

	T getHeight() const
	{
		return y2 - y1;
	}
	
	BasicRect flip() const
	{
		return BasicRect<T>(y1, x1, y2, x2);
	}

	bool isValid()
	{
		return x1 <= x2 && y1 <= y2;
	}
	
	void join(BasicRect const& b)
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

	bool isIntersecting(BasicRect const& b) const
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

	bool intersect(BasicRect const& b)
	{
		if(!isIntersecting(b))
		{	  
			return false;
		}

		if(b.x1 > x1)
			x1 = b.x1;
		if(b.y1 > y1)
			y1 = b.y1;
		if(b.x2 < x2)
			x2 = b.x2;
		if(b.y2 < y2)
			y2 = b.y2;

		return true;
	}

	bool isInside(T x, T y) const
	{
		T diffX = x - x1;
		T diffY = y - y1;
		
		return diffX < getWidth() && diffX >= T(0)
		    && diffY < getHeight() && diffY >= T(0);
		
	}
	
	BasicRect operator&(BasicRect const& b) const
	{
		return BasicRect(*this) &= b;
	}
	
	BasicRect& operator&=(BasicRect const& b)
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
	
	BasicRect operator+(BaseVec<T> const& b)
	{
		return BasicRect(*this) += b;
	}
	
	BasicRect& operator+=(BaseVec<T> const& b)
	{
		x1 += b.x;
		x2 += b.x;
		y1 += b.y;
		y2 += b.y;
		
		return *this;
	}
	
	BasicRect translate(T x, T y)
	{
		return BasicRect(x1 + x, y1 + y, x2 + x, y2 + y);
	}
};

typedef BasicRect<int> Rect;
typedef BasicRect<float> FRect;

#endif //OMFGUTIL_DETAIL_RECT_H
