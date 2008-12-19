#ifndef LIERO_TEXT_HPP
#define LIERO_TEXT_HPP

#include <string>

inline std::string toString(int v)
{
	char buf[20];
	std::sprintf(buf, "%d", v);
	return buf;
}

char const* timeToString(int sec);

inline void rtrim(std::string& str)
{
	std::string::size_type e = str.find_last_not_of(" \t\r\n");
	if(e == std::string::npos)
		str.clear();
	else
		str.erase(e + 1);
}

inline void findReplace(std::string& str, std::string const& find, std::string const& replace)
{
	std::string::size_type p = str.find(find);
	if(p != std::string::npos)
		str.replace(p, find.size(), replace);
}

bool ciCompare(std::string const& a, std::string const& b);
int unicodeToDOS(int c);

#endif // LIERO_TEXT_HPP
