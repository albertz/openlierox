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


typedef Uint32 UnicodeChar;
// the iterator shows at the next character after this operation
UnicodeChar GetNextUnicodeFromUtf8(std::string::const_iterator& it, const std::string::const_iterator& last);
std::string GetUtf8FromUnicode(UnicodeChar ch);



template<typename _Iterator>
void IncUtf8StringIterator(_Iterator& it, const _Iterator& last) {
	unsigned char c;
	for(it++; it != last; it++) {
		c = *it;
		if(!(c&0x80) || (c&0xC0)) break;
	}
}

template<typename _Iterator>
void MultIncUtf8StringIterator(_Iterator& it, const _Iterator& last, size_t count) {
	for(size_t i = 0; i < count; i++) {
		if(it == last) break;
		IncUtf8StringIterator(it, last);
	}
}

inline size_t Utf8StringLength(const std::string& utf8string) {
	size_t len = 0;
	std::string::const_iterator it = utf8string.begin();
	while(it != utf8string.end()) {
		len++;
		IncUtf8StringIterator(it, utf8string.end());
	}
	return len;
}

// returns the new pos
inline size_t InsertUnicodeChar(std::string& str, size_t pos, UnicodeChar ch) {
	std::string tmp = GetUtf8FromUnicode(ch);
	str.insert(pos, tmp);
	return pos + tmp.size();
}

#endif
