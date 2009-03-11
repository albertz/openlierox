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


#include <cstdarg>
#include <cassert>

#include "LieroX.h"
#include "Debug.h"
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
void ConvertTime(TimeDiff time, int *hours, int *minutes, int *seconds)
{
	*seconds = (int)time.seconds();
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



// for GetByteSwapped, declared in EndianSwap.h
// TODO: remove this from here (or not?)
unsigned char byteswap_buffer[16];




void printf(const std::string& txt) {
	notes << txt << flush;
}


///////////////////
// Return a fixed random number
float GetFixedRandomNum(uchar index)
{
	return RandomNumbers[index];
}


