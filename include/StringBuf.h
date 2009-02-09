/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __OLX__STRINGBUF_H__
#define __OLX__STRINGBUF_H__

// String Buffer
// Created 11/8/08
// Karel Petranek

#include <string>
#include <vector>

// String buffer class
class StringBuf  {
public:
	StringBuf() : sStr("") { tPos = sStr.begin(); }
	StringBuf(const std::string& str) : sStr(str) { tPos = sStr.begin(); }
private:
	std::string::iterator tPos;
	std::string sStr;

public:
	StringBuf& operator=(const StringBuf& b2)  {
		if (&b2 != this)  {
			tPos = b2.tPos;
			sStr = b2.sStr;
		}
		return *this;
	}

	StringBuf& operator=(const std::string& s)  {
		sStr = s;
		tPos = sStr.begin();
		return *this;
	}

	StringBuf& operator=(const char *s)  {
		sStr = s;
		tPos = sStr.begin();
		return *this;
	}

	char getC()		{ return tPos == sStr.end() ? 0 : *tPos; }
	void setC(char c) { if (!atEnd()) *tPos = c; }
	void incPos()	{ tPos++; }
	void decPos()	{ tPos--; }
	void resetPos()	{ tPos = sStr.begin(); }
	bool atEnd()	{ return tPos == sStr.end(); }
	void erase(size_t start, size_t count)  { sStr.erase(start, count); tPos = sStr.begin(); }
	void toLower();
	std::string readUntil(char c);
	std::string readUntil(const std::string& char_array);
	std::string read(size_t num);
	std::string getRestStr()  { return std::string(tPos, sStr.end()); }
	void trimBlank();
	void adjustBlank();
	size_t skipBlank();
	size_t size()	{ return sStr.size(); }
	bool empty()	{ return sStr.size() == 0; }
	const std::string& str()  { return sStr; }
	std::vector<std::string> splitByBlank();
	std::vector<std::string> splitBy(char c);
	void debugPrint();
};

#endif
