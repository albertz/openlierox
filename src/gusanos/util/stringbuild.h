#ifndef OMFGUTIL_DETAIL_STRINGBUILD_H
#define OMFGUTIL_DETAIL_STRINGBUILD_H

#include <string>
#include <sstream>

struct StringBuilder
{
	template<class T>
	StringBuilder(T const& v)
	{
		ss << v;
	}
	
	template<class T>
	StringBuilder& operator<<(T const& v)
	{
		ss << v;
		return *this;
	}
	
	operator std::string()
	{
		return ss.str();
	}
	
	operator bool()
	{
		return ss;
	}
	
private:
	std::stringstream ss;
};

typedef StringBuilder S_;

#endif //OMFGUTIL_DETAIL_STRINGBUILD_H
