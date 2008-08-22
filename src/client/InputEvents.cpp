/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#include <SDL_syswm.h>
#include <iostream>
#include <set>

#include "Clipboard.h"
#include "LieroX.h"
#include "InputEvents.h"
#include "AuxLib.h"
//#include "Menu.h"
#include "Timer.h"
#include "CInput.h"
#include "MathLib.h"


using namespace std;


// Keyboard, Mouse, & Event
static keyboard_t	Keyboard;
static mouse_t		Mouse;
static SDL_Event	SDLEvent;
static ModifiersState evtModifiersState;

static int         nFocus = true;
bool		bActivated = false;

std::list<EventListener *>	tEventListeners;

///////////////////
// Adds an event listener
void AddEventListener(EventListener *lst)
{
	tEventListeners.push_back(lst);
}

///////////////////
// Removes the event listener from the internal list
void RemoveEventListener(EventListener *lst)
{
	for (std::list<EventListener *>::iterator it = tEventListeners.begin(); it != tEventListeners.end(); it++)
		if (*it == lst)  {
			tEventListeners.erase(it);
			break;
		}
}

///////////////////
// Returns the current state of the modifier keys (alt, ctrl etc.)
ModifiersState *GetCurrentModstate()
{
	return &evtModifiersState;
}

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
	return &SDLEvent;
}

///////////////////////
// Converts SDL button to a mouse button
MouseButton SDLButtonToMouseButton(int sdlbut)
{
	if (sdlbut & SDL_BUTTON_LEFT)
		return mbLeft;

	if (sdlbut & SDL_BUTTON_RIGHT)
		return mbRight;

	if (sdlbut & SDL_BUTTON_MIDDLE)
		return mbMiddle;

// There is no SDL_BUTTON_X1 and SDL_BUTTON_X2 in my SDL headers, only SDL_BUTTON_WHEELUP/WHEELDOWN -
// on Linux wheel up/down is emulated by 4th and 5th mouse buttons.
/*
	if (sdlbut & SDL_BUTTON_X1)
		return mbExtra1;

	if (sdlbut & SDL_BUTTON_X2)
		return mbExtra2;
*/
	return mbLeft; // Default
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

static std::set<CInput*> cInputs;

void RegisterCInput(CInput* input) {
	cInputs.insert(input);
}

void UnregisterCInput(CInput* input) {
	cInputs.erase(input);
}

static void ResetCInputs() {
	for(std::set<CInput*>::iterator it = cInputs.begin(); it != cInputs.end(); it++) {
		if((*it)->getResetEachFrame())
			(*it)->reset();
	}
}

void HandleCInputs_KeyEvent(KeyboardEvent& ev) {
	for(std::set<CInput*>::iterator it = cInputs.begin(); it != cInputs.end(); it++)
		if((*it)->isKeyboard() && (*it)->getData() == ev.sym) {
			if(ev.down) {
				(*it)->nDown++;
				if(!(*it)->bDown) {
					(*it)->nDownOnce++;
					(*it)->bDown = true;
				}
			} else {
				(*it)->bDown = false;
				(*it)->nUp++;
			}
		}
}

void HandleCInputs_UpdateDownOnceForNonKeyboard() {
	for(std::set<CInput*>::iterator it = cInputs.begin(); it != cInputs.end(); it++)
		if((*it)->isUsed() && !(*it)->isKeyboard()) {
			// HINT: It is possible that wasUp() and !Down (a case which is not covered in further code)
			if((*it)->wasUp() && !(*it)->bDown) {
				(*it)->nDownOnce++;
				continue;
			}

			// HINT: It's possible that wasDown() > 0 and !isDown().
			// That is the case when we press a key and release it directly after (in one frame).
			// Though wasDown() > 0 doesn't mean directly isDownOnce because it also counts keypresses.
			// HINT: It's also possible that wasDown() == 0 and isDown().
			// That is the case when we have pressed the key in a previous frame and we still hold it
			// and the keyrepeat-interval is bigger than FPS. (Rare case.)
			if((*it)->wasDown() || (*it)->isDown()) {
				// wasUp() > 0 always means that it was down once (though it is not down anymore).
				// Though the released key in wasUp() > 0 was probably already recognised before.
				if((*it)->wasUp()) {
					(*it)->bDown = (*it)->isDown();
					if((*it)->bDown) // if it is again down, there is another new press
						(*it)->nDownOnce++;
					continue;
				}
				// !Down means that we haven't recognised yet that it is down.
				if(!(*it)->bDown) {
					(*it)->bDown = (*it)->isDown();
					(*it)->nDownOnce++;
					continue;
				}
			}
			else
				(*it)->bDown = false;
		}
}

void HandleCInputs_UpdateUpForNonKeyboard() {
	for(std::set<CInput*>::iterator it = cInputs.begin(); it != cInputs.end(); it++)
		if((*it)->isUsed() && !(*it)->isKeyboard()) {
			if((*it)->isDown() && !(*it)->bDown) {
				(*it)->nUp++;
			}
		}
}

static void ResetCurrentEventStorage() {
	ResetCInputs();

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
void HandleNextEvent() {
	if (tLX == NULL)
		return;

	switch(SDLEvent.type) {

	// Quit event
	case SDL_QUIT:
		// Quit
		tLX->bQuitGame = true;
		SetQuitEngineFlag("SDL_QUIT event");
		//tMenu->bMenuRunning = false;
		break;

	// Mouse wheel scroll
	case SDL_MOUSEBUTTONDOWN:
		switch(SDLEvent.button.button) {
			case SDL_BUTTON_WHEELUP:
				Mouse.WheelScrollUp = true;
				break;
			case SDL_BUTTON_WHEELDOWN:
				Mouse.WheelScrollDown  = true;
				break;
		}  // switch

		{
			MouseEvent mev = { SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button, true };
			Mouse.mouseQueue.push_back( mev );
		}
		break;

	case SDL_MOUSEBUTTONUP:
		{
			MouseEvent mev = { SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button, false };
			Mouse.mouseQueue.push_back( mev );
		}
		break;

	// Activation and deactivation
	case SDL_ACTIVEEVENT:
		if(!(SDLEvent.active.state & SDL_APPMOUSEFOCUS))  {
				nFocus = SDLEvent.active.gain;
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
		if(SDLEvent.key.state == SDL_PRESSED || SDLEvent.key.state == SDL_RELEASED) {

			UnicodeChar input = SDLEvent.key.keysym.unicode;
			if (input == 0)
				switch (SDLEvent.key.keysym.sym) {
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
					input = (uchar) (SDLEvent.key.keysym.sym - 208);
					break;
				case SDLK_KP_PERIOD:
				case SDLK_KP_DIVIDE:
					input = (uchar) (SDLEvent.key.keysym.sym - 220);
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
			if(SDLEvent.type == SDL_KEYDOWN)
				kbev.down = true;

			// Key up
			else if(SDLEvent.type == SDL_KEYUP || SDLEvent.key.state == SDL_RELEASED)
				kbev.down = false;

			else
				break; // don't save and handle it

			// save info
			kbev.ch = input;
			kbev.sym = SDLEvent.key.keysym.sym;
			Keyboard.queueLength++;

			// handle modifier state
			switch (kbev.sym)  {
			case SDLK_LALT: case SDLK_RALT:
				evtModifiersState.bAlt = kbev.down;
				break;
			case SDLK_LCTRL: case SDLK_RCTRL:
				evtModifiersState.bCtrl = kbev.down;
				break;
			case SDLK_LSHIFT: case SDLK_RSHIFT:
				evtModifiersState.bShift = kbev.down;
				break;
			case SDLK_LSUPER: case SDLK_RSUPER:
				evtModifiersState.bSuper = kbev.down;
				break;
			}

			// copy it
			kbev.state = evtModifiersState;

			HandleCInputs_KeyEvent(kbev);

			/*
			if(Event.key.state == SDL_PRESSED && Event.key.type == SDL_KEYDOWN)
				// I don't want to track keyrepeats here; but works only for special keys
				cout << tLX->fCurTime << ": pressed key " << kbev.sym << endl;
			else if(!kbev.down)
				cout << tLX->fCurTime << ": released key " << kbev.sym << endl;
			*/

		} else
			printf("strange Event.key.state = %i\n", SDLEvent.key.state);
		break;

	case SDL_SYSWMEVENT:
		handle_system_event(SDLEvent);
		break;

	case SDL_USEREVENT_TIMER:
		Timer::handleEvent(SDLEvent);
		break;

	default:
		//std::cout << "WARNING: unhandled event " << Event.type << std::endl;
		break;
	}

	// Notify the event listeners
	for (std::list<EventListener *>::iterator it = tEventListeners.begin(); it != tEventListeners.end(); it++)
		(*it)->OnEvent(&SDLEvent);
}

static void HandleMouseState() {
	{
		// Mouse
		int oldX = Mouse.X;
		int oldY = Mouse.Y;
		Mouse.Button = SDL_GetMouseState(&Mouse.X,&Mouse.Y);

		VideoPostProcessor::transformCoordinates_ScreenToVideo(Mouse.X, Mouse.Y);

		Mouse.deltaX = Mouse.X-oldX;
		Mouse.deltaY = Mouse.Y-oldY;
		Mouse.Up = 0;
		Mouse.FirstDown = 0;
	}

#ifdef FUZZY_ERROR_TESTING
	/*Mouse.Button = GetRandomInt(8);
	Mouse.deltaX = SIGN(GetRandomNum()) * GetRandomInt(655535);
	Mouse.deltaY = SIGN(GetRandomNum()) * GetRandomInt(655535);
	Mouse.Up = SIGN(GetRandomNum()) * GetRandomInt(655535);
	Mouse.FirstDown = SIGN(GetRandomNum()) * GetRandomInt(655535);
	return;*/
#endif

    for( int i=0; i<MAX_MOUSEBUTTONS; i++ ) {
		if(!(Mouse.Button & SDL_BUTTON(i)) && Mouse.Down & SDL_BUTTON(i))
			Mouse.Up |= SDL_BUTTON(i);
        if( !(Mouse.Down & SDL_BUTTON(i)) && (Mouse.Button & SDL_BUTTON(i)) )
            Mouse.FirstDown |= SDL_BUTTON(i);
    }

	Mouse.Down = Mouse.Button;
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
	if(SDL_WaitEvent(&SDLEvent)) {
		HandleNextEvent();
		ret = true;
	}

	// Perhaps there are more events in the queue.
	// In this case, handle all of them. we want an empty
	// queue after
	while(SDL_PollEvent(&SDLEvent)) {
		HandleNextEvent();
		ret = true;
	}

	HandleMouseState();
	HandleKeyboardState();
	if(bJoystickSupport) SDL_JoystickUpdate();
	HandleCInputs_UpdateUpForNonKeyboard();
	HandleCInputs_UpdateDownOnceForNonKeyboard();

	return ret;
}

// Declared in CInput.cpp
extern void updateAxisStates();

///////////////////
// Process the events
bool ProcessEvents()
{
	ResetCurrentEventStorage();

	bool ret = false;
	while(SDL_PollEvent(&SDLEvent)) {
		HandleNextEvent();
		ret = true;
	}

#ifdef FUZZY_ERROR_TESTING
	/*Event.type = GetRandomInt(255);
	HandleNextEvent();*/

	/*Event.type = SDL_KEYDOWN;
	Event.key.keysym.sym = (SDLKey)GetRandomInt(65535);
	Event.key.keysym.mod = (SDLMod)GetRandomInt(65535);
	Event.key.keysym.scancode = GetRandomInt(65535);
	Event.key.keysym.unicode = GetRandomInt(65535);
	Event.key.which = GetRandomInt(65535);
	Event.key.state = GetRandomInt(50) > 25 ? SDL_PRESSED : SDL_RELEASED;
	HandleNextEvent();*/
#endif

    // If we don't have focus, don't update as often
    if(!nFocus)
        SDL_Delay(14);

	HandleMouseState();
	HandleKeyboardState();
	if(bJoystickSupport)  {
		SDL_JoystickUpdate();
		updateAxisStates();
	}
	HandleCInputs_UpdateUpForNonKeyboard();
	HandleCInputs_UpdateDownOnceForNonKeyboard();

	return ret;
}

