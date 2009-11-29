#ifndef OMFGUTIL_DETAIL_COMMON_H
#define OMFGUTIL_DETAIL_COMMON_H

typedef unsigned char uchar;
typedef unsigned long ulong;

template<class T>
struct mover
{
	mover(mover& b)
	{
		data.swap(b.data);
	}
	
	mover(T& b)
	{
		data.swap(b);
	}
	
	operator T&()
	{
		return data;
	}
	
	operator T const&() const
	{
		return data;
	}
	
	friend T& operator<<(T& lhs, mover& rhs)
	{
		lhs.swap(rhs.data);
		return lhs;
	}
	
	T data;
};

#endif //OMFGUTIL_DETAIL_COMMON_H
