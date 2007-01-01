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
void CGuiLayout::Initialize(int LayoutID)
{
	Shutdown();

	iID = LayoutID;

	cWidgets = NULL;
	tEvent = new gui_event_t;
	cFocused = NULL;
	cMouseOverWidget = NULL;

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

	cFocused = NULL;
	cMouseOverWidget = NULL;
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

//////////////////
// Reads common events, that are available for almost every widget
void CGuiLayout::ReadEvents(xmlNodePtr Node, generic_events_t *Events)
{
	// Load the values
	xmlChar *evs[NumEvents];
	evs[OnMouseOver] = xmlGetProp(Node,(const xmlChar *)"onmouseover");
	evs[OnMouseOut]  = xmlGetProp(Node,(const xmlChar *)"onmouseout");
	evs[OnMouseDown] = xmlGetProp(Node,(const xmlChar *)"onmousedown");
	evs[OnClick]	 = xmlGetProp(Node,(const xmlChar *)"onclick");

	// Copy the values into the events
	int i;
	for (i=0;i<NumEvents;i++)  {
		if (evs[i]) {
			fix_strncpy(Events->Events[i], (char *)evs[i]);
		} else
			Events->Events[i][0] = '\0';
	}

	// Free the data
	xmlFree(evs[OnMouseOver]);
	xmlFree(evs[OnMouseOut]);
	xmlFree(evs[OnMouseDown]);
	xmlFree(evs[OnClick]);
}

//////////////////
// Build the layout according to code specified in skin file
bool CGuiLayout::Build(void)
{

	//
	//	1. Get the file to parse
	//

	char *sFilename = NULL;

	// Default skin extension
	char sExtension[4];
	strcpy(sExtension,"skn");

	// Get the skin path
	size_t skinpathlen = fix_strnlen(tLXOptions->sSkinPath);
	size_t reslen = fix_strnlen(tLXOptions->sResolution);
	char *path = new char[skinpathlen+reslen+1];
	size_t pathlen = skinpathlen+reslen+1;
	if (!path)  {
		Error(ERR_OUTOFMEMORY,"%s","Out of memory.");
		return false;
	}
	memcpy(path, tLXOptions->sSkinPath, skinpathlen);
	memcpy(path+skinpathlen, tLXOptions->sResolution, reslen+1);

	// Temp
	static char file[32];

	// Get the file name of the skin file
	switch (iID)  {
		case L_MAINMENU: strcpy(file,"mainmenu"); break;
		case L_LOCALPLAY: strcpy(file,"localplay"); break;
		case L_GAMESETTINGS: strcpy(file,"gamesettings"); break;
		case L_WEAPONOPTIONS: strcpy(file,"weaponoptions"); break;
		case L_LOADWEAPONS: strcpy(file,"loadweapons"); break;
		case L_SAVEWEAPONS: strcpy(file,"saveweapons"); break;
		case L_NET: strcpy(file,"net"); break;
		case L_NETINTERNET: strcpy(file,"netinternet"); break;
		case L_INTERNETDETAILS: strcpy(file,"internetdetails"); break;
		case L_ADDSERVER: strcpy(file,"addserver"); break;
		case L_NETLAN: strcpy(file,"netlan"); break;
		case L_LANDETAILS: strcpy(file,"landetails"); break;
		case L_NETHOST: strcpy(file,"nethost"); break;
		case L_NETFAVOURITES: strcpy(file,"netfavourites"); break;
		case L_FAVOURITESDETAILS: strcpy(file,"favouritesdetails"); break;
		case L_RENAMESERVER: strcpy(file,"renameserver"); break;
		case L_ADDFAVOURITE: strcpy(file,"addfavourite"); break;
		case L_CONNECTING: strcpy(file,"connecting"); break;
		case L_NETJOINLOBBY: strcpy(file,"netjoinlobby"); break;
		case L_NETHOSTLOBBY: strcpy(file,"nethostlobby"); break;
		case L_SERVERSETTINGS: strcpy(file,"serversettings"); break;
		case L_BANLIST: strcpy(file,"banlist"); break;
		case L_PLAYERPROFILES: strcpy(file,"playerprofiles"); break;
		case L_CREATEPLAYER: strcpy(file,"createplayer"); break;
		case L_VIEWPLAYERS: strcpy(file,"viewplayers"); break;
		case L_LEVELEDITOR: strcpy(file,"leveleditor"); break;
		case L_NEWDIALOG: strcpy(file,"newdialog"); break;
		case L_SAVELOADLEVEL: strcpy(file,"saveloadlevel"); break;
		case L_OPTIONS: strcpy(file,"options"); break;
		case L_OPTIONSCONTROLS: strcpy(file,"optionscontrols"); break;
		case L_OPTIONSGAME: strcpy(file,"optionsgame"); break;
		case L_OPTIONSSYSTEM: strcpy(file,"optionssystem"); break;
		case L_MESSAGEBOXOK: strcpy(file,"messageboxok"); break;
		case L_MESSAGEBOXYESNO: strcpy(file,"messageboxyesno"); break;
		default: strcpy(file, "fuckingshit");
	}

	// Get the Filename + Path
	size_t len = pathlen+strlen(file)+strlen(sExtension)+1;
	sFilename = new char[len];
	if(!sFilename)  {
		Error(ERR_OUTOFMEMORY,"%s","Out of memory.");
		return false;
	}	
	snprintf(sFilename,len,"%s/%s.%s",path,file,sExtension);
	dyn_markend(sFilename,len);

	//
	//	2. Parse the file
	//

	xmlDocPtr	tDocument;
	xmlNodePtr	tCurrentNode;

	// Parse the document
	tDocument = xmlParseFile(sFilename);
	if (tDocument == NULL)  {
		Error(ERR_COULDNOTPARSE,"Could not parse the document %s",sFilename);
		return false;
	}

	// Get the root node
	tCurrentNode = xmlDocGetRootElement(tDocument);
	if (tCurrentNode == NULL)  {
		Error(ERR_EMPTYDOC,"The '%s' document is empty",sFilename);
		return false;
	}

	// Validate the root node
	if (CMP(tCurrentNode->name,"skin"))  {
		Error(ERR_INVALIDROOT,"The document '%s' contains invalid parent node: %s",sFilename,tCurrentNode->name);
		return false;
	}

	// Get the first child
	tCurrentNode = tCurrentNode->xmlChildrenNode;

	// Get all the nodes in document and create widgets in layout
	while (tCurrentNode != NULL)  {

		// Box
		if (CMP(tCurrentNode->name,"box"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			int round  = xmlGetInt(tCurrentNode,"round");
			int border = xmlGetInt(tCurrentNode,"border");
			Uint32 light_colour = xmlGetColour(tCurrentNode,"lightcolor");
			Uint32 dark_colour = xmlGetColour(tCurrentNode,"darkcolor");
			Uint32 bgcolour = xmlGetColour(tCurrentNode,"bgcolor");

			Add(new CBox(round,border,light_colour,dark_colour,bgcolour),-1,left,top,width,height);
		}

		// Image
		if (CMP(tCurrentNode->name,"img"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src  = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the image
			CImage *Image = new CImage((char *) src);
			Add(Image,GetIdByName(name),left,top,width,height);
			Image->SetupEvents(&Events);

			// Free resources
			xmlFree(name);
			xmlFree(src);

		}

		// Button
		if (CMP(tCurrentNode->name,"button"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src	 = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the button
			CButton *Button = new CButton((char *)src);
			Add(Button,GetIdByName(name),left,top,width,height);
			Button->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(src);
		}


		// Checkbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the checkbox
			CCheckbox *Checkbox = new CCheckbox(false);
			Add(Checkbox,GetIdByName(name),left,top,width,height);
			Checkbox->SetupEvents(&Events);

			xmlFree(name);
		}


		// Combobox
		if (CMP(tCurrentNode->name,"combobox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the combobox
			CCombobox *Combobox = new CCombobox();
			Add(Combobox,GetIdByName(name),left,top,width,height);
			Combobox->SetupEvents(&Events);

			xmlFree(name);
		}

		// Inputbox
		if (CMP(tCurrentNode->name,"inputbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *image	 = xmlGetProp(tCurrentNode,(const xmlChar *)"image");
			xmlChar *title	 = xmlGetProp(tCurrentNode,(const xmlChar *)"title");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			//CInputbox *Inputbox = new CInputbox(0,"",(char *)image,(char *)title);
			//Add(Inputbox,GetIdByName(name),left,top,width,height);
			//Inputbox->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(image);
			xmlFree(title);
		}

		// Label
		if (CMP(tCurrentNode->name,"label"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *text = xmlGetProp(tCurrentNode,(const xmlChar *)"text");
			Uint32 colour = xmlGetColour(tCurrentNode,"color");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the label
			CLabel *Label = new CLabel((char *)text,colour);
			Add(Label,GetIdByName(name),left,top,width,height);
			Label->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(text);
		}

		// Listview
		if (CMP(tCurrentNode->name,"listview"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the listview
			CListview *Listview = new CListview();
			Add(Listview,GetIdByName(name),left,top,width,height);
			Listview->SetupEvents(&Events);

			xmlFree(name);
		}

		// Scrollbar
		if (CMP(tCurrentNode->name,"scrollbar"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the scrollbar
			CScrollbar *Scrollbar = new CScrollbar();
			Add(Scrollbar,GetIdByName(name),left,top,width,height);
			Scrollbar->SetupEvents(&Events);

			xmlFree(name);
		}

		// Slider
		if (CMP(tCurrentNode->name,"slider"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the slider
			CSlider *Slider = new CSlider(1);
			Add(Slider,GetIdByName(name),left,top,width,height);
			Slider->SetupEvents(&Events);

			xmlFree(name);
		}

		// Textbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			// Add the textbox
			CTextbox *Textbox = new CTextbox();
			Add(Textbox,GetIdByName(name),left,top,width,height);
			Textbox->SetupEvents(&Events);

			xmlFree(name);
		}


		tCurrentNode = tCurrentNode->next;
	}

	return true;
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

	if (!tEvent)  {
		tEvent = new gui_event_t;
		if (!tEvent)
			return NULL;
	}

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
				
				// Keydown
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

////////////////////
// Get the widget ID
int	CGuiLayout::GetIdByName(xmlChar *Name)
{
	int ID = -1;
	// Find the standard or previously added widget
	ID = LayoutWidgets[iID].getID((char *)Name);

	// Non-standard widget, add it to the list
	if (ID == -1)
		ID = LayoutWidgets[iID].Add((char *)Name);
	return ID;
}

////////////////////
// Notifies about the error that occured
void CGuiLayout::Error(int ErrorCode, char *Format, ...)
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
