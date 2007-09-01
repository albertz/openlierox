/*
	OpenLieroX

	string utilities
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

#include <SDL/SDL.h> // for Uint32
#include <stdio.h> // for FILE
#include <string>
#include <sstream>
#include <vector>

//
// C-string handling routines
//
// HINT: these are obsolete, use std::string where possible!!!

// Secure c-string handling macros
// WARNING: don't use expressions like buf[i++] with the macros, because the "i" variable will be incremented twice in some macros!
#define		fix_markend(chrarray) \
				chrarray[sizeof(chrarray)-1] = '\0';
#define		fix_strnlen(chrarray) \
				strnlen(chrarray,sizeof(chrarray))
#define		fix_strncpy(chrarray, src) \
			{	strncpy(chrarray, src, sizeof(chrarray)); \
			 	chrarray[sizeof(chrarray)-1] = '\0'; }
#define		fix_strncat(chrarray, src) \
			{	size_t destlen = strnlen(chrarray, sizeof(chrarray)); \
				strncpy(&chrarray[destlen], src, sizeof(chrarray)-destlen); \
				chrarray[sizeof(chrarray)-1] = '\0'; }
#define		dyn_markend(dest, len) \
				dest[len-1] = '\0';
#define		dyn_strncpy(dest, src, len) \
			{	strncpy(dest, src, len); \
				dest[len-1] = '\0'; }
#define		dyn_strncat(dest, src, len) \
			{	size_t destlen = strnlen(dest, len); \
				strncpy(&dest[destlen], src, len-destlen); \
				dest[len-1] = '\0'; }


// Strnlen definition for compilers that don't have it
#ifndef __USE_GNU
	inline size_t strnlen(const char *str, size_t maxlen) {
		register size_t i;
		for(i = 0; (i < maxlen) && str[i]; ++i) {}
		return i;
	}
#endif

// Misc cross-compiler compatibility problem solutions
#ifdef WIN32
#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
	inline int strncasecmp(const char *str1, const char *str2, size_t l) {
		return _strnicmp(str1, str2, l);
	}
#endif
#	define vsnprintf _vsnprintf
#	define snprintf	 _snprintf
#	define stricmp _stricmp
#	define fcloseall _fcloseall
#	define strcasecmp	stricmp
#else
inline char* strlwr(char* string) {
	char* ret = string;
	if(string) while( 0 != ( *string++ = (char)tolower( *string ) ) ) ;
	return ret;
}
#endif


/////////////
// Case-insensitive comparison of two chars, behaves like stringcasecmp
int chrcasecmp(const char c1, const char c2);

/////////////
// C-string itoa for non-windows compilers (on Windows it's defined in windows.h)
#ifndef WIN32
inline char* itoa(int val, char* buf, int base) {
	int i = 29; // TODO: bad style
	buf[i+1] = '\0';

    do {
        buf = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base] + buf;
        --i, val /= base;
    } while(val && i);

    return &buf[i+1];
}

// Cross-compiler compatibility
#	define		stricmp		strcasecmp
#endif


//
// C++ string (std::string) routines
//
// HINT: use these where possible

void			TrimSpaces(std::string& szLine);
bool			replace(const std::string& text, const std::string& what, const std::string& with, std::string& result);
bool			replace(std::string& text, const std::string& what, const std::string& with);
std::string		replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max);
std::string		replacemax(const std::string& text, const std::string& what, const std::string& with, int max);
std::string		strip(const std::string& text, int width);
bool			stripdot(std::string& text, int width);
void			ucfirst(std::string& text);
void			stringtolower(std::string& text);
std::string		ReadUntil(const std::string& text, char until_character = '\n');
std::string		ReadUntil(FILE* fp, char until_character = '\n');
Uint32			StrToCol(const std::string& str);
const std::vector<std::string>& explode(const std::string& str, const std::string& delim);
void			freadstr(std::string& result, size_t maxlen, FILE *fp);
size_t			fwrite(const std::string& txt, size_t len, FILE* fp);
size_t			findLastPathSep(const std::string& path);
void			stringlwr(std::string& txt);
bool			strincludes(const std::string& str, const std::string& what);
short			stringcasecmp(const std::string& s1, const std::string& s2);
const std::vector<std::string>& clever_split(const std::string& str, int maxlen);
void			StripQuotes(std::string& str);
std::string		GetFileExtension(const std::string& filename);
void			xmlEntities(std::string& text);

////////////////////
// Read a fixed-length C-string from a file
inline std::string freadfixedcstr(FILE *fp, size_t maxlen) {
	std::string fileData;
	freadstr(fileData, maxlen, fp);
	return ReadUntil(fileData, '\0');
}


// Conversion functions from string to numbers

template<typename T>
T from_string(const std::string& s, std::ios_base& (*f)(std::ios_base&), bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> f >> t).fail();
	return t;
}

template<typename T>
T from_string(const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
	std::istringstream iss(s); T t;
	iss >> f >> t;
	return t;
}

template<typename T>
T from_string(const std::string& s, bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> t).fail();
	return t;
}

template<typename T>
T from_string(const std::string& s) {
	std::istringstream iss(s); T t;
	iss >> t;
	return t;
}

inline int atoi(const std::string& str)  { return from_string<int>(str);  }
inline float atof(const std::string& str) { return from_string<float>(str);  }



// Conversion functions from numbers to string

template<typename T>
std::string to_string(T val) {
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

inline std::string ftoa(float val, int precision = -1)
{
	std::string res = to_string<float>(val);
	if (precision != -1)  {
		size_t dotpos = res.find_last_of('.');
		if (dotpos == std::string::npos)  {
			res += '.';
			for (int i = 0; i < precision; i++)
				res += '0';
		} else {
			res = res.substr(0, dotpos + precision);
		}
	}

	return res;
}

// std::string itoa
inline std::string itoa(int num, short base=10)  {
	std::string buf;
	bool negative = false;
	if (num<0)  { // Handle negative values
		negative = true;
		num = -num;
	}

	do {	
		buf = "0123456789abcdefghijklmnopqrstuvwxyz"[num % base] + buf;
		num /= base;
	} while(num);

	// Sign
	if (negative)
		buf = "-"+buf;

	return buf;
}


class simple_reversestring_hasher { public:
	inline size_t operator() (const std::string& str) const {
		std::string::const_reverse_iterator pos = str.rbegin();
		unsigned short nibble = 0;
		size_t result = 0;
		for(; pos != str.rend() && nibble < sizeof(size_t)*2; pos++, nibble++)
			result += ((size_t)*pos % 16) << nibble*4;
		return result;
	}
};


#endif
