/*
	OpenLieroX
 
	Unicode string
	
	code under LGPL
	created 03-05-2007
	by Albert Zeyer and Dark Charlie
 */

#ifndef __UCSTRING_H__
#define __UCSTRING_H__

#include <SDL/SDL.h> // for Uint32
#include <string>

typedef Uint32 UnicodeChar;



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

class UCString;
template<typename _BaseStringType, typename _BaseIteratorType> class Utf8StringIterator;
UnicodeChar GetNextUnicodeFromUtf8(Utf8StringIterator<const UCString, std::string::const_iterator>& it, const Utf8StringIterator<const UCString, std::string::const_iterator>& last);




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
	friend class UCString;
	
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


/*
	Unicode class
 
	wrapper around std::string, which uses it as a represantation of an UTF8 string
	it provides correct iterators for this issue
 */
class UCString : public std::string {
public:
	
	typedef Utf8StringIterator<UCString, std::string::iterator> iterator;
	typedef Utf8StringIterator<const UCString, std::string::const_iterator> const_iterator;
	// TODO
	/*typedef Utf8StringIterator<UCString, std::string::reverse_iterator> reverse_iterator;
	typedef Utf8StringIterator<const UCString, std::string::const_reverse_iterator> const_reverse_iterator;*/
	
	UCString() {}
	UCString(const char* s) : std::string(s) {}
	UCString(const char* s, size_t len) : std::string(s, len) {}
	UCString(const std::string& s) : std::string(s) {}
	UCString(const UCString& s) : std::string(s) {}
	UCString(const_iterator range_l, const_iterator range_r) : std::string(range_l.it, range_r.it) {}
	void operator=(const char* s) { (std::string&)*this = s; }
	void operator=(const UCString& s) { (std::string&)*this = s; }
	
	iterator begin() { return iterator(this, ((std::string*)this)->begin()); }
	const_iterator begin() const { return const_iterator(this, ((std::string*)this)->begin()); }
	iterator end() { return iterator(this, ((std::string*)this)->end()); }
	const_iterator end() const { return const_iterator(this, ((std::string*)this)->end()); }
	// TODO
	/*reverse_iterator rbegin() { return reverse_iterator(this, ((std::string*)this)->rbegin()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(this, ((std::string*)this)->rbegin()); }
	reverse_iterator rend() { return reverse_iterator(this, ((std::string*)this)->rend()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(this, ((std::string*)this)->rend()); }*/
	
	size_t size() const {
		size_t len = 0;
		const_iterator it = begin();
		while(end() != it) { len++; it++; }
		return len;
	}
	
	size_t length() const { return size(); }
	
};

UCString operator+(const UCString& str1, const UCString& str2) { return (std::string&)str1 + (std::string&)str2; }
UCString operator+(const char* str1, const UCString& str2) { return std::string(str1) + (std::string&)str2; }
UCString operator+(const UCString& str1, const char* str2) { return (std::string&)str1 + std::string(str2); }

#endif
