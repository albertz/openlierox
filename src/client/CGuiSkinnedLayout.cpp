/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include <assert.h>
#include <stdarg.h>

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




///////////////////
// Initialize the layout
void CGuiSkinnedLayout::Initialize()
{
	Shutdown();

	cWidgets = NULL;
	cWidgetsFromEnd = NULL;
	cFocused = NULL;
	//cMouseOverWidget = NULL;

	// Reset mouse repeats
	nMouseButtons = 0;
	for(int i=0; i<3; i++)
		fMouseNext[i] = -9999;

}


///////////////////
// Add a widget to the gui layout
void CGuiSkinnedLayout::Add(CWidget *widget, int id, int x, int y, int w, int h)
{
	widget->Setup(id, x + iOffsetX, y + iOffsetY, w, h);
	widget->Create();
	widget->setParent(this);

	// Link the widget in
	widget->setPrev(NULL);
	widget->setNext(cWidgets);

	if(cWidgets)
		cWidgets->setPrev(widget);

	cWidgets = widget;
	if( cWidgetsFromEnd == NULL ) 
		cWidgetsFromEnd = widget;
}

void CGuiSkinnedLayout::SetOffset( int x, int y )
{
	int diffX = x - iOffsetX;
	int diffY = y - iOffsetY;
	iOffsetX = x;
	iOffsetY = y; 

	for( CWidget * w=cWidgets ; w ; w=w->getNext() ) 
	{
		w->Setup( w->getID(), w->getX() + diffX, w->getY() + diffY, w->getWidth(), w->getHeight() );
	};
};

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
	/*
	if( cMouseOverWidget )
	{
        if( w->getID() == cMouseOverWidget->getID() )
            cMouseOverWidget = NULL;
    }
	*/

    // Unlink the widget
    if( w->getPrev() )
        w->getPrev()->setNext( w->getNext() );
    else
        cWidgets = w->getNext();

    if( w->getNext() )
        w->getNext()->setPrev( w->getPrev() );
	else
		cWidgetsFromEnd = w->getPrev();

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
	cWidgetsFromEnd = NULL;

	cFocused = NULL;
	//cMouseOverWidget = NULL;
	setParent(NULL);
	cChildLayout = NULL;
}


///////////////////
// Draw the widgets
void CGuiSkinnedLayout::Draw(SDL_Surface *bmpDest)
{
	CWidget *w;
	
	if( ! cChildLayout || ( cChildLayout && ! bChildLayoutFullscreen ) )
	for( w = cWidgetsFromEnd ; w ; w = w->getPrev() ) 
	{
		if( w->getEnabled() )
			w->Draw(bmpDest);
	}
	
	if( cChildLayout )
		cChildLayout->Draw(bmpDest);
}


///////////////////
// Process all the widgets
bool CGuiSkinnedLayout::Process(void)
{
	mouse_t *tMouse = GetMouse();
	keyboard_t *Keyboard = GetKeyboard();
	// int ev=-1; // We don't have any event structure here, all events are just passed down to children.

	//SetGameCursor(CURSOR_ARROW); // Reset the cursor here


	// Put it here, so the mouse will never display
	//SDL_ShowCursor(SDL_DISABLE);

	// Parse keyboard events
	for(int i = 0; i < Keyboard->queueLength; i++) {
		const KeyboardEvent& kbev = Keyboard->keyQueue[i];
		if(kbev.down)
			KeyDown(kbev.ch, kbev.sym, kbev.state);
		else
			KeyUp(kbev.ch, kbev.sym, kbev.state);
	}


	if( tMouse->Down )
		MouseDown(tMouse, tMouse->Down);
	if( tMouse->Up )
		MouseUp(tMouse, tMouse->Up);
	if( tMouse->WheelScrollDown )
		MouseWheelDown(tMouse);
	if( tMouse->WheelScrollUp )
		MouseWheelUp(tMouse);
	MouseOver(tMouse);
	CGuiSkin::ProcessUpdateCallbacks();	// Update the news box (and IRC chat if I will ever make it)
	
	return ! bExitCurrentDialog;
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

void CGuiSkinnedLayout::ProcessGuiSkinEvent(int iEvent)
{
	if( iEvent < 0 )	// Global event - pass it to all children
	{
		for( CWidget * w = cWidgets ; w ; w = w->getNext() )
			w->ProcessGuiSkinEvent( iEvent );
		if( cChildLayout )
			cChildLayout->ProcessGuiSkinEvent( iEvent );
	};
};

int		CGuiSkinnedLayout::MouseOver(mouse_t *tMouse)
{
	if( cChildLayout )
	{
		cChildLayout->MouseOver(tMouse);
		return -1;
	};
	SetGameCursor(CURSOR_ARROW); // Set default mouse cursor - widget will change it
	for( CWidget * w = cWidgets ; w ; w = w->getNext() ) 
	{
		if(!w->getEnabled())
			continue;
		if(w->InBox(tMouse->X,tMouse->Y))
		{
			int ev = w->MouseOver(tMouse);
			if( ev >= 0 )
				w->ProcessGuiSkinEvent( ev );
			return -1;
		};
	};
	return -1;
};

int		CGuiSkinnedLayout::MouseUp(mouse_t *tMouse, int nDown)
{
	if( cChildLayout )
	{
		cChildLayout->MouseUp(tMouse, tMouse->Up);
		return -1;
	};
	if( cFocused ) if( ! cFocused->CanLoseFocus() )	// User finished selecting text in textbox - send message
	{
		int ev = cFocused->MouseUp(tMouse, tMouse->Down);
		if( ev >= 0 )
			cFocused->ProcessGuiSkinEvent( ev );
		return -1;
	};
	for( CWidget * w = cWidgets ; w ; w = w->getNext() ) 
	{
		if(!w->getEnabled())
			continue;
		if(w->InBox(tMouse->X,tMouse->Y))
		{
			FocusOnMouseClick( w );
			int ev = w->MouseUp(tMouse, tMouse->Up);
			if( ev >= 0 )
				w->ProcessGuiSkinEvent( ev );
			return -1;
		};
	};
	FocusOnMouseClick( NULL );
	return -1;
};

int		CGuiSkinnedLayout::MouseDown(mouse_t *tMouse, int nDown)
{
	if( cChildLayout )
	{
		cChildLayout->MouseDown(tMouse, tMouse->Down);
		return -1;
	};
	if( cFocused ) if( ! cFocused->CanLoseFocus() )	// User selects text in textbox - send message to it
	{
		int ev = cFocused->MouseDown(tMouse, tMouse->Down);
		if( ev >= 0 )
			cFocused->ProcessGuiSkinEvent( ev );
		return -1;
	};
	for( CWidget * w = cWidgets ; w ; w = w->getNext() ) 
	{
		if(!w->getEnabled())
			continue;
		if(w->InBox(tMouse->X,tMouse->Y))
		{
			FocusOnMouseClick( w );
			int ev = w->MouseDown(tMouse, tMouse->Down);
			if( ev >= 0 )
				w->ProcessGuiSkinEvent( ev );
			return -1;
		};
	};
	// Click on empty space
	FocusOnMouseClick( NULL );
	return -1;
};

int		CGuiSkinnedLayout::MouseWheelDown(mouse_t *tMouse)
{
	if( cChildLayout )
	{
		cChildLayout->MouseWheelDown(tMouse);
		return -1;
	};
	for( CWidget * w = cWidgets ; w ; w = w->getNext() ) 
	{
		if(!w->getEnabled())
			continue;
		if(w->InBox(tMouse->X,tMouse->Y))
		{
			int ev = w->MouseWheelDown(tMouse);
			if( ev >= 0 )
				w->ProcessGuiSkinEvent( ev );
			return -1;
		};
	};
	return -1;
};

int		CGuiSkinnedLayout::MouseWheelUp(mouse_t *tMouse)
{
	if( cChildLayout )
	{
		cChildLayout->MouseWheelUp(tMouse);
		return -1;
	};
	for( CWidget * w = cWidgets ; w ; w = w->getNext() ) 
	{
		if(!w->getEnabled())
			continue;
		if(w->InBox(tMouse->X,tMouse->Y))
		{
			int ev = w->MouseWheelUp(tMouse);
			if( ev >= 0 )
				w->ProcessGuiSkinEvent( ev );
			return -1;
		};
	};
	return -1;
};

int		CGuiSkinnedLayout::KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if( cChildLayout )
	{
		cChildLayout->KeyDown(c, keysym, modstate);
		return -1;
	};
	FocusOnKeyPress(c, keysym, false);
	if ( cFocused )
	{
		if(!cFocused->getEnabled())
			return -1;
		int ev = cFocused->KeyDown(c, keysym, modstate);
		if( ev >= 0 )
			cFocused->ProcessGuiSkinEvent( ev );
	};
	return -1;
};

int		CGuiSkinnedLayout::KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if( cChildLayout )
	{
		cChildLayout->KeyUp(c, keysym, modstate);
		return -1;
	};
	FocusOnKeyPress(c, keysym, true);
	if ( cFocused )
	{
		if(!cFocused->getEnabled())
			return -1;
		int ev = cFocused->KeyUp(c, keysym, modstate);
		if( ev >= 0 )
			cFocused->ProcessGuiSkinEvent( ev );
	};
	return -1;
};

void CGuiSkinnedLayout::FocusOnMouseClick( CWidget * w )
{
		if( cFocused == w )
			return;
		if( cFocused )
		{
			// Can we take the focus off?
			if (cFocused->CanLoseFocus())  
			{
				cFocused->setFocused(false);
				cFocused = NULL;
			}
			else  
			{
				//cFocused->setLoseFocus(true);
			};
		};
		if( w && cFocused == NULL )
		{
			w->setFocused(true);
			cFocused = w;
		};
};

void CGuiSkinnedLayout::FocusOnKeyPress(UnicodeChar c, int keysym, bool keyup)
{
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
				// Tab switches between widgets
				/*if ( c == SDLK_TAB && keyup )  
				{
					if (cFocused)  
					{
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
					} 
					else 
					{
						cFocused = cWidgets;
						cFocused->setFocused(true);
					}
					
					// Repeat the same thing until we find first enabled widget
					while (!cFocused->getEnabled())  
					{
						// The current one is not focused anymore
						cFocused->setFocused(false);
						
						if (cFocused->getNext())  {
							cFocused = cFocused->getNext();
							cFocused->setFocused(true);
						// The current focused widget is the last one in the list
						} 
						else 
						{
							cFocused = cWidgets;
							cFocused->setFocused(true);
						}
					}
				}*/
};

void CGuiSkinnedLayout::ExitDialog( const std::string & param, CWidget * source )
{
	CGuiSkinnedLayout * lp = (CGuiSkinnedLayout *) source->getParent();
	lp->bExitCurrentDialog = true;
	CGuiSkinnedLayout * ll = (CGuiSkinnedLayout *) lp->getParent();
	if( ll != NULL )
		ll->cChildLayout = NULL;
	lp->setParent( NULL );
};

void CGuiSkinnedLayout::ChildDialog( const std::string & param, CWidget * source )
{
	CGuiSkinnedLayout * lp = (CGuiSkinnedLayout *) source->getParent();
	if( lp->cChildLayout != NULL )
		return;
	// Simple parsing of params
	std::vector<std::string> v = explode(param, ",");
	for( unsigned i=0; i<v.size(); i++ )
		TrimSpaces(v[i]);
	int x = 0, y = 0;
	bool fullscreen = false;
	if( v.size() > 1 )
		if( v[1] == "fullscreen" )
			fullscreen = true;
	if( v.size() > 2 )
	{
		x = atoi( v[1] );
		y = atoi( v[2] );
	};
	std::string file = v[0];
	CGuiSkinnedLayout * ll = CGuiSkin::GetLayout( file );
	if( ll == NULL )
		return;
	ll->SetOffset(x,y);
	ll->bExitCurrentDialog = false;
	ll->setParent( lp );
	lp->cChildLayout = ll;
	lp->bChildLayoutFullscreen = fullscreen;
};

void CGuiSkinnedLayout::SubstituteDialog( const std::string & param, CWidget * source )
{
	CGuiSkinnedLayout * lp = (CGuiSkinnedLayout *) source->getParent();
	CGuiSkinnedLayout * ll = (CGuiSkinnedLayout *) lp->getParent();
	if( ll != NULL )
	{
		ExitDialog( "", source );
		CLabel dummy( "", tLX->clPink );
		dummy.setParent(ll);
		ChildDialog( param, &dummy );
	};
};

static bool bRegisteredCallbacks = CScriptableVars::RegisterVars("GUI")
	( & CGuiSkinnedLayout::ExitDialog, "ExitDialog" )
	( & CGuiSkinnedLayout::ChildDialog, "ChildDialog" )
	( & CGuiSkinnedLayout::SubstituteDialog, "SubstituteDialog" )	// Needed for tab-list emulation
	;
