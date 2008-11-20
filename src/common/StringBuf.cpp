/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// String Buffer
// Created 11/8/08
// Karel Petranek

#include <iostream>

#include "StringBuf.h"
#include "StringUtils.h"

// Helper functions
std::string strlwr(const std::string& s)
{
	std::string res = s;
	stringlwr(res);
	return res;
}

/////////////////////
// Adjusts the blank characters so that there are no repeated blank characters and the blank characters are all
// turned to spaces
void StringBuf::adjustBlank()
{
	// Check
	if (sStr.size() < 3)
		return;

	// Initialize iterators
	std::string::const_iterator it = sStr.begin();
	std::string::const_iterator prev = it;
	it++;

	// Get the adjusted string
	std::string res;
	res += *sStr.begin(); // First character, sStr != "" here
	for (; it != sStr.end(); it++, prev++)  {
		if (isspace((uchar)*it))  {  // Blank
			if (isspace((uchar)*prev)) // Previous also blank, ignroe
				continue;
			else  { // Previous not blank, convert to space and add
				res += ' ';
				continue;
			}
		}
		res += *it; // Normal character
	}

	sStr = res;
	tPos = sStr.begin();
}

/////////////////////
// Trims all blank characters from both sides of the string
void StringBuf::trimBlank()
{
	// Start
	while (sStr.size() && isspace((uchar)(*(sStr.begin()))))
		sStr.erase(sStr.begin());

	// End
	while (sStr.size() && isspace((uchar)(*(sStr.rbegin()))))
		sStr.erase(sStr.size() - 1, 1);

	tPos = sStr.begin();
}

////////////////////
// Tokenizes the string buffer by blank characters, multiple blank characters are taken as one
std::vector<std::string> StringBuf::splitByBlank()
{
	std::vector<std::string> res;
	std::string token;
	bool was_space = false;
	for (std::string::iterator it = sStr.begin(); it != sStr.end(); it++)  {
		if (isspace((uchar)(*it)))  {
			if (was_space) // Multiple spaces get ignored
				continue;
			else  {
				res.push_back(token); // Add the token
				token = "";
			}

			was_space = true;
			continue;
		}

		was_space = false;
		token += *it;
	}

	// Last token
	res.push_back(token);

	return res;
}

////////////////////
// Tokenizes the string buffer by blank the specified character
std::vector<std::string> StringBuf::splitBy(char c)
{
	std::vector<std::string> res;
	std::string token;
	for (std::string::iterator it = sStr.begin(); it != sStr.end(); it++)  {
		if (*it == c)  {
			res.push_back(token); // Add the token
			token = "";
			continue;
		}

		token += *it;
	}

	// Last token
	res.push_back(token);

	return res;
}

////////////////////////
// Reads until the specified character and skips it
std::string StringBuf::readUntil(char c)
{
	std::string res;
	while (!atEnd() && *tPos != c)  {
		res += *tPos;
		incPos();
	}

	// Skip the breaking character
	if (!atEnd())
		incPos();

	return res;
}

////////////////////////
// Reads until one of the characters specified in char_array, NOT until the string in char_array
std::string StringBuf::readUntil(const std::string& char_array)
{
	std::string res;
	while (!atEnd() && char_array.find(*tPos) == std::string::npos)  {
		res += *tPos;
		incPos();
	}

	// Skip the breaking character
	if (!atEnd())
		incPos();

	return res;
}

/////////////////////////
// Reads the specified number of bytes from the buf
std::string StringBuf::read(size_t num)
{
	std::string::iterator oth = tPos;
	SafeAdvance(tPos, num, sStr.end());
	return std::string(tPos, oth);
}

///////////////////////
// Skips any blank characters
size_t StringBuf::skipBlank()
{
	size_t res = 0;
	while (!atEnd() && isspace((uchar)(*tPos))) {
		++res;
		incPos();
	}
	return res;
}

/////////////////////
// Converts the buffer to lower case
void StringBuf::toLower()
{
	stringlwr(sStr);
}

/////////////////////
// Prints the contents to stdout
void StringBuf::debugPrint()
{
	std::cout << sStr << std::endl << std::endl;
}
