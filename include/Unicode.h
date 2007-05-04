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
#include "UCString.h"

#include "Utils.h"


// the iterator shows at the next character after this operation
UCString GetUtf8FromUnicode(UnicodeChar ch);



// returns the new pos
inline size_t InsertUnicodeChar(UCString& str, size_t pos, UnicodeChar ch) {
	UCString tmp = GetUtf8FromUnicode(ch);
	str.insert(pos, tmp);
	return pos + tmp.size();
}



#endif
