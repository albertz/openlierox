/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Misclelaneous functions
// Created 18/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"




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
	static char buf[1024];
	va_list arg;
	
	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	printf(buf);
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
// TODO: buffer unsafe
bool replace(char *text, const char *what, const char *with, char *result)
{
  bool ret = false;

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
// Replace a string in text, returns result, replaces maximally max occurences
char *replacemax(char *text, char *what, char *with, char *result, int max)
{
	if(result != text)
		strcpy(result,text);

  int pos = (int) (strstr(result,what)-result);
  int occurences = 0;
  int check_from = 0;
  while (strstr(result+check_from,what) != NULL && occurences <= max)  {
	int length = strlen(result)-strlen(what)+strlen(with);
	memmove(result+pos+strlen(with),result+pos,strlen(result)-pos+strlen(with));
	strncpy(result+pos,with,strlen(with));
	check_from = pos+strlen(with);
	memmove(result+pos+strlen(with),result+pos+strlen(with)+strlen(what),strlen(result)-(pos+strlen(with)+strlen(what))+1);
	pos = (int) (strstr(result+check_from,what)-result);
	*(result+length) = 0;
	occurences++;
  }

  return result;
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
void replace(std::string& text, std::string what, std::string with) {
	size_t pos = 0;
	size_t what_len = what.length();
	size_t with_len = with.length();
	while((pos = text.find(what, pos)) != std::string::npos) {
		text.replace(pos, what_len, with);
		pos += with_len;
	}
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
// Gets the string <beginning of text,searched character)
void ReadUntil(const char *text, char until_character, char *result, size_t reslen)
{
	size_t i = 0;
	for(; i < reslen-1 && text[i] != until_character && text[i] != '\0'; i++)
		result[i] = text[i];
	result[i] = '\0';
}

//////////////////
// Converts a string to a colour
Uint32 StrToCol(char *str)
{
	char *org_val = NULL;
	char tmp[3];
	int r,g,b;
	tmp[2] = 0;  // Third character is terminating

	org_val = str; // Save the original pointer

	// By default return pink
	if(!str)
		return MakeColour(255,0,255);
	if(strlen(str) < 6)
		return MakeColour(255,0,255);

	// Ignore the # character
	if (*str == '#')
		str++;

	// Check again
	if(strlen(str) < 6)
		return MakeColour(255,0,255);

	// R value
	strncpy(tmp,str,2);
	strlwr(tmp);
	r = MIN((int)strtol(tmp,NULL,16),255);

	// G value
	strncpy(tmp,str+2,2);
	strlwr(tmp);
	g = MIN((int)strtol(tmp,NULL,16),255);

	// B value
	strncpy(tmp,str+4,2);
	strlwr(tmp);
	b = MIN((int)strtol(tmp,NULL,16),255);

	// Make the colour
	Uint32 result = MakeColour((Uint8)r,(Uint8)g,(Uint8)b);

	// Restore the original value
	str = org_val;

	return result;
}


// ==============================
//
// Useful XML functions	
//
// ==============================

///////////////////
// Get an integer from the specified property
int xmlGetInt(xmlNodePtr Node, const char *Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name);
	if(!sValue)
		return 0;
	int result = atoi((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified property
float xmlGetFloat(xmlNodePtr Node, const char *Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name);
	if (!sValue)
		return 0;
	float result = (float)atof((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified property
Uint32 xmlGetColour(xmlNodePtr Node, const char *Name)
{
	xmlChar *sValue;

	// Get the value
	sValue = xmlGetProp(Node,(const xmlChar *)Name);

	Uint32 result = StrToCol((char *)sValue);

	xmlFree(sValue);
	return result;
}

/////////////////
// Replaces all the escape characters with html entities
void xmlEntities(char *text)
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

