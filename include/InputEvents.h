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
#include "EventQueue.h"


// application has focus
// These vars are like events, they got true once in a frame when the specific event occured.
// TODO: remove them
extern	bool			bActivated;
extern	bool			bDeactivated;


#define     MAX_KEYQUEUE        32
#define     MAX_MOUSEBUTTONS    8 // SDL_GetMouseState returns UInt8 bitmask

// State of modifier keys (ctrl, alt and shift)
struct ModifiersState  { 
	ModifiersState() { clear(); }
	void clear()  { bShift = bCtrl = bAlt = bGui = false; }

	bool bShift;
	bool bCtrl;
	bool bAlt;
	bool bGui;
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
	bool	KeyUp[SDL_NUM_SCANCODES];
	bool	KeyDown[SDL_NUM_SCANCODES];
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
extern std::map<SDL_EventType, SDLEvent> sdlEvents;

extern bool processedEvent;

void 		InitEventSystem();
void		ShutdownEventSystem();
void		ProcessEvents(); // returns false if no new event

bool		EventSystemInited();
bool		IsWaitingForEvent();
void		WakeupIfNeeded();

// Should be called on SDL_SYSWMEVENT from main thread
void		EvHndl_SysWmEvent_MainThread(SDL_Event* ev);

keyboard_t	*GetKeyboard();
mouse_t		*GetMouse();
SDL_Event	*GetEvent();
ModifiersState *GetCurrentModstate();

bool		WasKeyboardEventHappening(int sym, bool down = true);


class CInput;
void		RegisterCInput(CInput* input);
void		UnregisterCInput(CInput* input);

bool		ApplicationHasFocus();


extern Event<> onDummyEvent;


#endif
