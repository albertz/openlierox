/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include "CGuiSkinnedLayout.h"

#include "LieroX.h"
#include "AuxLib.h"
#include "Menu.h"
#include "StringUtils.h"
#include "CBox.h"
#include "CImage.h"
#include "CButton.h"
#include "CCheckbox.h"
#include "CLabel.h"
#include "CSlider.h"
#include "CTextbox.h"

#include <assert.h>



///////////////////
// Initialize the layout
void CGuiSkinnedLayout::Initialize()
{
	Shutdown();

	cWidgets = NULL;
	cFocused = NULL;
	cMouseOverWidget = NULL;

	// Reset mouse repeats
	nMouseButtons = 0;
	for(int i=0; i<3; i++)
		fMouseNext[i] = -9999;

}


///////////////////
// Add a widget to the gui layout
void CGuiSkinnedLayout::Add(CWidget *widget, int id, int x, int y, int w, int h)
{
	widget->Setup(id, x, y, w, h);
	widget->Create();
	widget->setParent(this);

	// Link the widget in
	widget->setPrev(NULL);
	widget->setNext(cWidgets);

	if(cWidgets)
		cWidgets->setPrev(widget);

	cWidgets = widget;
}


///////////////////
// Remove a widget
void CGuiSkinnedLayout::removeWidget(int id)
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
void CGuiSkinnedLayout::Shutdown(void)
{
	CWidget *w,*wid;

	for(w=cWidgets ; w ; w=wid) {
		wid = w->getNext();

		w->Destroy();

		if(w)
			delete w;
	}
	cWidgets = NULL;

	// tEvent is freed in destructor

	cFocused = NULL;
	cMouseOverWidget = NULL;
}


///////////////////
// Draw the widgets
void CGuiSkinnedLayout::Draw(SDL_Surface *bmpDest)
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
gui_event_t *CGuiSkinnedLayout::Process(void)
{
	CWidget *w;
	mouse_t *tMouse = GetMouse();
	SDL_Event *Event = GetEvent();
	int ev=-1;
	int widget = false;

	SetGameCursor(CURSOR_ARROW); // Reset the cursor here

	if (!tEvent)  {
		return NULL;
	}

	// Switch between window and fullscreen mode
	keyboard_t *Keyboard = GetKeyboard();

	// Put it here, so the mouse will never display
	SDL_ShowCursor(SDL_DISABLE);

	// Parse keyboard events to the focused widget
	// Make sure a key event happened
	if(Event->type == SDL_KEYUP || Event->type == SDL_KEYDOWN) {

		// If we don't have any focused widget, get the first textbox
		if (!cFocused)  {
			CWidget *txt = cWidgets;
			for (;txt;txt=txt->getNext())  {
				if (txt->getType() == wid_Textbox && txt->getEnabled()) {
					cFocused = txt;
					txt->setFocused(true);
					break;
				}
			}
		}


		if (cFocused)  {
			// Check the characters
			if(Event->key.state == SDL_PRESSED || Event->key.state == SDL_RELEASED) {
				tEvent->cWidget = cFocused;
				tEvent->iControlID = cFocused->getID();

				UnicodeChar input = Event->key.keysym.unicode;
				if (input == 0)
					switch (Event->key.keysym.sym) {
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
						input = (uchar) (Event->key.keysym.sym - 208);
						break;
					case SDLK_KP_PERIOD:
					case SDLK_KP_DIVIDE:
						input = (uchar) (Event->key.keysym.sym - 220);
						break;
					case SDLK_KP_ENTER:
						input = '\r';
						break;
                    default:
                        // TODO: much not handled keys; is this correct?
                        break;

				}  // switch


				if(Event->type == SDL_KEYUP || Event->key.state == SDL_RELEASED)
					ev = cFocused->KeyUp(input, Event->key.keysym.sym);

				// Handle more keys at once keydown
				if (Keyboard->queueLength > 1) 
					for(int i=0; i<Keyboard->queueLength; i++)
						if(!Keyboard->keyQueue[i].down || Keyboard->keyQueue[i].ch != input)  {
							ev = cFocused->KeyDown(Keyboard->keyQueue[i].ch, Keyboard->keyQueue[i].sym);
							if (ev != -1)  {
								tEvent->iEventMsg = ev;
								return tEvent;
							}
						}

				// Keydown
				if(Event->type == SDL_KEYDOWN)  {
					ev = cFocused->KeyDown(input, Event->key.keysym.sym);
				}

				// Tab switches between widgets
				/*if (Keyboard->KeyUp[SDLK_TAB])  {
					if (cFocused)  {
						// The current one is not focused anymore
						cFocused->setFocused(false);

						// Switch to next widget
						if (cFocused->getNext())  {
							cFocused = cFocused->getNext();
							cFocused->setFocused(true);
						// The current focused widget is the last one in the list
						} else {
							cFocused = cWidgets;
							cFocused->setFocused(true);
						}
					} else {
						cFocused = cWidgets;
						cFocused->setFocused(true);
					}

					// Repeat the same thing until we find first enabled widget
					while (!cFocused->getEnabled())  {
						// The current one is not focused anymore
						cFocused->setFocused(false);

						if (cFocused->getNext())  {
							cFocused = cFocused->getNext();
							cFocused->setFocused(true);
						// The current focused widget is the last one in the list
						} else {
							cFocused = cWidgets;
							cFocused->setFocused(true);
						}
					}
				}*/

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

			// Process the skin-defined code
			w->ProcessEvent(OnMouseDown);

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

				// Process the skin defined code
				w->ProcessEvent(OnMouseDown);

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

				// Process the skin defined code
				w->ProcessEvent(OnClick);

				if( (ev=w->MouseUp(tMouse, tMouse->Up)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}


			// Mouse over
			if (w != cMouseOverWidget)  {
				w->ProcessEvent(OnMouseOver);

				// For the current Mouse over widget this means a mouse out event
				if(cMouseOverWidget)
					cMouseOverWidget->ProcessEvent(OnMouseOut);

				cMouseOverWidget = w;
			}

			if( (ev=w->MouseOver(tMouse)) >= 0) {
				tEvent->iEventMsg = ev;
				return tEvent;
			}

			// -2 - the widget says, that no event happened (various reasons)
			if (ev != -2)
				return NULL;
		}
	}

	// If mouse is over empty space it means, it's not over any widget ;-)
	if (cMouseOverWidget)  {
		cMouseOverWidget->ProcessEvent(OnMouseOut);
		cMouseOverWidget = NULL;
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
		tEvent->iControlID = -9999;
		tEvent->iEventMsg = SDL_BUTTON_WHEELUP;
		return tEvent;
	}

	// Non-widget wheel down
	if(tMouse->WheelScrollDown)  {
		tEvent->iControlID = -9999;
		tEvent->iEventMsg = SDL_BUTTON_WHEELDOWN;
		return tEvent;
	}


	return NULL;
}


///////////////////
// Return a widget based on id
CWidget *CGuiSkinnedLayout::getWidget(int id)
{
	CWidget *w;

	for(w=cWidgets ; w ; w=w->getNext()) {
		if(w->getID() == id)
			return w;
	}

	return NULL;
}

////////////////////
// Get the widget ID
int	CGuiSkinnedLayout::GetIdByName(const std::string & Name)
{
	int ID = -1;
	// Find the standard or previously added widget
	ID = LayoutWidgets.getID(Name);

	// Non-standard widget, add it to the list
	if (ID == -1)
		ID = LayoutWidgets.Add(Name);
	return ID;
}

////////////////////
// Notifies about the error that occured
void CGuiSkinnedLayout::Error(int ErrorCode, const char *Format, ...)
{
	static char buf[512];
	va_list	va;

	va_start(va,Format);
	vsnprintf(buf,sizeof(buf),Format,va);
	fix_markend(buf);
	va_end(va);

	// TODO: this better
	printf("%i: %s",ErrorCode,buf);
}

///////////////////
// Send a message to a widget
DWORD CGuiSkinnedLayout::SendMessage(int iControl, int iMsg, DWORD Param1, DWORD Param2)
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
				w->setEnabled(Param1 != 0);
				break;
		}
		return 0;
	}

	return w->SendMessage(iMsg, Param1, Param2);
}

DWORD CGuiSkinnedLayout::SendMessage(int iControl, int iMsg, const std::string& sStr, DWORD Param)
{
	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	return w->SendMessage(iMsg, sStr, Param);

}

DWORD CGuiSkinnedLayout::SendMessage(int iControl, int iMsg, std::string *sStr, DWORD Param)
{
	// Check the string
	if (!sStr)
		return 0;

	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	return w->SendMessage(iMsg, sStr, Param);
}
