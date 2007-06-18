/*
	OpenLieroX

	UTF8/Unicode conversions
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#include "Unicode.h"

// grabbed from SDL_ttf (also LGPL)
void UNICODE_to_UTF8(unsigned char *utf8, UnicodeChar unicode)
{
    int j=0;

    if (unicode < 0x80)
    {
        utf8[j] = unicode & 0x7F;
    }
    else if (unicode < 0x800)
    {
        utf8[j] = 0xC0 | (unicode >> 6);
        utf8[++j] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode < 0x10000)
    {
        utf8[j] = 0xE0 | (unicode >> 12);
        utf8[++j] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[++j] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode < 0x200000)
    {
        utf8[j] = 0xF0 | (unicode >> 18);
        utf8[++j] = 0x80 | ((unicode >> 12) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[++j] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode < 0x4000000)
    {
        utf8[j] = 0xF8 | (unicode >> 24);
        utf8[++j] = 0x80 | ((unicode >> 18) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 12) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[++j] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode < 0x80000000)
    {
        utf8[j] = 0xFC | (unicode >> 30);
        utf8[++j] = 0x80 | ((unicode >> 24) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 18) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 12) & 0x3F);
        utf8[++j] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[++j] = 0x80 | (unicode & 0x3F);
    }
    else
    	utf8[j] = 0;

    utf8[++j] = 0;
}

std::string GetUtf8FromUnicode(UnicodeChar ch) {
	if(ch == 0) return std::string("\0", 1);
	static unsigned char utf8[7];
	UNICODE_to_UTF8(utf8, ch);
	return (const char*)utf8;
}


UnicodeChar GetNextUnicodeFromUtf8(std::string::const_iterator &it, const std::string::const_iterator& last) {
	if(it == last) return 0;
	
	unsigned char ch = *it;
	UnicodeChar res = ch;
	if ( ch >= 0xFC ) {
		res  =  (ch&0x01) << 30; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 24; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 18; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 12; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 6; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F);
	} else
	if ( ch >= 0xF8 ) {
		res  =  (ch&0x03) << 24; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 18; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 12; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 6; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F);
	} else
	if ( ch >= 0xF0 ) {
		res  =  (ch&0x07) << 18; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 12; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 6; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F);
	} else
	if ( ch >= 0xE0 ) {
		res  =  (ch&0x0F) << 12; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F) << 6; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F);
	} else
	if ( ch >= 0xC0 ) {
		res  =  (ch&0x1F) << 6; it++; if(it == last) return 0; ch = *it;
		res |=  (ch&0x3F);
	}

	it++;
	return res;
}



