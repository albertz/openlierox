/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// SDL user events for OLX
// Created 28/7/08
// Karel Petranek

#ifndef __SDLEVENTS_H__
#define __SDLEVENTS_H__

#include "Event.h"
#include <SDL.h>


// TODO: remove these, use default SDLUserEvent technic instead
// OLX user events
enum SDLUserEventNr { 
	SDL_USEREVENT_TIMER = SDL_USEREVENT + 1
};


#endif // __SDLEVENTS_H__
