/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __INPUTEVENTS_H__
#define __INPUTEVENTS_H__

#include "Unicode.h"



#define     MAX_KEYQUEUE        32


class KeyboardEvent {
public:
    UnicodeChar ch;
	bool down;
};

// Keyboard structure
class keyboard_t { public:
	Uint8	*keys;
	UnicodeChar	KeyUp[SDLK_LAST];
	UnicodeChar	KeyDown[SDLK_LAST];
    int     queueLength;
    KeyboardEvent keyQueue[MAX_KEYQUEUE];
};


// Mouse structure
class mouse_t { public:
	int		X,Y;
	int		deltaX,deltaY;
	int		Button;

	// HINT: the following three variables cannot be bool
	//       because they contain info about the pressed button (left, right, ...)
	//		 as well
	// TODO: change this immediatly
	int		Up;
	int		Down;
    int     FirstDown;

	bool	WheelUp;
	bool	WheelDown;
	bool	WheelScrollUp;
	bool	WheelScrollDown;
};




void		ProcessEvents(void);
keyboard_t	*GetKeyboard(void);
mouse_t		*GetMouse(void);
SDL_Event	*GetEvent(void);

#endif
