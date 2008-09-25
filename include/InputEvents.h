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

// Override this class and add your listener to using the AddEventListener class
class EventListener  { public:
	virtual void OnEvent(SDL_Event *ev) = 0;
	virtual ~EventListener() {}
};

void		AddEventListener(EventListener *lst);
void		RemoveEventListener(EventListener *lst);


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

#endif
