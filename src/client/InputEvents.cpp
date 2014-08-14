/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/


#include <set>

#include "Clipboard.h"
#include "LieroX.h"

#include "EventQueue.h"
#include "InputEvents.h"
#include "AuxLib.h"
#include "DeprecatedGUI/Menu.h"
#include "Timer.h"
#include "CInput.h"
#include "MathLib.h"
#include "NotifyUser.h"
#include "Event.h"
#include "MainLoop.h"

#include "gusanos/allegro.h"

#ifdef WIN32
#include <Windows.h>
#endif


// Keyboard, Mouse, & Event
static keyboard_t	Keyboard;
static mouse_t		Mouse;
static SDL_Event	sdl_event;
static ModifiersState evtModifiersState;

static bool         bHaveFocus = true;
bool		bActivated = false;
bool		bDeactivated = false;



///////////////////
// Returns the current state of the modifier keys (alt, ctrl etc.)
ModifiersState *GetCurrentModstate()
{
	return &evtModifiersState;
}

///////////////////
// Return the keyboard structure
keyboard_t *GetKeyboard()
{
	return &Keyboard;
}

///////////////////
// Return the mouse structure
mouse_t *GetMouse()
{
	return &Mouse;
}


///////////////////
// Return the event
SDL_Event *GetEvent()
{
	// TODO: this should not be used like it is atm because it only returns the last event
	// but in ProcessEvents() could be more than one Event get passed
	return &sdl_event;
}

bool bEventSystemInited = false;
bool bWaitingForEvent = false;

////////////////////
// Returns true if the event system is initialized
bool EventSystemInited()
{
	return bEventSystemInited;
}

bool IsWaitingForEvent() {
	return bWaitingForEvent;
}


///////////////////////
// Converts SDL button to a mouse button
MouseButton SDLButtonToMouseButton(int sdlbut)
{
	switch(sdlbut) {
		case SDL_BUTTON_LEFT: return mbLeft;
		case SDL_BUTTON_RIGHT: return mbRight;
		case SDL_BUTTON_MIDDLE: return mbMiddle;
		case SDL_BUTTON_X1: return mbExtra1;
		case SDL_BUTTON_X2: return mbExtra2;
		default: return mbLeft;
	}
}

// Returns just a single (the most important) button.
MouseButton SDLButtonStateToMouseButton(int sdlbut)
{
	if(sdlbut & SDL_BUTTON_LMASK) return mbLeft;
	if(sdlbut & SDL_BUTTON_RMASK) return mbRight;
	if(sdlbut & SDL_BUTTON_MMASK) return mbMiddle;
	if(sdlbut & SDL_BUTTON_X1MASK) return mbExtra1;
	if(sdlbut & SDL_BUTTON_X2MASK) return mbExtra2;
	return mbLeft; // default
}



std::map<SDL_EventType, SDLEvent> sdlEvents;

Event<> onDummyEvent;

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

	// Reset the video mode changed flag here
	if (tLX)
		tLX->bVideoModeChanged = false;

//	for(int k=0;k<SDL_NUM_SCANCODES;k++) {
//		Keyboard.KeyUp[k] = false;
//	}

	bActivated = false;
	bDeactivated = false;
}


bool WasKeyboardEventHappening(int sym, bool down) {
	for(int i = 0; i < Keyboard.queueLength; i++)
		if(Keyboard.keyQueue[i].sym == sym && Keyboard.keyQueue[i].down == down)
			return true;
	return false;
}


typedef void (*EventHandlerFct) (SDL_Event* ev);


static void EvHndl_WindowEvent(SDL_Event* ev) {
	switch(ev->window.event) {
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_FOCUS_LOST:
			bool hadFocusBefore = bHaveFocus;
			bHaveFocus = ev->window.event == SDL_WINDOWEVENT_FOCUS_GAINED;
			bActivated = bHaveFocus;
			bDeactivated = !bHaveFocus;
			
			// HINT: Reset the mouse state - this should avoid the mouse staying pressed
			Mouse.Button = 0;
			Mouse.Down = 0;
			Mouse.FirstDown = 0;
			Mouse.Up = 0;
			
			if(!hadFocusBefore && bHaveFocus) {
				//notes << "OpenLieroX got the focus" << endl;
				ClearUserNotify();
			} else if(hadFocusBefore && !bHaveFocus) {
				//notes << "OpenLieroX lost the focus" << endl;
			}
			
			if(tLXOptions->bAutoFileCacheRefresh && bActivated)
				updateFileListCaches();
				
			break;
	}
}

static void EvHndl_KeyDownUp(SDL_Event* ev) {
	// Check the characters
	if(ev->key.state == SDL_PRESSED || ev->key.state == SDL_RELEASED) {
		UnicodeChar input = ev->key.keysym.unicode;
		if (input == 0)
			switch (ev->key.keysym.sym) {
			case SDLK_HOME:
				input = 2;
				break;
			case SDLK_END:
				input = 3;
				break;
			case SDLK_KP_0:
			case SDLK_KP_1:
			case SDLK_KP_2:
			case SDLK_KP_3:
			case SDLK_KP_4:
			case SDLK_KP_5:
			case SDLK_KP_6:
			case SDLK_KP_7:
			case SDLK_KP_8:
			case SDLK_KP_9:
			case SDLK_KP_MULTIPLY:
			case SDLK_KP_MINUS:
			case SDLK_KP_PLUS:
			case SDLK_KP_EQUALS:
				input = (uchar) (ev->key.keysym.sym - 208);
				break;
			case SDLK_KP_PERIOD:
			case SDLK_KP_DIVIDE:
				input = (uchar) (ev->key.keysym.sym - 220);
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
			warnings << "Keyboard queue full" << endl;
		}

		KeyboardEvent& kbev = Keyboard.keyQueue[Keyboard.queueLength];

		// Key down
		if(ev->type == SDL_KEYDOWN)
			kbev.down = true;

		// Key up
		else if(ev->type == SDL_KEYUP || ev->key.state == SDL_RELEASED)
			kbev.down = false;

		else
			return; // don't save and handle it

		// save info
		kbev.ch = input;
		kbev.sym = ev->key.keysym.sym;
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
		case SDLK_LGUI: case SDLK_RGUI:
			evtModifiersState.bGui = kbev.down;
			break;
		}

		// copy it
		kbev.state = evtModifiersState;

		HandleCInputs_KeyEvent(kbev);

		/*
		if(Event.key.state == SDL_PRESSED && Event.key.type == SDL_KEYDOWN)
			// I don't want to track keyrepeats here; but works only for special keys
			notes << tLX->currentTime << ": pressed key " << kbev.sym << endl;
		else if(!kbev.down)
			notes << tLX->currentTime << ": released key " << kbev.sym << endl;
		*/
		
	} else
		warnings << "Strange Event.key.state = " << ev->key.state << endl;

}

static void EvHndl_MouseMotion(SDL_Event*) {}
static void EvHndl_MouseButtonDown(SDL_Event* ev) {}
static void EvHndl_MouseButtonUp(SDL_Event* ev) {}

static void EvHndl_MouseWheel(SDL_Event* ev) {
	if(ev->wheel.y < 0)
		Mouse.WheelScrollUp = true;
	else if(ev->wheel.y > 0)
		Mouse.WheelScrollDown = true;
}

static void EvHndl_Quit(SDL_Event*) {
	game.state = Game::S_Quit;
}

void EvHndl_SysWmEvent_MainThread(SDL_Event* ev) {
	handle_system_event(*ev); // Callback for clipboard on X11, should be called every time new event arrived
}

static void EvHndl_VideoExpose(SDL_Event*) {}

static void EvHndl_UserEvent(SDL_Event* ev) {
	if(ev->user.code == UE_CustomEventHandler) {
		((Action*)ev->user.data1)->handle();
		delete (Action*)ev->user.data1;
	}
}

void InitEventSystem() {	
	for(int k = 0;k<SDL_NUM_SCANCODES;k++)
		GetKeyboard()->KeyUp[k] = false;

	GetMouse()->Button = 0;
	GetMouse()->Down = 0;
	GetMouse()->FirstDown = 0;
	GetMouse()->Up = 0;

	sdlEvents[SDL_WINDOWEVENT].handler() = getEventHandler(&EvHndl_WindowEvent);
	sdlEvents[SDL_KEYDOWN].handler() = getEventHandler(&EvHndl_KeyDownUp);
	sdlEvents[SDL_KEYUP].handler() = getEventHandler(&EvHndl_KeyDownUp);
	sdlEvents[SDL_MOUSEMOTION].handler() = getEventHandler(&EvHndl_MouseMotion);
	sdlEvents[SDL_MOUSEBUTTONDOWN].handler() = getEventHandler(&EvHndl_MouseButtonDown);
	sdlEvents[SDL_MOUSEBUTTONUP].handler() = getEventHandler(&EvHndl_MouseButtonUp);
	sdlEvents[SDL_MOUSEWHEEL].handler() = getEventHandler(&EvHndl_MouseWheel);
	sdlEvents[SDL_QUIT].handler() = getEventHandler(&EvHndl_Quit);
	//sdlEvents[SDL_SYSWMEVENT].handler() = getEventHandler(&EvHndl_SysWmEvent); // Should be done from main thread
	sdlEvents[SDL_VIDEOEXPOSE].handler() = getEventHandler(&EvHndl_VideoExpose);
	sdlEvents[SDL_USEREVENT].handler() = getEventHandler(&EvHndl_UserEvent);

	bEventSystemInited = true;
	bWaitingForEvent = false;
}

void ShutdownEventSystem()
{
	bEventSystemInited = false;
	bWaitingForEvent = false;
}


// main function for handling next event
// HINT: we are using the global variable Event here atm
// in both cases where we call this function this is correct
// TODO: though the whole architecture has to be changed later
// but then also GetEvent() has to be changed or removed
void HandleNextEvent() {
	auto it = sdlEvents.find((SDL_EventType)sdl_event.type);
	if(it != sdlEvents.end())
		it->second.occurred(&sdl_event);
}

static void HandleMouseState() {
	{
		// Mouse
		int oldX = Mouse.X;
		int oldY = Mouse.Y;
		Mouse.Button = SDL_GetMouseState(&Mouse.X,&Mouse.Y); // Doesn't call libX11 funcs, so it's safe to call not from video thread

		// TODO: is that needed and does it make sense?
		//VideoPostProcessor::transformCoordinates_ScreenToVideo(Mouse.X, Mouse.Y);

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

    for( int i=1; i<=MAX_MOUSEBUTTONS; i++ ) {
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
	for(int k=0;k<SDL_NUM_SCANCODES;k++) {
		Keyboard.KeyUp[k] = false;

		if(!Keyboard.keys[k] && Keyboard.KeyDown[k]) // it is up now but it was down previously
			Keyboard.KeyUp[k] = true;
		Keyboard.KeyDown[k] = Keyboard.keys[k];
	}
}

// Declared in CInput.cpp
extern void updateAxisStates();

bool processedEvent = false;

///////////////////
// Process the events
void ProcessEvents()
{
	ResetCurrentEventStorage();

	bool ret = false;
	if(game.allowedToSleepForEvent() && !mainQueue->hasItems()) {
		bWaitingForEvent = true;
		if(isMainThread())
			handleSDLEvents(true);
		else if(mainQueue->wait(sdl_event)) {
			bWaitingForEvent = false;
			HandleNextEvent();
			ret = true;
		}
		bWaitingForEvent = false;
	}

	if(isMainThread())
		handleSDLEvents(false);

	while(mainQueue->poll(sdl_event)) {
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

	if (bDedicated)
	{
		#ifdef WIN32
		MSG msg;
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT || msg.message == WM_CLOSE )
			{
				EventItem ev;
				ev.type = SDL_QUIT;
				mainQueue->push(ev);
			}
		}
		#endif
	}

	if (!bDedicated) {
		// If we don't have focus, don't update as often
		if(!bHaveFocus)
			SDL_Delay(14);

		HandleMouseState();
		HandleKeyboardState();
#ifndef DEDICATED_ONLY
#ifndef DISABLE_JOYSTICK
		if(bJoystickSupport)  {
			SDL_JoystickUpdate();
			updateAxisStates();
		}
#endif
#endif
		HandleCInputs_UpdateUpForNonKeyboard();
		HandleCInputs_UpdateDownOnceForNonKeyboard();
	}

	processedEvent = ret;
}

void WakeupIfNeeded() {
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = UE_NopWakeup;
	SDL_PushEvent(&ev);
}

bool ApplicationHasFocus()
{
	return bHaveFocus;
}
