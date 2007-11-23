/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Timer
// Created 12/11/01
// Jason Boettcher


#include "LieroX.h"

#include <time.h>
#include "Timer.h"

int		Frames = 0;
float	OldFPSTime = 0;
int		Fps = 0;


///////////////////
// Get the frames per second count
int GetFPS(void)
{
	Frames++;

	if(GetMilliSeconds() - OldFPSTime >= 1.0f) {
		OldFPSTime = GetMilliSeconds();
		Fps = Frames;
		Frames = 0;
	}

	return Fps;
}

///////////////////
// Get the actual time
std::string GetTime()
{
	static char cTime[100];
	time_t t;
	time(&t);
	struct tm* tp;
	tp = localtime(&t);
	strftime(cTime, 26, "%Y-%m-%d-%a-%H-%M-%S", tp);
	return cTime;
}
