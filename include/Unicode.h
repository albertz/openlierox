/*
	OpenLieroX

	UTF8/Unicode conversions
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __UNICODE_H__
#define __UNICODE_H__

#include <SDL/SDL.h> // for Uint32
#include <string>

#include "Utils.h"


typedef Uint32 UnicodeChar;  // Note: only 16bits are currently being used by OLX

template<typename _Iterator1, typename _Iterator2>
inline void IncUtf8StringIterator(_Iterator1& it, const _Iterator2& last) {
	unsigned char c;
	for(it++; last != it; it++) {
		c = *it;
		if(!(c&0x80) || (c&0xC0)) break;
	}
}

template<typename _Iterator>
inline void MultIncUtf8StringIterator(_Iterator& it, const _Iterator& last, size_t count) {
	for(size_t i = 0; i < count; i++) {
		if(it == last) break;
		IncUtf8StringIterator(it, last);
	}
}

///////////////////
// The iterator points at first byte of the UTF8 encoded character
template<typename _Iterator1, typename _Iterator2>
inline void DecUtf8StringIterator(_Iterator1& it, const _Iterator2& first) {
	unsigned char c;
	for(it--; first != it; it--) {
		c = *it;
		if(!(c&0x80) || (c&0xC0)) {
			break;
		}
	}
}

template<typename _Iterator>
inline _Iterator GetMultIncUtf8StringIterator(_Iterator it, const _Iterator& last, size_t count) {
	MultIncUtf8StringIterator(it, last, count);
	return it;
}

inline std::string::const_iterator Utf8PositionToIterator(const std::string& str, size_t pos) {
	std::string::const_iterator res = str.begin();
	MultIncUtf8StringIterator(res, str.end(), pos);
	return res;
}

inline std::string::iterator Utf8PositionToIterator(std::string& str, size_t pos) {
	std::string::iterator res = str.begin();
	MultIncUtf8StringIterator(res, str.end(), pos);
	return res;
}



////////////////////////
// Reads next unicode character from a UTF8 encoded string
// the iterator shows at the next character after this operation
UnicodeChar GetNextUnicodeFromUtf8(std::string::const_iterator &it, const std::string::const_iterator& last);

////////////////////
// Gets the UTF8 representation of the unicode character (can be more bytes)
std::string GetUtf8FromUnicode(UnicodeChar ch);



inline size_t Utf8StringSize(const std::string& str)  {
	size_t res = 0;
	std::string::const_iterator it = str.begin();
	for(; it != str.end(); IncUtf8StringIterator(it, str.end()))
		res++;

	return res;
}

inline std::string Utf8SubStr(const std::string& str, size_t start, size_t n = -1) {
	if (n == -1)
		return std::string(Utf8PositionToIterator(str,start),str.end());
	else
		return std::string(
			Utf8PositionToIterator(str, start),
			Utf8PositionToIterator(str, start + n));
}

inline void Utf8Erase(std::string& str, size_t start, size_t n = -1) {
	std::string::iterator it = Utf8PositionToIterator(str, start);
	str.erase(it, GetMultIncUtf8StringIterator(it, str.end(), n));	
}

inline void Utf8Insert(std::string& str, size_t start, const std::string& s) {
	str.insert(Utf8PositionToIterator(str, start), s.begin(), s.end());
}

inline void InsertUnicodeChar(std::string& str, size_t pos, UnicodeChar ch) {
	std::string tmp = GetUtf8FromUnicode(ch);
	Utf8Insert(str, pos, tmp);
}

#endif
