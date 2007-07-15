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

// secure str handling macros
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



#ifndef __USE_GNU
	inline size_t strnlen(const char *str, size_t maxlen) {
		register size_t i;
		for(i = 0; (i < maxlen) && str[i]; ++i) {}
		return i;
	}
#endif

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
#else
inline char* strlwr(char* string) {
	char* ret = string;
	if(string) while( 0 != ( *string++ = (char)tolower( *string ) ) ) ;
	return ret;
}
#endif


// like strcasecmp, but for a char
int chrcasecmp(const char c1, const char c2);

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
#	define		stricmp		strcasecmp
#else // WIN32
#	define		strcasecmp	stricmp
#endif




void	TrimSpaces(std::string& szLine);
bool	replace(const std::string& text, const std::string& what, const std::string& with, std::string& result);
bool	replace(std::string& text, const std::string& what, const std::string& with);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, int max);
std::string strip(const std::string& text, int width);
bool stripdot(std::string& text, int width);
void	ucfirst(std::string& text);
void	stringtolower(std::string& text);
std::string	ReadUntil(const std::string& text, char until_character = '\n');
std::string	ReadUntil(FILE* fp, char until_character = '\n');
Uint32	StrToCol(const std::string& str);
const std::vector<std::string>& explode(const std::string& str, const std::string& delim);
std::string freadstr(FILE *fp, size_t maxlen);
inline std::string freadfixedcstr(FILE *fp, size_t maxlen) { return ReadUntil(freadstr(fp, maxlen), '\0'); }
size_t fwrite(const std::string& txt, size_t len, FILE* fp);
size_t findLastPathSep(const std::string& path);
void stringlwr(std::string& txt);
bool strincludes(const std::string& str, const std::string& what);
short stringcasecmp(const std::string& s1, const std::string& s2);


std::string GetFileExtension(const std::string& filename);


// TODO: remove all the following functions
void	StripQuotes(char *dest, char *src); // TODO: remove this
void	StripQuotes(std::string& str);
void    lx_strncpy(char *dest, char *src, int count); // TODO: remove this
char    *StripLine(char *szLine);
char    *TrimSpaces(char *szLine);
bool	replace(char *text, const char *what, const char *with, char *result);
char	*strip(char *buf, int width);
bool	stripdot(char *buf, int width);
char	*ucfirst(char *text);




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

inline int atoi(const std::string& str)  { return from_string<int>(str);  }
inline float atof(const std::string& str) { return from_string<float>(str);  }



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
