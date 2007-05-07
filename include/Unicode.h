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
void IncUtf8StringIterator(_Iterator1& it, const _Iterator2& last) {
	unsigned char c;
	for(it++; last != it; it++) {
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




////////////////////////
// Reads next unicode character from a UTF8 encoded string
UnicodeChar GetNextUnicodeFromUtf8(std::string::const_iterator &it, const std::string::const_iterator& last);

////////////////////
// Gets the UTF8 representation of the unicode character (can be more bytes)
// the iterator shows at the next character after this operation
std::string GetUtf8FromUnicode(UnicodeChar ch);



// returns the new pos
inline size_t InsertUnicodeChar(std::string& str, size_t pos, UnicodeChar ch) {
	std::string tmp = GetUtf8FromUnicode(ch);
	str.insert(pos, tmp);
	return pos + tmp.size();
}



#endif
