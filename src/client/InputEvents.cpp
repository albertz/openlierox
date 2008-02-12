/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#include <SDL_syswm.h>
#include <iostream>

#include "Clipboard.h"
#include "LieroX.h"
#include "InputEvents.h"
#include "AuxLib.h"
#include "Menu.h"
#include "Timer.h"

using namespace std;


// Keyboard, Mouse, & Event
static keyboard_t	Keyboard;
static mouse_t		Mouse;
static SDL_Event	Event;
static ModifiersState keyModifiersState;

static int         nFocus = true;
bool		bActivated = false;


///////////////////
// Return the keyboard structure
keyboard_t *GetKeyboard(void)
{
	return &Keyboard;
}

///////////////////
// Return the mouse structure
mouse_t *GetMouse(void)
{
	return &Mouse;
}


///////////////////
// Return the event
SDL_Event *GetEvent(void)
{
	// TODO: this should not be used like it is atm because it only returns the last event
	// but in ProcessEvents() could be more than one Event get passed
	return &Event;
}


void InitEventSystem() {
	ProcessEvents();
	for(int k = 0;k<SDLK_LAST;k++)
		GetKeyboard()->KeyUp[k] = false;

	GetMouse()->Button = 0;
	GetMouse()->Down = 0;
	GetMouse()->FirstDown = 0;
	GetMouse()->Up = 0;
	GetMouse()->mouseQueue.reserve(32); // just make space to avoid always reallocation
}

static void ResetCurrentEventStorage() {
    // Clear the queue
    Keyboard.queueLength = 0;

	// Reset mouse wheel
	Mouse.WheelScrollUp = false;
	Mouse.WheelScrollDown = false;
	Mouse.mouseQueue.clear();
	
	// Reset the video mode changed flag here
	if (tLX)
		tLX->bVideoModeChanged = false;

//	for(int k=0;k<SDLK_LAST;k++) {
//		Keyboard.KeyUp[k] = false;
//	}

	bActivated = false;
}

// main function for handling next event
// HINT: we are using the global variable Event here atm
// in both cases where we call this function this is correct
// TODO: though the whole architecture has to be changed later
// but then also GetEvent() has to be changed or removed
static void HandleNextEvent() {
	if (tLX == NULL)
		return;

	switch(Event.type) {
	
	// Quit event
	case SDL_QUIT:
		// Quit
		tLX->bQuitGame = true;
		tLX->bQuitEngine = true;
		tMenu->bMenuRunning = false;
		break;

	// Mouse wheel scroll
	case SDL_MOUSEBUTTONDOWN:
		switch(Event.button.button) {
			case SDL_BUTTON_WHEELUP:
				Mouse.WheelScrollUp = true;
				break;
			case SDL_BUTTON_WHEELDOWN:
				Mouse.WheelScrollDown  = true;
				break;
		}  // switch
	
		{
			MouseEvent mev = { Event.button.x, Event.button.y, Event.button.button, true };
			Mouse.mouseQueue.push_back( mev );
		}
		break;
		
	case SDL_MOUSEBUTTONUP:
		{
			MouseEvent mev = { Event.button.x, Event.button.y, Event.button.button, false };
			Mouse.mouseQueue.push_back( mev );
		}
		break;

	// Activation and deactivation
	case SDL_ACTIVEEVENT:
		if(!(Event.active.state & SDL_APPMOUSEFOCUS))  {
				nFocus = Event.active.gain;
				bActivated = nFocus != 0;

				// HINT: Reset the mouse state - this should avoid the mouse staying pressed
				Mouse.Button = 0;
				Mouse.Down = 0;
				Mouse.FirstDown = 0;
				Mouse.Up = 0;
		}
		break;

	// Keyboard events
	case SDL_KEYUP: case SDL_KEYDOWN:

		// Check the characters
		if(Event.key.state == SDL_PRESSED || Event.key.state == SDL_RELEASED) {

			UnicodeChar input = Event.key.keysym.unicode;
			if (input == 0)
				switch (Event.key.keysym.sym) {
				case SDLK_HOME:
					input = 2;
					break;
				case SDLK_END:
					input = 3;
					break;
				case SDLK_KP0:
				case SDLK_KP1:
				case SDLK_KP2:
				case SDLK_KP3:
				case SDLK_KP4:
				case SDLK_KP5:
				case SDLK_KP6:
				case SDLK_KP7:
				case SDLK_KP8:
				case SDLK_KP9:
				case SDLK_KP_MULTIPLY:
				case SDLK_KP_MINUS:
				case SDLK_KP_PLUS:
				case SDLK_KP_EQUALS:
					input = (uchar) (Event.key.keysym.sym - 208);
					break;
				case SDLK_KP_PERIOD:
				case SDLK_KP_DIVIDE:
					input = (uchar) (Event.key.keysym.sym - 220);
					break;
				case SDLK_KP_ENTER:
					input = '\r';
					break;
				default:
					// nothing
					break;
			}  // switch

			// If we're going to over the queue length, shift the list down and remove the oldest key
			if(Keyboard.queueLength+1 >= MAX_KEYQUEUE) {
				for(int i=0; i<Keyboard.queueLength-1; i++)
					Keyboard.keyQueue[i] = Keyboard.keyQueue[i+1];
				Keyboard.queueLength--;
				printf("warning: keyboard queue full\n");
			}

			KeyboardEvent& kbev = Keyboard.keyQueue[Keyboard.queueLength];
			
			// Key down
			if(Event.type == SDL_KEYDOWN)
				kbev.down = true;

			// Key up
			else if(Event.type == SDL_KEYUP || Event.key.state == SDL_RELEASED)
				kbev.down = false;

			else
				break; // don't save and handle it
			
			// save info
			kbev.ch = input;
			kbev.sym = Event.key.keysym.sym;
			Keyboard.queueLength++;
			
			// handle modifier state
			switch (kbev.sym)  {
			case SDLK_LALT: case SDLK_RALT:
				keyModifiersState.bAlt = kbev.down;
				break;
			case SDLK_LCTRL: case SDLK_RCTRL:
				keyModifiersState.bCtrl = kbev.down;
				break;
			case SDLK_LSHIFT: case SDLK_RSHIFT:
				keyModifiersState.bShift = kbev.down;
				break;
			case SDLK_LSUPER: case SDLK_RSUPER:
				keyModifiersState.bSuper = kbev.down;
				break;
			}
			
			// copy it
			kbev.state = keyModifiersState;
		
			/*
			if(Event.key.state == SDL_PRESSED && Event.key.type == SDL_KEYDOWN)
				// I don't want to track keyrepeats here; but works only for special keys
				cout << tLX->fCurTime << ": pressed key " << kbev.sym << endl; 
			else if(!kbev.down)
				cout << tLX->fCurTime << ": released key " << kbev.sym << endl;
			*/
		
		} else
			printf("strange Event.key.state = %i\n", Event.key.state);
		break;
		
	case SDL_SYSWMEVENT:
		handle_system_event(Event);
		break;
	
	case SDL_USEREVENT_TIMER:
		Timer::handleEvent(Event);	
		break;
		
	default:
		//std::cout << "WARNING: unhandled event " << Event.type << std::endl;
		break;
	}
}

static void HandleMouseState() {
	{
		// Mouse
		int oldX = Mouse.X;
		int oldY = Mouse.Y;
		Mouse.Button = SDL_GetMouseState(&Mouse.X,&Mouse.Y);
		Mouse.deltaX = Mouse.X-oldX;
		Mouse.deltaY = Mouse.Y-oldY;
		Mouse.Up = 0;
		Mouse.FirstDown = 0;
	}
	
    for( int i=0; i<MAX_MOUSEBUTTONS; i++ ) {
		if(!(Mouse.Button & SDL_BUTTON(i)) && Mouse.Down & SDL_BUTTON(i))
			Mouse.Up |= SDL_BUTTON(i);
        if( !(Mouse.Down & SDL_BUTTON(i)) && (Mouse.Button & SDL_BUTTON(i)) )
            Mouse.FirstDown |= SDL_BUTTON(i);
    }

	Mouse.Down = Mouse.Button;

    // SAFETY HACK: If we get any mouse presses, we must have focus
    // TODO: why is this needed? this isn't true for all systems
    // (under Linux it is possible to receive mouse-clicks without having the focus)
    if(Mouse.Down)  {
		if (!nFocus)
			bActivated = true;
        nFocus = true;
	}
}

static void HandleKeyboardState() {	
	// HINT: KeyDown is the state of the keyboard
	// KeyUp is like an event and will only be true once
	
	// Keyboard
	Keyboard.keys = SDL_GetKeyState(NULL);

	// Update the key up's
	for(int k=0;k<SDLK_LAST;k++) {
		Keyboard.KeyUp[k] = false;

		if(!Keyboard.keys[k] && Keyboard.KeyDown[k]) // it is up now but it was down previously
			Keyboard.KeyUp[k] = true;
		Keyboard.KeyDown[k] = Keyboard.keys[k];
	}
}

// halt the current thread until there is a new event
bool WaitForNextEvent() {
	ResetCurrentEventStorage();
	
	bool ret = false;
	if(SDL_WaitEvent(&Event)) {
		HandleNextEvent();		
		ret = true;
	}
	
	// Perhaps there are more events in the queue.
	// In this case, handle all of them. we want an empty
	// queue after
	while(SDL_PollEvent(&Event)) {
		HandleNextEvent();
		ret = true;
	}

	HandleMouseState();
	HandleKeyboardState();
	if(bJoystickSupport) SDL_JoystickUpdate();

	return ret;
}


///////////////////
// Process the events
bool ProcessEvents()
{
	ResetCurrentEventStorage();
	
	bool ret = false;
	while(SDL_PollEvent(&Event)) {
		HandleNextEvent();
		ret = true;
	}

    // If we don't have focus, don't update as often
    if(!nFocus)
        SDL_Delay(14);

	HandleMouseState();
	HandleKeyboardState();
	if(bJoystickSupport) SDL_JoystickUpdate();
	
	return ret;
}
