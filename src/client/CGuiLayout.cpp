/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// GUI Layout class
// Created 5/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Initialize the layout
void CGuiLayout::Initialize(void)
{
	Shutdown();

	cWidgets = NULL;
	tEvent = new gui_event_t;
	cFocused = NULL;

	// Reset mouse repeats
	nMouseButtons = 0;
	for(int i=0; i<3; i++)
		fMouseNext[i] = -9999;

}


///////////////////
// Add a widget to the gui layout
void CGuiLayout::Add(CWidget *widget, int id, int x, int y, int w, int h)
{
	widget->Setup(id, x, y, w, h);
	widget->Create();

	// Link the widget in
	widget->setPrev(NULL);
	widget->setNext(cWidgets);

	if(cWidgets)
		cWidgets->setPrev(widget);

	cWidgets = widget;
}


///////////////////
// Remove a widget
void CGuiLayout::removeWidget(int id)
{
    CWidget *w = getWidget(id);
    if( !w )
        return;

    // If this is the focused widget, set focused to null
    if(cFocused) {
        if(w->getID() == cFocused->getID())
            cFocused = NULL;
    }

    // Unlink the widget
    if( w->getPrev() )
        w->getPrev()->setNext( w->getNext() );
    else
        cWidgets = w->getNext();

    if( w->getNext() )
        w->getNext()->setPrev( w->getPrev() );

    // Free it
    w->Destroy();
	assert(w);
    delete w;
}


///////////////////
// Shutdown the gui layout
void CGuiLayout::Shutdown(void)
{
	CWidget *w,*wid;

	for(w=cWidgets ; w ; w=wid) {		
		wid = w->getNext();

		w->Destroy();

		if(w)
			delete w;
	}
	cWidgets = NULL;

	if(tEvent) {
		delete tEvent;
		tEvent = NULL;
	}
}


///////////////////
// Draw the widgets
void CGuiLayout::Draw(SDL_Surface *bmpDest)
{
	CWidget *w, *end;

	// Draw the widgets in reverse order
	end = NULL;
	for(w=cWidgets ; w ; w=w->getNext()) {
		if(w->getNext() == NULL) {
			end = w;
			break;
		}
	}


	for(w=end ; w ; w=w->getPrev()) {
		if(w->getEnabled() && w)
			w->Draw(bmpDest);
	}
}


///////////////////
// Process all the widgets
gui_event_t *CGuiLayout::Process(void)
{
	CWidget *w;
	mouse_t *tMouse = GetMouse();
	SDL_Event *Event = GetEvent();
	int ev=-1;
	int widget = false;

	// Switch between window and fullscreen mode
	keyboard_t *Keyboard = GetKeyboard();
	if( cSwitchMode.isDown() )  {
		// Set to fullscreen
		tLXOptions->iFullscreen = !tLXOptions->iFullscreen;

		// Set the new video mode
		SetVideoMode();

		// Update both menu and game screens
		tMenu->bmpScreen = SDL_GetVideoSurface();

		// Redraw the mouse
		Menu_RedrawMouse(true);
	}

	// Put it here, so the mouse will never display
	SDL_ShowCursor(SDL_DISABLE);

	// Parse keyboard events to the focused widget
	if(cFocused) {
		
		// Make sure a key event happened
		if(Event->type == SDL_KEYUP || Event->type == SDL_KEYDOWN) {

			// Check the characters
			if(Event->key.state == SDL_PRESSED || Event->key.state == SDL_RELEASED) {
				tEvent->cWidget = cFocused;
				tEvent->iControlID = cFocused->getID();

				int input = (Event->key.keysym.unicode);
				if (input == 0)
					switch (Event->key.keysym.sym) {
					case SDLK_LEFT:
					case SDLK_RIGHT:
					case SDLK_DELETE:
					case SDLK_HOME:
					case SDLK_END:
						input = Event->key.keysym.sym;
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
						input = (char) (Event->key.keysym.sym - 208);
						break;
					case SDLK_KP_PERIOD:
					case SDLK_KP_DIVIDE:
						input = (char) (Event->key.keysym.sym - 220);
						break;
					case SDLK_KP_ENTER:
						input = '\r';
						break;


				}  // switch

				if(Event->type == SDL_KEYUP || Event->key.state == SDL_RELEASED)
					ev = cFocused->KeyUp(input);

				// Handle more keys at once keydown
				for(int i=0; i<Keyboard->queueLength; i++)
					if (Keyboard->keyQueue[i] != input)
						ev = cFocused->KeyDown(Keyboard->keyQueue[i]);
				
				if(Event->type == SDL_KEYDOWN)  {
					ev = cFocused->KeyDown(input);
				}

				if(ev >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}
		}
	}


	// Go through all the widgets
	for(w=cWidgets ; w ; w=w->getNext()) {
		tEvent->cWidget = w;
		tEvent->iControlID = w->getID();

		// Don't process disabled widgets
		if(!w->getEnabled())
			continue;

		// Special mouse button event first (for focused widgets)
		if(tMouse->Down && cFocused == w && !iCanFocus && !w->InBox(tMouse->X,tMouse->Y)) {
			widget = true;

			if( (ev=w->MouseDown(tMouse, tMouse->Down)) >= 0) {
				tEvent->iEventMsg = ev;
				return tEvent;
			}
		}


		if(w->InBox(tMouse->X,tMouse->Y)) {

			// Mouse wheel up
			if(tMouse->WheelScrollUp)  {
				widget = true;
				if(cFocused)
					cFocused->setFocused(false);
				w->setFocused(true);
				cFocused = w;

				if( (ev=w->MouseWheelUp(tMouse)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse wheel down
			if(tMouse->WheelScrollDown)  {
				widget = true;
				if(cFocused)
					cFocused->setFocused(false);
				w->setFocused(true);
				cFocused = w;

				if( (ev=w->MouseWheelDown(tMouse)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse button event first
			if(tMouse->Down) {

				widget = true;
				if (iCanFocus)  {
					if(cFocused)  {
						if (cFocused->CanLoseFocus())  {
							cFocused->setFocused(false);
							w->setFocused(true);
							cFocused = w;
							iCanFocus = false;
						}
					}
					else  {
						w->setFocused(true);
						cFocused = w;
						iCanFocus = false;
					}

				}

				if( (ev=w->MouseDown(tMouse, tMouse->Down)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse up event
			if(tMouse->Up) {
				iCanFocus = true;
				widget = true;
				if(cFocused)  {
					if(cFocused->CanLoseFocus())  {
						cFocused->setFocused(false);
						w->setFocused(true);
						cFocused = w;
					}
				}
				else  {
					w->setFocused(true);
					cFocused = w;
				}


				if( (ev=w->MouseUp(tMouse, tMouse->Up)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}				
			}

			// Mouse over
			if( (ev=w->MouseOver(tMouse)) >= 0) {
				tEvent->iEventMsg = ev;
				return tEvent;
			}

			return NULL;
		}
	}

	// If the mouse is clicked on empty space, take the focus of off the widget (if we can)
	if(!widget && (tMouse->Up)) {
		iCanFocus = true;
		if(cFocused)  {
			// Can we take the focus off?
			if (cFocused->CanLoseFocus())  {
				cFocused->setFocused(false);
				cFocused = NULL;
			}
			else  {
				cFocused->MouseUp(tMouse, tMouse->Up);
				cFocused->setLoseFocus(true);
			}
		}
		else  {
			cFocused = NULL;
		}

	}

	// Non-widget wheel up
	if(tMouse->WheelScrollUp)  {
		tEvent->iEventMsg = SDL_BUTTON_WHEELUP;
		return tEvent;
	}

	// Non-widget wheel down
	if(tMouse->WheelScrollDown)  {
		tEvent->iEventMsg = SDL_BUTTON_WHEELDOWN;
		return tEvent;
	}


	return NULL;
}


///////////////////
// Return a widget based on id
CWidget *CGuiLayout::getWidget(int id)
{
	CWidget *w;

	for(w=cWidgets ; w ; w=w->getNext()) {
		if(w->getID() == id)
			return w;
	}

	return NULL;
}


///////////////////
// Send a message to a widget
int CGuiLayout::SendMessage(int iControl, int iMsg, DWORD Param1, DWORD Param2)
{
	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	// Check if it's a widget message
	if(iMsg < 0) {
		switch( iMsg ) {
			
			// Set the enabled state of the widget
			case WDM_SETENABLE:
				w->setEnabled(Param1);
				break;
		}
		return 0;
	}

	return w->SendMessage(iMsg, Param1, Param2);
}
