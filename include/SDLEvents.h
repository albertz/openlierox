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
	SDL_USEREVENT_NET_ACTIVITY,
	SDL_USEREVENT_TIMER
//	SDL_USEREVENT_REPAINT,
//	SDL_USEREVENT_ADDWIDGET,
//	SDL_USEREVENT_DESTROYWIDGET
};


#endif // __SDLEVENTS_H__
