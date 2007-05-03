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
#include "UCString.h"
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




#ifdef WIN32
	inline int strncasecmp(const char *str1, const char *str2, size_t l) {return _strnicmp(str1,str2,l); }
#	define vsnprintf _vsnprintf
#	define snprintf	 _snprintf
#else
inline void strlwr(char* string) {
	if(string) while( 0 != ( *string++ = (char)tolower( *string ) ) ) ;
}
#endif


// like strcasecmp, but for a char
int chrcasecmp(const char c1, const char c2);

#ifndef WIN32
inline char* itoa(int val, char* buf, int base) {
	int i = 29; // TODO: bad style!
	buf[i+1] = '\0';

	for(; val && i ; --i, val /= base)	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
}
#	define		stricmp		strcasecmp
#else // WIN32
#	define		strcasecmp	stricmp
#endif




void	TrimSpaces(UCString& szLine);
bool	replace(const UCString& text, const UCString& what, const UCString& with, UCString& result);
bool	replace(UCString& text, const UCString& what, const UCString& with);
UCString replacemax(const UCString& text, const UCString& what, const UCString& with, UCString& result, int max);
UCString replacemax(const UCString& text, const UCString& what, const UCString& with, int max);
UCString strip(const UCString& text, int width);
bool stripdot(UCString& text, int width);
void	ucfirst(UCString& text);
void	stringtolower(UCString& text);
UCString	ReadUntil(const UCString& text, char until_character = '\n');
UCString	ReadUntil(FILE* fp, char until_character = '\n');
Uint32	StrToCol(const UCString& str);
const std::vector<UCString>& explode(const UCString& str, const UCString& delim);
UCString freadstr(FILE *fp, size_t maxlen);
inline UCString freadfixedcstr(FILE *fp, size_t maxlen) { return ReadUntil(freadstr(fp, maxlen), '\0'); }
size_t fwrite(const UCString& txt, size_t len, FILE* fp);
size_t findLastPathSep(const UCString& path);
void stringlwr(UCString& txt);
bool strincludes(const UCString& str, const UCString& what);
short stringcasecmp(const UCString& s1, const UCString& s2);


UCString GetFileExtension(const UCString& filename);


// TODO: remove all the following functions
void	StripQuotes(char *dest, char *src); // TODO: remove this
void	StripQuotes(UCString& str);
void    lx_strncpy(char *dest, char *src, int count); // TODO: remove this
char    *StripLine(char *szLine);
char    *TrimSpaces(char *szLine);
bool	replace(char *text, const char *what, const char *with, char *result);
char	*strip(char *buf, int width);
bool	stripdot(char *buf, int width);
char	*ucfirst(char *text);




template<typename T>
T from_string(const UCString& s, std::ios_base& (*f)(std::ios_base&), bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> f >> t).fail();
	return t;
}

template<typename T>
T from_string(const UCString& s, std::ios_base& (*f)(std::ios_base&)) {
	std::istringstream iss(s); T t;
	iss >> f >> t;
	return t;
}

template<typename T>
T from_string(const UCString& s, bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> t).fail();
	return t;
}

template<typename T>
T from_string(const UCString& s) {
	std::istringstream iss(s); T t;
	iss >> t;
	return t;
}



// UCString itoa
inline UCString itoa(int num,int base=10)  {
	// TODO: better!! (use ostringstream)
	static char buf[64];
	static UCString ret;
	ret = itoa(num,buf,base);
	fix_markend(buf);
	return ret;
}

inline int atoi(const UCString& str)  { return from_string<int>(str);  }
inline float atof(const UCString& str) { return from_string<float>(str);  }



class simple_reversestring_hasher { public:
	inline size_t operator() (const UCString& str) const {
		UCString::const_reverse_iterator pos = str.rbegin();
		unsigned short nibble = 0;
		size_t result = 0;
		for(; pos != str.rend() && nibble < sizeof(size_t)*2; pos++, nibble++)
			result += ((size_t)*pos % 16) << nibble*4;
		return result;
	}
};


#endif
