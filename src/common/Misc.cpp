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


#include <stdarg.h>
#include <iostream>
#include <assert.h>

#include "LieroX.h"
#include "GfxPrimitives.h"
#include "InputEvents.h"
#include "StringUtils.h"


// Random Number list
#include "RandomNumberList.h"







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
// Returns true if the mouse is inside a rectangle
bool MouseInRect(int x, int y, int w, int h)
{
    mouse_t *m = GetMouse();
	return ((m->X >= x) && (m->X <= x+w))  &&
		   ((m->Y >= y) && (m->Y <= y+h));
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



// for GetByteSwapped, declared in EndianSwap.h
// TODO: remove this from here (or not?)
unsigned char byteswap_buffer[16];




void printf(const std::string& txt) {
	std::cout << txt << std::flush;
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
#ifdef _MSC_VER
void nameThread(const DWORD threadId, const std::string& name)
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
   strncpy(szSafeThreadName, name.c_str(), BUFFER_LEN - 1);   // copying name
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

