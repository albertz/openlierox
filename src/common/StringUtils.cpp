/*
	OpenLieroX

	string utilities
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifdef _MSC_VER
#pragma warning(disable: 4786)  // WARNING: identifier XXX was truncated to 255 characters in the debug info...
#endif

#include "LieroX.h" // for tLX
#include "StringUtils.h"
#include "Utils.h"
#include "GfxPrimitives.h" // for MakeColour
#include "CFont.h" // for CFont
#include "ConfigHandler.h" // for getting color value from data/frontend/colours.cfg

void StripQuotes(std::string& str)
{
	if(str.size() > 0 && str[0] == '\"')  {
		str.erase(0, 1);
	}

	if(str.size() > 0 && str[str.size() - 1] == '\"')  {
		str.erase(str.size() - 1);
	}
}


///////////////////
// Trim the leading & ending spaces from a string
void TrimSpaces(std::string& szLine) {
	size_t n = 0;
	std::string::iterator p;
	for(p = szLine.begin(); p != szLine.end(); p++, n++)
		if(!isspace(*p) || isgraph(*p)) break;
	if(n>0) szLine.erase(0,n);

	n = 0;
	std::string::reverse_iterator p2;
	for(p2 = szLine.rbegin(); p2 != szLine.rend(); p2++, n++)
		if(!isspace(*p2) || isgraph(*p2)) break;
	if(n>0) szLine.erase(szLine.size()-n);
}


///////////////////
// Replace a string in text, returns true, if something was replaced
bool replace(const std::string& text, const std::string& what, const std::string& with, std::string& result)
{
	result = text;
	return replace(result, what, with);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max)
{
	result = text;

	size_t pos = 0;
	size_t what_len = what.length();
	size_t with_len = with.length();
	if((pos = result.find(what, pos)) != std::string::npos) {
		result.replace(pos, what_len, with);
		pos += with_len;
	}

	return result;
}

std::string replacemax(const std::string& text, const std::string& what, const std::string& with, int max) {
	std::string result;
	return replacemax(text, what, with, result, max);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
// returns true, if at least one replace was made
bool replace(std::string& text, const std::string& what, const std::string& with) {
	bool one_repl = false;
	size_t pos = 0;
	while((pos = text.find(what, pos)) != std::string::npos) {
		text.replace(pos, what.length(), with);
		pos += with.length();
		one_repl = true;
	}
	return one_repl;
}

// chrcasecmp - like strcasecomp, but for a single char
int chrcasecmp(const char c1, const char c2)
{
	return (tolower(c1) == tolower(c2));
}

//////////////////
// Gets the string [beginning of text,searched character)
std::string ReadUntil(const std::string& text, char until_character) {
	size_t pos = 0;
	for(std::string::const_iterator i = text.begin(); i != text.end(); i++, pos++) {
		if(*i == until_character)
			return text.substr(0, pos);
	}
	return text;
}

std::string	ReadUntil(FILE* fp, char until_character) {
	static char buf[256];
	static std::string res;
	res = "";
	size_t buf_pos = 0;
	while(true) {
		if(fread(&buf[buf_pos],1,1,fp) == 0 || buf[buf_pos] == until_character) {
			res.append(buf,buf_pos);
			break;
		}
		buf_pos++;
		if(buf_pos >= sizeof(buf)) {
			buf_pos = 0;
			res.append(buf,sizeof(buf));
		}
	}

	return res;
}


//////////////////
// Converts a string to a colour
// HINT: it uses MakeColour
Uint32 StrToCol(const std::string& str) {
	if (str == "")
		return tLX->clPink;

	// Create the temp and copy it there
	static std::string temp;
	temp = str;

	// Is the # character present?
	if (temp[0] == '#') // str != "" here
		temp.erase(0,1);
	else	// Try searching color in data/frontend/colours.cfg
	{
		Uint32 col;
		ReadColour( "data/frontend/colours.cfg", "Colours", temp, &col, tLX->clPink );
		return col;
	};

	// Check
	if (temp.length() < 6)
		return tLX->clPink;

	// Convert to lowercase
	stringlwr(temp);

	// Convert
	Uint8 r,g,b;
	r = MIN(from_string<int>(temp.substr(0,2),std::hex),255);
	g = MIN(from_string<int>(temp.substr(2,2),std::hex),255);
	b = MIN(from_string<int>(temp.substr(4,2),std::hex),255);

	return MakeColour(r,g,b);
}

short stringcasecmp(const std::string& s1, const std::string& s2) {
	std::string::const_iterator p1, p2;
	p1 = s1.begin();
	p2 = s2.begin();
	short dif;
	while(true) {
		if(p1 == s1.end()) {
			if(p2 == s2.end())
				return 0;
			// not at end of s2
			return -1; // s1 < s2
		}
		if(p2 == s2.end())
			// not at end of s1
			return 1; // s1 > s2

		dif = (short)tolower(*p1) - (short)tolower(*p2);
		if(dif != 0) return dif; // dif > 0  <=>  s1 > s2

		p1++; p2++;
	}
}

// HINT: it returns a reference
// TODO: perhaps it is not the best way to return a std::vector; but I still have to think about it how to do better (perhaps a functional solution...)
const std::vector<std::string>& explode(const std::string& str, const std::string& delim) {
	static std::vector<std::string> result;
	result.clear();

	size_t delim_len = delim.size();
	std::string rest = str;
	size_t pos;
	while((pos = rest.find(delim)) != std::string::npos) {
		result.push_back(rest.substr(0,pos));
		rest.erase(0,pos+delim_len);
	}
	result.push_back(rest);

	return result;
}

// reads up to maxlen-1 chars from fp
void freadstr(std::string& result, size_t maxlen, FILE *fp) {
	if (!fp) return;

	static char buf[1024];
	size_t ret, c;
	result = "";

	for(size_t len = 0; len < maxlen; len += sizeof(buf)) {
		c = MIN(sizeof(buf), maxlen - len);
		ret = fread(buf, 1, c, fp);
		if(ret > 0)
			result.append(buf, ret);
		if(ret < c)
			break;
	}
}


size_t fwrite(const std::string& txt, size_t len, FILE* fp) {
	size_t len_of_txt = MIN(txt.size()+1, len-1);
	size_t ret = fwrite(txt.c_str(), 1, len_of_txt, fp);
	if(ret != len_of_txt)
		return ret;
	for(; len_of_txt < len; len_of_txt++)
		if(fwrite("\0", 1, 1, fp) == 0)
			return len_of_txt;
	return len;
}


size_t findLastPathSep(const std::string& path) {
	size_t slash = path.rfind('\\');
	size_t slash2 = path.rfind('/');
	if(slash == std::string::npos)
		slash = slash2;
	else if(slash2 != std::string::npos)
		slash = MAX(slash, slash2);
	return slash;
}


void stringlwr(std::string& txt) {
	for(std::string::iterator i = txt.begin(); i != txt.end(); i++)
		*i = tolower(*i);
}


bool strincludes(const std::string& str, const std::string& what) {
	return str.find(what) != std::string::npos;
}

std::string GetFileExtension(const std::string& filename) {
	size_t p = filename.rfind('.');
	if(p == std::string::npos) return "";
	return filename.substr(p+1);
}

std::string strip(const std::string& buf, int width)
{
	// TODO: this width depends on tLX->cFont; this is no solution, fix it
	static std::string result;
	result = buf;
	for(int j=result.length()-1; tLX->cFont.GetWidth(result) > width && j>0; j--)
		result.erase(result.length()-1);

	return result;
}


bool stripdot(std::string& buf, int width)
{
	// TODO: this width depends on tLX->cFont; this is no solution, fix it
	int dotwidth = tLX->cFont.GetWidth("...");
	bool stripped = false;
	for(int j=buf.length()-1; tLX->cFont.GetWidth(buf) > width && j>0; j--)  {
		buf.erase(buf.length()-1);
		stripped = true;
	}

	if(stripped)  {
		buf = strip(buf,tLX->cFont.GetWidth(buf)-dotwidth);
		buf += "...";
	}

	return stripped;
}



void ucfirst(std::string& text)
{
	if (text == "") return;

	text[0] = toupper(text[0]);
	bool wasalpha = isalpha(text[0]) != 0;

	for (std::string::iterator it=text.begin()+1;it != text.end();it++)  {
		if (isalpha(*it))  {
			if (wasalpha)
				*it = tolower((uchar)*it);
			else
				*it = toupper((uchar)*it);
			wasalpha = true;
		} else {
			wasalpha = false;
		}
	}


}

//////////////////
// Splits the str in two pieces, part before space and part after space, if no space is found, the second string is empty
// Used internally by splitstring
static void split_by_space(const std::string& str, std::string& before_space, std::string& after_space)
{
	size_t spacepos = str.rfind(' ');
	if (spacepos == std::string::npos || spacepos == str.size() - 1 || str == "")  {
		before_space = str;
		after_space = "";
	} else {
		before_space = str.substr(0, spacepos);
		after_space = str.substr(spacepos + 1); // exclude the space
	}
}

//////////////////////
// Splits the string to pieces that none of the pieces can be longer than maxlen and wider than maxwidth
// TODO: perhaps it is not the best way to return a std::vector; but I still have to think about it how to do better (perhaps a functional solution...)
const std::vector<std::string>& splitstring(const std::string& str, size_t maxlen, size_t maxwidth, CFont& font)
{
	static std::vector<std::string> result;
	result.clear();
	std::string::const_iterator it = str.begin();
	std::string::const_iterator last_it = str.begin();
	size_t i = 0;
	std::string token;

	for (it++; it != str.end(); i += IncUtf8StringIterator(it, str.end()))  {

		// Check for maxlen
		if (i > maxlen)  {
			std::string before_space;
			split_by_space(token, before_space, token);
			result.push_back(before_space);
			i = token.size();
		}

		// Check for maxwidth
		if ((size_t)font.GetWidth(token) <= maxwidth && (size_t)font.GetWidth(token + std::string(last_it, it)) > maxwidth) {
			std::string before_space;
			split_by_space(token, before_space, token);
			result.push_back(before_space);
			i = token.size();
		}

		// Add the current bytes to token
		token += std::string(last_it, it);

		last_it = it;
	}

	 // Last token
	result.push_back(token + std::string(last_it, it));

	return result;
}


/////////////////////////
// Find a substring in a string
// WARNING: does NOT support UTF8, use Utf8StringCaseFind instead
size_t stringcasefind(const std::string& text, const std::string& search_for)
{
	if (text.size() == 0 || search_for.size() == 0 || search_for.size() > text.size())
		return std::string::npos;

	std::string::const_iterator it1 = text.begin();
	std::string::const_iterator it2 = search_for.begin();

	size_t number_of_same = 0;
	size_t result = 0;

	// Go through the text
	while (it1 != text.end())  {
		char c1 = (char)tolower((uchar)*it1);
		char c2 = (char)tolower((uchar)*it2);

		// The two characters are the same
		if (c1 == c2)  {
			number_of_same++;  // If number of same characters equals to the size of the substring, we've found it!
			if (number_of_same == search_for.size())
				return result - number_of_same + 1;
			it2++;
		} else {
			number_of_same = 0;
			it2 = search_for.begin();
		}

		result++;
		it1++;
	}

	return std::string::npos; // Not found
}

/////////////////////////
// Find a substring in a string, starts searching from the end of the text
// WARNING: does NOT support UTF8
size_t stringcaserfind(const std::string& text, const std::string& search_for)
{
	// HINT: simply the above one with reverse iterators

	if (text.size() == 0 || search_for.size() == 0 || search_for.size() > text.size())
		return std::string::npos;

	std::string::const_reverse_iterator it1 = text.rbegin();
	std::string::const_reverse_iterator it2 = search_for.rbegin();

	size_t number_of_same = 0;
	size_t result = 0;

	// Go through the text
	while (it1 != text.rend())  {
		char c1 = (char)tolower((uchar)*it1);
		char c2 = (char)tolower((uchar)*it2);

		// The two characters are the same
		if (c1 == c2)  {
			number_of_same++;  // If number of same characters equals to the size of the substring, we've found it!
			if (number_of_same == search_for.size())
				return text.size() - result - 1;
			it2++;
		} else {
			number_of_same = 0;
			it2 = search_for.rbegin();
		}

		result++;
		it1++;
	}

	return std::string::npos; // Not found
}

