/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __INPUTEVENTS_H__
#define __INPUTEVENTS_H__

#include <vector>
#include "Unicode.h"
#include "Event.h"


// application has focus
extern	bool			bActivated;
extern	bool			bDeactivated;


#define     MAX_KEYQUEUE        32
#define     MAX_MOUSEBUTTONS    8 // SDL_GetMouseState returns UInt8 bitmask

// State of modifier keys (ctrl, alt and shift)
struct ModifiersState  { 
	ModifiersState() { clear(); }
	void clear()  { bShift = bCtrl = bAlt = bSuper = false; }

	bool bShift;
	bool bCtrl;
	bool bAlt;
	bool bSuper;
};

struct KeyboardEvent {
	int sym;
    UnicodeChar ch;
	bool down;
	ModifiersState state;
};

struct MouseEvent {
	int x, y;
	int button;
	bool down;
};

// Keyboard structure
// HINT: KeyDown is the state of the keyboard
// KeyUp is like an event and will only be true once
struct keyboard_t {
	Uint8	*keys;
	UnicodeChar	KeyUp[SDLK_LAST];
	UnicodeChar	KeyDown[SDLK_LAST];
    int     queueLength;
    KeyboardEvent keyQueue[MAX_KEYQUEUE];
};


// Mouse structure
struct mouse_t {
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
	
	std::vector<MouseEvent> mouseQueue;
};

enum MouseButton  {
	mbLeft,
	mbRight,
	mbMiddle,
	mbExtra1,
	mbExtra2
};

MouseButton SDLButtonToMouseButton(int sdlbut);

typedef Event<SDL_Event*> SDLEvent;

// All main SDL events. You can add your own global handlers here. Though that should
// not be needed in most cases.
// If you want to add a user event, DON'T add the handler here. Use SendSDLUserEvent()
// and your event will get called automatically. 
extern SDLEvent sdlEvents[SDL_NUMEVENTS];


void 		InitEventSystem();
bool		ProcessEvents(); // returns false if no new event
bool		WaitForNextEvent(); // waits for next event and handles all of then; returns false if no new event

keyboard_t	*GetKeyboard();
mouse_t		*GetMouse();
SDL_Event	*GetEvent();
ModifiersState *GetCurrentModstate();

class CInput;
void		RegisterCInput(CInput* input);
void		UnregisterCInput(CInput* input);

bool		ApplicationHasFocus();


struct SDLUserEventData {
	void* data;
	SDLUserEventData(void* d = NULL) : data(d) {}
};

typedef Event<SDLUserEventData> SDLUserEvent;

inline void SendSDLUserEvent(SDLUserEvent* event, SDLUserEventData data) {
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = 0;
	ev.user.data1 = event;
	ev.user.data2 = data.data;
	SDL_PushEvent(&ev);
}


#endif
