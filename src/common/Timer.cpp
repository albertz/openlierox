/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Timer
// Created 12/11/01
// Jason Boettcher


#include "defs.h"

int		Frames = 0;
float	OldFPSTime = 0;
int		Fps = 0;

///////////////////
// Get the number of milliseconds since SDL started the timer
float GetMilliSeconds(void)
{
	return (float)SDL_GetTicks() * 0.001f;
}


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
void GetTime(char cTime[26])
{
	SYSTEMTIME st = {0};
	::GetSystemTime(&st);
	//char cTime[26];
	sprintf(cTime,"%d-%d-%d-%d-%d-%d-%d-%d",st.wYear,st.wMonth,st.wDay,st.wDayOfWeek,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
}
