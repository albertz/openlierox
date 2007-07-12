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


#ifndef __TIMER_H__
#define __TIMER_H__

#include <string>
#include <SDL/SDL.h>

///////////////////
// Get the number of milliseconds since SDL started the timer
inline float	GetMilliSeconds(void) { return (float)SDL_GetTicks() * 0.001f; }


int				GetFPS(void);
std::string		GetTime();




#endif  //  __TIMER_H__
