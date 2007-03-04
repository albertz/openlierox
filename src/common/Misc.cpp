/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Misclelaneous functions
// Created 18/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"




/*
===========================

    Collision Detection

===========================
*/



class set_col_and_break {
public:
	CVec collision;
	bool hit;

	set_col_and_break() : hit(false) {}
	bool operator()(int x, int y) {
		hit = true;
		collision.x = x;
		collision.y = y;
		return false;
	}
};



///////////////////
// Check for a collision
// HINT: this function is not used at the moment; and it is incomplete...
int CheckCollision(float dt, CVec pos, CVec vel, uchar checkflags, CMap *map)
{
/*	set_col_and_break col_action;
	col_action = fastTraceLine(trg, pos, map, checkflags, col_action);
	if(col_action.hit) {

	}*/
	assert(false);
	return 0;

/*	int		CollisionSide = 0;
	int		mw = map->GetWidth();
	int		mh = map->GetHeight();
	int		px,py, x,y, w,h;
	int		top,bottom,left,right;

	px=(int)pos.x;
	py=(int)pos.y;

	top=bottom=left=right=0;

	w = width;
	h = height;

	// Hit edges
	// Check the collision side
	if(px-w<0)
		CollisionSide |= COL_LEFT;
	if(py-h<0)
		CollisionSide |= COL_TOP;
	if(px+w>=mw)
		CollisionSide |= COL_RIGHT;
	if(py+h>=mh)
		CollisionSide |= COL_BOTTOM;
	if(CollisionSide) return CollisionSide;




	for(y=py-h;y<=py+h;y++) {

		// Clipping means that it has collided
		if(y<0)	{
			CollisionSide |= COL_TOP;
			return CollisionSide;
		}
		if(y>=mh) {
			CollisionSide |= COL_BOTTOM;
			return CollisionSide;
		}


		const uchar *pf = map->GetPixelFlags() + y*mw + px-w;

		for(x=px-w;x<=px+w;x++) {

			// Clipping
			if(x<0) {
				CollisionSide |= COL_LEFT;
				return CollisionSide;
			}
			if(x>=mw) {
				CollisionSide |= COL_RIGHT;
				return CollisionSide;
			}

			if(*pf & PX_DIRT || *pf & PX_ROCK) {
				if(y<py)
					top++;
				if(y>py)
					bottom++;
				if(x<px)
					left++;
				if(x>px)
					right++;

				//return Collision(*pf);
			}

			pf++;
		}
	}

	// Check for a collision
	if(top || bottom || left || right) {
		CollisionSide = 0;


		// Find the collision side
		if( (left>right || left>2) && left>1 && vel.x < 0)
			CollisionSide = COL_LEFT;

		if( (right>left || right>2) && right>1 && vel.x > 0)
			CollisionSide = COL_RIGHT;

		if(top>1 && vel.y < 0)
			CollisionSide = COL_TOP;

		if(bottom>1 && vel.y > 0)
			CollisionSide = COL_BOTTOM;
	}


	return CollisionSide; */
}



/*
===========================

         Misc crap

===========================
*/


///////////////////
// Convert a float time into it's hours, minutes & seconds
void ConvertTime(float time, int *hours, int *minutes, int *seconds)
{
	*seconds = (int)time;
	*minutes = *seconds / 60;
	if(*minutes)
	{
		*seconds -= (*minutes * 60);
		*hours = *minutes / 60;
		if(*hours)
			*minutes -= (*hours * 60);
	}
	else
		*hours = 0;
}


///////////////////
// Carve a hole
// Returns the number of dirt pixels carved
int CarveHole(CMap *cMap, CVec pos)
{
	int x,y,n;
	Uint32 Colour = cMap->GetTheme()->iDefaultColour;

	// Go through until we find dirt to throw around
	y = MIN((int)pos.y,cMap->GetHeight()-1);
	y = MAX(y,0);

	for(x=(int)pos.x-2;x<=(int)pos.x+2;x++) {
		// Clipping
		if(x<0)	continue;
		if(x>=cMap->GetWidth())	break;

		if(cMap->GetPixelFlag(x,y) & PX_DIRT) {
			Colour = GetPixel(cMap->GetImage(),x,(int)pos.y);
			for(n=0;n<3;n++)
				SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
			break;
		}
	}

	// Just carve a hole for the moment
	return cMap->CarveHole(3,pos);
}


///////////////////
// Debug printf
// Will only print out if this is a debug build
void d_printf(char *fmt, ...)
{
#ifdef DEBUG
	va_list arg;

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
#endif
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

void StripQuotes(std::string& str)
{
	if (str == "")
		return;

	if (str[0] == '\"')  {
		str.erase((unsigned int)0);
	}

	if (str[str.length()-1] == '\"')  {
		str.erase(str.length()-1);
	}
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
// Returns true if the mouse is inside a rectangle
bool MouseInRect(int x, int y, int w, int h)
{
    mouse_t *m = GetMouse();

    if( m->X >= x && m->X <= x+w ) {
        if( m->Y >= y && m->Y <= y+h ) {
            return true;
        }
    }

    return false;
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
	size_t what_len = what.length();
	size_t with_len = with.length();
	while((pos = text.find(what, pos)) != std::string::npos) {
		text.replace(pos, what_len, with);
		pos += with_len;
		one_repl = true;
	}
	return one_repl;
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

std::string strip(const std::string& buf, int width)
{
	static std::string result;
	result = buf;
	for(int j=result.length()-1; tLX->cFont.GetWidth(result) > width && j>0; j--)
		result.erase(result.length()-1);

	return result;
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

bool stripdot(std::string& buf, int width)
{
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

///////////////////
// Changes the string to have all first letters upper case
// Returns text
// TODO: the parameter will be changed directly; that's bad style
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

void ucfirst(std::string& text)
{
	if (text == "") return;

	text[0] = toupper(text[0]);
	bool wasalpha = isalpha(text[0]) != 0;

	for (std::string::iterator it=text.begin()+1;it != text.end();it++)  {
		if (isalpha(*it))  {
			if (wasalpha)
				*it = toupper(*it);
			else
				*it = tolower(*it);
			wasalpha = true;
		} else {
			wasalpha = false;
		}
	}


}


// for GetByteSwapped, declared in defs.h
unsigned char byteswap_buffer[16];

void ByteSwap(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   register unsigned char tmp;
   while (i<j)
   {
      tmp = b[i]; b[i] = b[j]; b[j] = tmp;
      i++, j--;
   }
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
			return text.substr(0,pos);
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
Uint32 StrToCol(const std::string& str) {
	if (str == "")
		return tLX->clPink;

	// Create the temp and copy it there
	static std::string temp;
	temp = str;

	// Is the # character present?
	if (temp[0] == '#')
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
std::vector<std::string>& explode(const std::string& str, const std::string& delim) {
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

// reads a c-string from the file
std::string freadcstr(FILE *fp, size_t maxlen) {
	if (!fp) return "";

	static std::string result;
	static char buf[1024];
	size_t ret, c;
	result = "";

	for(size_t len = 0; len < maxlen; len += sizeof(buf)) {
		c = MIN(sizeof(buf), maxlen - len);
		ret = fread(buf, 1, c, fp);
		if(ret > 0)  {
			buf[ret] = '\0';
			size_t len = result.length();
			result += buf;
			if (result.length() - len < ret)
				break;
		}
		if(ret < c)
			break;
	}

	return result;
}

// reads maxlen-1 chars from fp
std::string freadstr(FILE *fp, size_t maxlen) {
	if (!fp) return "";

	static std::string result;
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

void printf(const std::string& txt) {
	printf("%s", txt.c_str());
}



// ==============================
//
// Useful XML functions
//
// ==============================

///////////////////
// Get an integer from the specified property
int xmlGetInt(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if(!sValue)
		return 0;
	int result = atoi((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified property
float xmlGetFloat(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if (!sValue)
		return 0;
	float result = (float)atof((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified property
Uint32 xmlGetColour(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;

	// Get the value
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());

	Uint32 result = StrToCol((char*)sValue);

	xmlFree(sValue);
	return result;
}

/////////////////
// Replaces all the escape characters with html entities
void xmlEntities(std::string& text)
{
	replace(text,"\"","&quot;",text);  // "
	replace(text,"'", "&apos;",text);  // '
	replace(text,"&", "&amp;", text);  // &
	replace(text,"<", "&lt;",  text);  // <
	replace(text,">", "&gt;",  text);  // >
}

//
// Thread functions
//

//////////////////
// Gives a name to the thread
// Code taken from Thread Validator help
#ifdef WIN32
void nameThread(const DWORD threadId, const char *name)
{
   // You can name your threads by using the following code.
   // Thread Validator will intercept the exception and pass it along (so if you are also running
   // under a debugger the debugger will also see the exception and read the thread name

   // NOTE: this is for 'unmanaged' C++ ONLY!

   #define MS_VC_EXCEPTION 0x406D1388
   #define BUFFER_LEN      16

   typedef struct tagTHREADNAME_INFO
   {
      DWORD   dwType;   // must be 0x1000
      LPCSTR   szName;   // pointer to name (in user address space)
               // buffer must include terminator character
      DWORD   dwThreadID;   // thread ID (-1 == caller thread)
      DWORD   dwFlags;   // reserved for future use, must be zero
   } THREADNAME_INFO;

   THREADNAME_INFO   ThreadInfo;
   char         szSafeThreadName[BUFFER_LEN];   // buffer can be any size,
                           // just make sure it is large enough!

   memset(szSafeThreadName, 0, sizeof(szSafeThreadName));   // ensure all characters are NULL before
   strncpy(szSafeThreadName, name, BUFFER_LEN - 1);   // copying name
   //szSafeThreadName[BUFFER_LEN - 1] = '\0';

   ThreadInfo.dwType = 0x1000;
   ThreadInfo.szName = szSafeThreadName;
   ThreadInfo.dwThreadID = threadId;
   ThreadInfo.dwFlags = 0;

   __try
   {
      RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadInfo) / sizeof(DWORD), (DWORD*)&ThreadInfo);
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
      // do nothing, just catch the exception so that you don't terminate the application
   }
}
#endif

