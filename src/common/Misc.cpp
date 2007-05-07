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

#include <iostream>
#include <assert.h>

#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "InputEvents.h"
#include "StringUtils.h"


// Random Number list
#include "RandomNumberList.h"



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

	for(x=(int)pos.x-2; x<=(int)pos.x+2; x++) {
		// Clipping
		if(x<0)	continue;
		if((uint)x>=cMap->GetWidth())	break;

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




void printf(const std::string& txt) {
	std::cout << txt << std::flush;
// Print out to debug pane
#ifdef _MSC_VER
	// TODO: this is wrong!
	// 1. it behaves different from printf (which is not acceptable)
	// 2. printf is not for debugging use but for printing something on std
	// 3. for debugging, we should create and use some common function
	OutputDebugStringA(txt.c_str());
#endif
}


///////////////////
// Return a fixed random number
float GetFixedRandomNum(uchar index)
{
	return RandomNumbers[index];
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

