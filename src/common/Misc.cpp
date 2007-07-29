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


// Tokenise (is that even the right word) a string by spaces
int token (char *input, char *output)
{
	int quotes=0;
	int whitespace=1;
	int j=0;
	for(int i=0;i<256;i++)
	{
		// Check if the string is in quotes
		if(input[i]=='"') {
			quotes=(quotes^1)&1;
			continue;
		}
		// Check for whitespace to end the token
		if(input[i]==' ' || input[i]=='\t' || input[i]=='\n' || input[i]=='\0')
			if(!quotes && !whitespace)
				break;
		// Check for escape (right word?) sequences
		if(input[i]=='\\') {
			if(input[i+1]=='\\')
				output[j]='\\';
			if(input[i+1]=='n')
				output[j]='\n';
			if(input[i+1]=='t')
				output[j]='\t';
			if(input[i+1]=='"')
				output[j]='"';
			i++,j++;
			continue;
		}
		// Remove non-printable characters
		if(input[i]<32)
			continue;
		if(input[i]!=' ')
			whitespace=0;
		// Remove leading whitespace
		if(!whitespace)
			output[j++]=input[i];
	}
	// Null terminate the returned string
	output[j]='\0';
	// Return the offset to check from to get the next token
	return i+1;
}


//
// Thread functions
//

//////////////////
// Gives a name to the thread
// Code taken from Thread Validator help
#ifdef _MSC_VER
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

