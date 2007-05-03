/*
	OpenLieroX

	string utilities
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/


#include "StringUtils.h"
#include "Utils.h"
#include "GfxPrimitives.h" // for MakeColour
#include "LieroX.h" // for tLX

void StripQuotes(UCString& str)
{
	if(str.size() > 0 && str[0] == '\"')  {
		str.erase(0, 1);
	}

	if(str.size() > 0 && str[str.size()-1] == '\"')  {
		str.erase(str.length()-1);
	}
}


///////////////////
// Trim the leading & ending spaces from a string
void TrimSpaces(UCString& szLine) {
	size_t n = 0;
	UCString::iterator p;
	for(p = szLine.begin(); p != szLine.end(); p++, n++)
		if(!isspace(*p) || isgraph(*p)) break;
	if(n>0) szLine.erase(0,n);

	n = 0;
	UCString::reverse_iterator p2;
	for(p2 = szLine.rbegin(); p2 != szLine.rend(); p2++, n++)
		if(!isspace(*p2) || isgraph(*p2)) break;
	if(n>0) szLine.erase(szLine.size()-n);
}


///////////////////
// Replace a string in text, returns true, if something was replaced
bool replace(const UCString& text, const UCString& what, const UCString& with, UCString& result)
{
	result = text;
	return replace(result, what, with);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
UCString replacemax(const UCString& text, const UCString& what, const UCString& with, UCString& result, int max)
{
	result = text;

	size_t pos = 0;
	size_t what_len = what.length();
	size_t with_len = with.length();
	if((pos = result.find(what, pos)) != UCString::npos) {
		result.replace(pos, what_len, with);
		pos += with_len;
	}

	return result;
}

UCString replacemax(const UCString& text, const UCString& what, const UCString& with, int max) {
	UCString result;
	return replacemax(text, what, with, result, max);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
// returns true, if at least one replace was made
bool replace(UCString& text, const UCString& what, const UCString& with) {
	bool one_repl = false;
	size_t pos = 0;
	size_t what_len = what.length();
	size_t with_len = with.length();
	while((pos = text.find(what, pos)) != UCString::npos) {
		text.replace(pos, what_len, with);
		pos += with_len;
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
UCString ReadUntil(const UCString& text, char until_character) {
	size_t pos = 0;
	for(UCString::const_iterator i = text.begin(); i != text.end(); i++, pos++) {
		if(*i == until_character)
			return text.substr(0,pos);
	}
	return text;
}

UCString	ReadUntil(FILE* fp, char until_character) {
	static char buf[256];
	static UCString res;
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
Uint32 StrToCol(const UCString& str) {
	if (str == "")
		return tLX->clPink;

	// Create the temp and copy it there
	static UCString temp;
	temp = str;

	// Is the # character present?
	if (temp[0] == '#') // str != "" here
		temp.erase(0,1);

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

short stringcasecmp(const UCString& s1, const UCString& s2) {
	UCString::const_iterator p1, p2;
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
const std::vector<UCString>& explode(const UCString& str, const UCString& delim) {
	static std::vector<UCString> result;
	result.clear();

	size_t delim_len = delim.size();
	UCString rest = str;
	size_t pos;
	while((pos = rest.find(delim)) != UCString::npos) {
		result.push_back(rest.substr(0,pos));
		rest.erase(0,pos+delim_len);
	}
	result.push_back(rest);

	return result;
}

// reads up to maxlen-1 chars from fp
UCString freadstr(FILE *fp, size_t maxlen) {
	if (!fp) return "";

	static UCString result;
	static char buf[1024];
	size_t ret, c;
	result = "";

	for(size_t len = 0; len < maxlen; len += sizeof(buf)) {
		c = MIN(sizeof(buf), maxlen - len);
		ret = fread(buf, 1, c, fp);
		if(ret > 0)
			result.append(buf,ret);
		if(ret < c)
			break;
	}

	return result;
}


size_t fwrite(const UCString& txt, size_t len, FILE* fp) {
	size_t len_of_txt = MIN(txt.size()+1, len-1);
	size_t ret = fwrite(txt.c_str(), 1, len_of_txt, fp);
	if(ret != len_of_txt)
		return ret;
	for(; len_of_txt < len; len_of_txt++)
		if(fwrite("\n", 1, 1, fp) == 0)
			return len_of_txt;
	return len;
}


size_t findLastPathSep(const UCString& path) {
	size_t slash = path.rfind('\\');
	size_t slash2 = path.rfind('/');
	if(slash == UCString::npos)
		slash = slash2;
	else if(slash2 != UCString::npos)
		slash = MAX(slash, slash2);
	return slash;
}


void stringlwr(UCString& txt) {
	for(UCString::iterator i = txt.begin(); i != txt.end(); i++)
		*i = tolower(*i);
}


bool strincludes(const UCString& str, const UCString& what) {
	return str.find(what) != UCString::npos;
}

UCString GetFileExtension(const UCString& filename) {
	size_t p = filename.rfind('.');
	if(p == UCString::npos) return "";
	return filename.substr(p+1);
}

UCString strip(const UCString& buf, int width)
{
	// TODO: this width depends on tLX->cFont; this is no solution, fix it
	static UCString result;
	result = buf;
	for(int j=result.length()-1; tLX->cFont.GetWidth(result) > width && j>0; j--)
		result.erase(result.length()-1);

	return result;
}


bool stripdot(UCString& buf, int width)
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


void ucfirst(UCString& text)
{
	if (text == "") return;

	text[0] = toupper(text[0]);
	bool wasalpha = isalpha(text[0]) != 0;

	for (UCString::iterator it=text.begin()+1;it != text.end();it++)  {
		if (isalpha(*it))  {
			if (wasalpha)
				*it = tolower(*it);
			else
				*it = toupper(*it);
			wasalpha = true;
		} else {
			wasalpha = false;
		}
	}


}


















































///////////////////
// Strip quotes away from a string
void StripQuotes(char *dest, char *src)
{
	if(!dest || !src)
		return;

	int pos = 0;
	int srclen = strlen(src);
	int length = srclen;

	if(src[0] == '\"') {
		pos = 1;
		length--;
	}

	if(src[srclen-1] == '\"')
		length--;

	strncpy(dest, src+pos, length);
	dest[length] = 0;
}



///////////////////
// Safe string copy routine
void lx_strncpy(char *dest, char *src, int count)
{
/*    while (*src && count--)
	{
		*dest++ = *src++;
	}
	if (count)
		*dest++ = 0; */
	strncpy(dest, src, count); // this is faster
}


///////////////////
// Strip a string down to bare ascii characters
char *StripLine(char *szLine)
{
    // Convert newlines to spaces
    char *s = szLine;
    while( *s++ )
        if(*s=='\n') *s=' ';

    // Trim the spaces
    return TrimSpaces(szLine);
}

///////////////////
// Trim the leading & ending spaces from a string
char *TrimSpaces(char *szLine)
{
    // Remove preceeding spaces
    while( !isgraph(*szLine) && isspace(*szLine) )
        szLine++;

    // Get rid of the ending spaces
	int i;
    for(i=strlen(szLine)-1; i>=0; i--) {
        if( isgraph(szLine[i]) || !isspace(szLine[i]) )
            break;
    }
    szLine[i+1] = '\0';

    return szLine;
}

///////////////////
// Replace a string in text, returns true, if something was replaced
// NOTE: buffer unsafe
// HINT: only for backward compatibility
bool replace(char *text, const char *what, const char *with, char *result)
{
  bool ret = false;

  if (text != result)
	strcpy(result,text);

  int pos = (int) (strstr(result,what)-result); // Position of the string being replaced
  int check_from = 0; // position to check from, avoids infinite replacing when the "with" string is contained in "what"

  // Replace while the "what" string exists in result
  while (strstr(result+check_from,what) != NULL)  {
	ret = true;
	// Make space for "with" string (move the result+pos string strlen(with) characters to right)
	memmove(result+pos+strlen(with),result+pos,strlen(result)-pos+strlen(with));
	// Copy the "with" string into the above created space (without terminating character)
	strncpy(result+pos,with,strlen(with));
	// Update the check_from to avoid circular replacing
	check_from = pos+strlen(with);
	// Delete the original string
	memmove(result+pos+strlen(with),result+pos+strlen(with)+strlen(what),strlen(result)-(pos+strlen(with)+strlen(what))+1);
	// Find position of next occurence
	pos = (int) (strstr(result+check_from,what)-result);
  }

  return ret;
}


///////////////////
// Strips the text to have the specified width, returns buf
char *strip(char *buf, int width)
{
	for(int j=strlen(buf)-1; tLX->cFont.GetWidth(buf) > width && j>0; j--)
		buf[j] = '\0';

	return buf;
}

///////////////////
// Strips the text to have the specified width and adds three dots at it's end, if the text was stripped
bool stripdot(char *buf, int width)
{
	int dotwidth = tLX->cFont.GetWidth("...");
	bool stripped = false;
	for(int j=strlen(buf)-1; tLX->cFont.GetWidth(buf) > width && j>0; j--)  {
		buf[j] = '\0';
		stripped = true;
	}

	if(stripped)  {
		strip(buf,tLX->cFont.GetWidth(buf)-dotwidth);
		strcat(buf,"...");
	}

	return stripped;
}

///////////////////
// Changes the string to have all first letters upper case
// Returns text
// TODO: the parameter will be changed directly; that's bad style
// TODO: remove this function; it should not be used, as we should use UCString everywhere
char *ucfirst(char *text)
{
	size_t i = 0;
	bool make_upper = true;

	for(; text[i] != '\0'; i++) {
		if(text[i] == ' ' || text[i] == '.') {
			make_upper = true;
		} else if(make_upper) {
			text[i] = toupper(text[i]);
			make_upper = false;
		} else {
			text[i] = tolower(text[i]);
		}
	}

	return text;
}
