/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifdef WIN32
#include <SDL/SDL_syswm.h>
#endif
#include "InputEvents.h"
#include "AuxLib.h"
#include "LieroX.h"
#include "Menu.h"


// Keyboard, Mouse, & Event
keyboard_t	Keyboard;
mouse_t		Mouse;
SDL_Event	Event;

int         nFocus = true;
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
	return &Event;
}




///////////////////
// Process the events
void ProcessEvents(void)
{
    // Clear the queue
    Keyboard.queueLength = 0;

	// Reset mouse wheel
	Mouse.WheelScrollUp = false;
	Mouse.WheelScrollDown = false;

	bActivated = false;

	while(SDL_PollEvent(&Event)) {

        // Quit event
		if(Event.type == SDL_QUIT) {
			// Quit
			tLX->iQuitGame = true;
			tLX->iQuitEngine = true;
			tMenu->iMenuRunning = false;
		}

		// Mouse wheel scroll
		if(Event.type == SDL_MOUSEBUTTONDOWN)  {
			switch(Event.button.button){
				case SDL_BUTTON_WHEELUP:
					Mouse.WheelScrollUp = true;
					break;
				case SDL_BUTTON_WHEELDOWN:
					Mouse.WheelScrollDown  = true;
					break;
			}  // switch
		 }  // if


#ifdef WIN32
        // System events
        if(Event.type == SDL_SYSWMEVENT) {
            SDL_SysWMmsg *msg = Event.syswm.msg;

            switch(msg->msg) {

                // Lose focus event
                case WM_KILLFOCUS:
			        nFocus = false;
                    break;

                // Gain focus event
                case WM_SETFOCUS:  {
						// Hint: Reset the mouse state - this should avoid the mouse staying pressed
						Mouse.Button = 0;
						Mouse.Down = 0;
						Mouse.FirstDown = 0;
						Mouse.Up = 0;

						nFocus = true;
						bActivated = true;
					}
                    break;
            }
        }
#else
	// TODO: ignore it?
#endif

        // Keyboard events
		if(Event.type == SDL_KEYUP || Event.type == SDL_KEYDOWN) {

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
                }

                // Key down
                if(Event.type == SDL_KEYDOWN) {
					Keyboard.keyQueue[Keyboard.queueLength].down = true;
                    Keyboard.keyQueue[Keyboard.queueLength].ch = input;
                    Keyboard.queueLength++;
                }

				// Key up
				if(Event.type == SDL_KEYUP || Event.key.state == SDL_RELEASED) {
					Keyboard.keyQueue[Keyboard.queueLength].down = false;
                    Keyboard.keyQueue[Keyboard.queueLength].ch = input;
                    Keyboard.queueLength++;
				}

            }
        }
	}


    // If we don't have focus, don't update as often
    if(!nFocus)
        SDL_Delay(14);


	// Mouse
	Mouse.Button = SDL_GetMouseState(&Mouse.X,&Mouse.Y);
	Mouse.Up = 0;
    Mouse.FirstDown = 0;

	// Left Mouse Button Up event
	if(!(Mouse.Button & SDL_BUTTON(SDL_BUTTON_LEFT)) && Mouse.Down & SDL_BUTTON(SDL_BUTTON_LEFT))
		Mouse.Up |= SDL_BUTTON(SDL_BUTTON_LEFT);

	// Right Mouse Button Up event
	if(!(Mouse.Button & SDL_BUTTON(SDL_BUTTON_RIGHT)) && Mouse.Down & SDL_BUTTON(SDL_BUTTON_RIGHT))
		Mouse.Up |= SDL_BUTTON(SDL_BUTTON_RIGHT);

	// First down
    for( int i=0; i<3; i++ ) {
        if( !(Mouse.Down & SDL_BUTTON(i)) && (Mouse.Button & SDL_BUTTON(i)) )
            Mouse.FirstDown |= SDL_BUTTON(i);
    }

	Mouse.Down = Mouse.Button;

    // SAFETY HACK: If we get any mouse presses, we must have focus
    if(Mouse.Down)  {
		if (!nFocus)
			bActivated = true;
        nFocus = true;
	}



	// Keyboard
	Keyboard.keys = SDL_GetKeyState(NULL);

	// Update the key up's
	for(int k=0;k<SDLK_LAST;k++) {
		Keyboard.KeyUp[k] = false;

		if(!Keyboard.keys[k] && Keyboard.KeyDown[k])
			Keyboard.KeyUp[k] = true;
		Keyboard.KeyDown[k] = Keyboard.keys[k];
	}
}
