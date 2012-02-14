#include <string>
#include "StringConv.h"
#include "StringUtils.h"
#include "CVec.h"


template<>
bool from_string<bool>(const std::string& s, bool& fail) {
	std::string s1(stringtolower(s));
	TrimSpaces(s1);
	if( s1 == "true" || s1 == "yes" || s1 == "on" ) return true;
	else if( s1 == "false" || s1 == "no" || s1 == "off" ) return false;
	return from_string<int>(s, fail) != 0;
}


template<>
VectorD2<int> from_string< VectorD2<int> >(const std::string& s, bool& fail) {
	std::string tmp = s;
	TrimSpaces(tmp);
	if(tmp.size() > 2 && tmp[0] == '(' && tmp[tmp.size()-1] == ')')
		tmp = tmp.substr(1, tmp.size() - 2);
	size_t f = tmp.find(',');
	if(f == std::string::npos) { fail = true; return VectorD2<int>(); }
	VectorD2<int> v;
	fail = false;
	v.x = from_string<int>(tmp.substr(0, f), fail);
	if(fail) return VectorD2<int>();
	v.y = from_string<int>(tmp.substr(f + 1), fail);
	if(fail) return VectorD2<int>();
	return v;
}
