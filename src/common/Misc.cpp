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



///////////////////
// Check for a collision
int CheckCollision(CVec pos, CVec vel, int width, int height, CMap *map)
{
	int		CollisionSide = 0;	
	int		mw = map->GetWidth();
	int		mh = map->GetHeight();
	int		px,py, x,y, w,h;
	int		top,bottom,left,right;

	px=(int)pos.GetX();
	py=(int)pos.GetY();
		
	top=bottom=left=right=0;

	w = width;
	h = height;


	// Hit edges
	if(px-w<0 || py-h<0 || px+w>=mw || py+h>=mh) {

		// Check the collision side
		if(px-w<0)
			CollisionSide |= COL_LEFT;
		if(py-h<0)
			CollisionSide |= COL_TOP;
		if(px+w>=mw)
			CollisionSide |= COL_RIGHT;
		if(py+h>=mh)
			CollisionSide |= COL_BOTTOM;

		return CollisionSide;
	}

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

		
		uchar *pf = map->GetPixelFlags() + y*mw + px-w;

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
		if( (left>right || left>2) && left>1 && vel.GetX() < 0)
			CollisionSide = COL_LEFT;
		
		if( (right>left || right>2) && right>1 && vel.GetX() > 0)
			CollisionSide = COL_RIGHT;

		if(top>1 && vel.GetY() < 0)
			CollisionSide = COL_TOP;

		if(bottom>1 && vel.GetY() > 0)
			CollisionSide = COL_BOTTOM;
	}


	return CollisionSide;
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
	y = MIN((int)pos.GetY(),cMap->GetHeight()-1);
	y = MAX(y,0);

	for(x=(int)pos.GetX()-2;x<=(int)pos.GetX()+2;x++) {
		// Clipping
		if(x<0)	continue;
		if(x>=cMap->GetWidth())	break;

		if(cMap->GetPixelFlag(x,y) & PX_DIRT) {
			Colour = GetPixel(cMap->GetImage(),x,(int)pos.GetY());
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
#ifdef _DEBUG
	char buf[1024];
	va_list arg;
	
	va_start(arg, fmt);
	vsprintf(buf, fmt, arg);
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
	int length = strlen(src);
	
	if(src[0] == '\"') {
		pos = 1;
		length--;
	}
	
	if(src[strlen(src)-1] == '\"')
		length--;

	strncpy(dest, src+pos, length);
	dest[length] = 0;
}


///////////////////
// Safe string copy routine
void lx_strncpy(char *dest, char *src, int count)
{
    while (*src && count--)
	{
		*dest++ = *src++;
	}
	if (count)
		*dest++ = 0;
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
// Replace a string in text, returns result
char *replace(char *text, char *what, char *with, char *result)
{
  strcpy(result,text);

  int pos = (int) (strstr(result,what)-result); // Position of the string being replaced

  // Replace while the "what" string exists in result
  while (strstr(result,what) != NULL)  {
	// Make space for "with" string (move the result+pos string strlen(with) characters to right)
	memmove(result+pos+strlen(with),result+pos,strlen(result)-pos+strlen(with));
	// Copy the "with" string into the above created space (without terminating character)
	strncpy(result+pos,with,strlen(with));
	// Delete the original string
	memmove(result+pos+strlen(with),result+pos+strlen(with)+strlen(what),strlen(result)-(pos+strlen(with)+strlen(what))+1);
	// Find position of next occurence
	pos = (int) (strstr(result,what)-result);
  }

  return result;
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
char *replacemax(char *text, char *what, char *with, char *result, int max)
{
  strcpy(result,text);

  int pos = (int) (strstr(result,what)-result);
  int occurences = 0;
  while (strstr(result,what) != NULL && occurences <= max)  {
	int length = strlen(result)-strlen(what)+strlen(with);
	memmove(result+pos+strlen(with),result+pos,strlen(result)-pos+strlen(with));
	strncpy(result+pos,with,strlen(with));
	memmove(result+pos+strlen(with),result+pos+strlen(with)+strlen(what),strlen(result)-(pos+strlen(with)+strlen(what))+1);
	pos = (int) (strstr(result,what)-result);
	*(result+length) = 0;
	occurences++;
  }

  return result;
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
// TODO: the parameter will be changed directly; that's bad stile
char *ucfirst(char *text)
{
#ifdef WIN32	
	strlwr(text);

	char buf[2]; 
	buf[0] = text[0];
	buf[1] = '\0';
	strupr(buf);

	text[0] = buf[0];

	for(int i=0; i<(int)strlen(text)-1; i++)
		if (*(text+i) == ' ' || *(text+i) == '.')  {
			buf[0] = *(text+i+1);
			buf[1] = '\0';
			strupr(buf);
			*(text+i+1) = buf[0];
		}
#else
	// TODO: ...
#endif
	
	return text;
}


// for GetByteSwapped, declared in defs.h
unsigned char byteswap_buffer[16];

void ByteSwap(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   register char tmp;
   while (i<j)
   {
      tmp = b[i]; b[i] = b[j]; b[j] = tmp;
      i++, j--;
   }
}


// chrcasecmp - like strcasecomp, but for a single char
int chrcasecmp(const char c1, const char c2)
{
	register char buf1[2];
	register char buf2[2];
	buf1[0] = c1; buf1[1] = '\0';
	buf2[0] = c2; buf2[1] = '\0';
	
	return stricmp(&buf1[0], &buf2[0]);
}
