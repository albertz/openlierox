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


/*
	iterator-wrapper class for a string-iterator, which uses a different increase-function
	
	_BaseStringType is the type of the string, where this iterator iterates on
		(thought to be const std::string or std::string)
	_BaseIteratorType is the type of the used iterator
		(thought to be std::string::iterator or similar)
	_IncFunctor is the functor for the increasing
		it gets (_BaseStringType*,_BaseIteratorType)
		return value is ignored
		(thought to be ..)
*/
template<typename _BaseStringType, typename _BaseIteratorType/*, typename _IncFunctor*/>
class Utf8StringIterator {
	template<typename _BaseStringType2, typename _BaseIteratorType2>
	friend class Utf8StringIterator;
	friend class std::string;

private:
	_BaseStringType* str;
	_BaseIteratorType it;

public:
	template<typename _BaseStringType2, typename _BaseIteratorType2>
	Utf8StringIterator(const Utf8StringIterator<_BaseStringType2, _BaseIteratorType2>& i) :
		str(i.str), it(i.it) {}
	
	Utf8StringIterator() : str(NULL) {}
	Utf8StringIterator(_BaseStringType* s, const _BaseIteratorType& i) : str(s), it(i) {}
	Utf8StringIterator& operator=(const Utf8StringIterator& i) { str = i.str; it = i.it; return *this; }

	template<typename _BaseStringType2, typename _BaseIteratorType2>
	bool operator==(const Utf8StringIterator<_BaseStringType2, _BaseIteratorType2>& i) const {
		return it == i.it; }
	template<typename _BaseStringType2, typename _BaseIteratorType2>
	bool operator!=(const Utf8StringIterator<_BaseStringType2, _BaseIteratorType2>& i) const {
		return ! ((*this) == i); }
	bool operator==(const _BaseIteratorType& it) const { return this->it == it; }
	bool operator!=(const _BaseIteratorType& it) const { return this->it != it; }

	Utf8StringIterator& operator++() {
		IncUtf8StringIterator(it, str->end()); return *this; }
	void operator++(int) { IncUtf8StringIterator(it, str->end()); }
	Utf8StringIterator& operator+(unsigned int p) { for(unsigned int i = 0; i < p; i++) (*this)++; return *this; }
	
	UnicodeChar operator*() const {
		// TODO: hack for now, fix...
		Utf8StringIterator<const _BaseStringType, std::string::const_iterator> tmp = *this;
		return GetNextUnicodeFromUtf8(tmp, ((const _BaseStringType*)str)->end()); }

/*	UnicodeChar& operator*() {
		// TODO: hack for now, fix...
		Utf8StringIterator<const _BaseStringType, std::string::const_iterator> tmp = *this;
		return GetNextUnicodeFromUtf8(tmp, ((const _BaseStringType*)str)->end()); }
		*/
};

////////////////////////
// Reads next unicode character from a UTF8 encoded string
template<typename _BaseStringType, typename _BaseIteratorType> class Utf8StringIterator;
UnicodeChar GetNextUnicodeFromUtf8(Utf8StringIterator<const std::string, std::string::const_iterator>& it, const Utf8StringIterator<const std::string, std::string::const_iterator>& last);

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
