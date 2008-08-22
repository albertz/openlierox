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

#include <SDL.h>

// OLX user events
enum	{ 
	SDL_USEREVENT_NET_ACTIVITY = SDL_USEREVENT + 1,
	SDL_USEREVENT_TIMER,
	SDL_USEREVENT_REPAINT,
	SDL_USEREVENT_ADDWIDGET,
	SDL_USEREVENT_DESTROYWIDGET
};

inline void SendEvent(int type, void *dat1, void *dat2, int code = 0)
{
	SDL_Event ev;
	ev.type = type;
	ev.user.code = code;
	ev.user.data1 = dat1;
	ev.user.data2 = dat2;
	SDL_PushEvent(&ev);
}

#endif // __SDLEVENTS_H__
