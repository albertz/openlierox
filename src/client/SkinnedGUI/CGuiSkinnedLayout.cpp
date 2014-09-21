/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////



#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "SkinnedGUI/CGuiSkinnedLayout.h"
#include "SkinnedGUI/CGuiSkin.h"
#include "LieroX.h"
#include "Debug.h"
#include "Cursor.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "AuxLib.h"
#include "StringUtils.h"
#include "XMLutils.h"
#include "FindFile.h"


namespace SkinnedGUI {

////////////////
// Create
CGuiSkinnedLayout::CGuiSkinnedLayout() : CContainerWidget("_TopLayout", NULL) {
	cFocused = NULL;
	iType = wid_GuiLayout;
	iModalsRunning = 0;
	iClientWidth = iClientHeight = iClientX = iClientY = 0;
	bFullRepaint = false;
	cSkin = NULL;

	if (VideoPostProcessor::videoSurface().get())
		Resize(0, 0, VideoPostProcessor::videoSurface()->w, VideoPostProcessor::videoSurface()->h);
}

CGuiSkinnedLayout::CGuiSkinnedLayout(COMMON_PARAMS) : CContainerWidget(name, parent)
{
	cFocused = NULL;
	iType = wid_GuiLayout;
	iModalsRunning = 0;
	iClientWidth = iClientHeight = iClientX = iClientY = 0;
	bFullRepaint = false;
	cSkin = NULL;
}

///////////////////
// Shutdown the gui layout
CGuiSkinnedLayout::~CGuiSkinnedLayout()
{
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)
		delete (*w);

	cWidgets.clear();

	cFocused = NULL;
	setParent(NULL);
}

///////////////////
// Add a widget to the gui layout
// HINT: this is called from the widget constructor
void CGuiSkinnedLayout::Add(CWidget *widget)
{
	for (std::list<CWidget *>::iterator it = cWidgets.begin(); it != cWidgets.end(); it++)  {
		if (*it == widget)
			return; // Widget already added
		if ((*it)->getName() == widget->getName() && widget->getName() != STATIC && !(*it)->isDestroyed() && !(*it)->isDestroying())  {
			if ((*it)->getType() == widget->getType()) // Same name and type - this happens for default widgets
				return;
			else  { // Weird - the skinner probably defines an incorrect widget for the specified ID
				warnings << "CGuiSkinnedLayout::Add: warning, trying to add a widget with an ID that already exists" << endl;
				return;
			}
		}
	}

	if (widget->isModal())
		incModalsRunning();

	// Link the widget in
	cWidgets.push_back(widget);

	// Send a "Widget added" message to call DoCreate
	if (bCreated)
		cMainSkin->onAddWidget.pushToMainQueue(CGuiSkin::WidgetData(this, widget));
}


///////////////////////
// Sent when we add a child after the layout was created
void CGuiSkinnedLayout::DoChildAddEvent(CWidget *child)
{
	// Verify the child (it could be removed already)
	for (std::list<CWidget *>::iterator w = cWidgets.begin(); w != cWidgets.end(); w++)
		if ((*w) == child)  {
			child->ApplyCSS(cCSS);
			child->DoCreate();
			break;
		}
}

////////////////////
// Deregister a widget (remove it from the widget list, but don't free it)
// Used when moving the widget from one layout to another
void CGuiSkinnedLayout::DeregisterWidget(CWidget *w)
{
	for (std::list<CWidget *>::iterator it = cWidgets.begin(); it != cWidgets.end(); it++)
		if (*it == w)  {
			cWidgets.erase(it);

			// Repaint the rectangle where the widget was
			if (!w->isOverlapping())
				DoRepaintRect(w->getRect());
			break;
		}
}

///////////////////
// Focus a widget
void CGuiSkinnedLayout::FocusWidget(CWidget *w)
{
	if (w == NULL)  {
		if (cFocused)  {
			if (cFocused->DoLoseFocus(NULL) == WID_NOT_PROCESSED) // NOT_PROCESSED means that it cannot lose the focus
				return;
			cFocused->setFocused(false);
		}
		cFocused = NULL;
		return;
	}

	if (cFocused)  {
		if (cFocused != w)  {
			if (cFocused->DoLoseFocus(w) == WID_NOT_PROCESSED)
				return;
			if (w->DoFocus(cFocused) == WID_NOT_PROCESSED) // The widget being focused has not accepted the focus
				return;

			cFocused->setFocused(false);
			cFocused = w;
			w->setFocused(true);
		}
	} else  {
		if (w->DoFocus(NULL) == WID_NOT_PROCESSED)
			return;

		cFocused = w;
		cFocused->setFocused(true);
	}
}

////////////////////
// Move the layout by specified number of pixels
void CGuiSkinnedLayout::MoveBy( int dx, int dy )
{
	Resize(getX() + dx, getY() + dy, getWidth(), getHeight());
}

////////////////////
// Move the layout to a specified position
void CGuiSkinnedLayout::MoveTo(int x, int y)
{
	int dx = x - getX();
	int dy = y - getY();
	MoveBy(dx, dy);
}

////////////////////
// Resize the layout
void CGuiSkinnedLayout::Resize(int x, int y, int w, int h)
{
	// Apply the constraints first
	w = MAX(w, getMinWidth());
	h = MAX(h, getMinHeight());

	if (w != getWidth() && h != getHeight())  {
		bFullRepaint = true;

		// Notify the children about the resize
		// TODO: if one of the widgets changes the w and h, should we go through the list again?
		for( std::list<CWidget *>::iterator wid = cWidgets.begin() ; wid != cWidgets.end() ; wid++)  {
			(*wid)->DoParentResize(w, h); // HINT: the w and h can change here
		}
	}

	CContainerWidget::Resize(x, y, w, h);
	RecalculateClientRect();

	// Update the rectangles
	if (bmpBuffer.get())  {
		tBgRect = bmpBuffer->clip_rect;
		tBorderRect = tBgRect;
	}
}

////////////////
// Recalculate the client area rectangle
void CGuiSkinnedLayout::RecalculateClientRect()
{
	iClientX = getX() + cBorder.getLeftW();
	iClientY = getY() + cBorder.getTopW();
	iClientWidth = getWidth() - cBorder.getLeftW() - cBorder.getRightW();
	iClientHeight = getHeight() - cBorder.getTopW() - cBorder.getBottomW();
}

///////////////////
// Increase the number of modal children running
void CGuiSkinnedLayout::incModalsRunning()
{
	iModalsRunning++;
}

///////////////////
// Decrease the number of modal children running
void CGuiSkinnedLayout::decModalsRunning()
{
	iModalsRunning--;
	iModalsRunning = MAX(0, iModalsRunning);
}

////////////////
// Remove a widget
void CGuiSkinnedLayout::removeWidget(CWidget *widget)  
{
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)  {
		if (widget == (*w))  {

			// If this is the focused widget, set focused to null
			if(*w == cFocused)
				FocusWidget(NULL);

			// If modal, decrease the modal count
			if((*w)->isModal())  {
				decModalsRunning();
			}

			// Call DoDestroy
			(*w)->DoDestroy(false);

			// HINT: the widget is freed later, after it finishes with
			// its deinitialization (for example effects)

			break;
		}
	}

}

///////////////////
// Remove a widget by name
void CGuiSkinnedLayout::removeWidget(const std::string& name)
{
	removeWidget(getWidgetByName(name));
}

///////////////////
// Child has changed its position or size
int CGuiSkinnedLayout::DoChildResize(CWidget *child, const SDL_Rect& oldrect, const SDL_Rect& newrect)
{
	// Repaint the necessary areas
	if (child->isOverlapping())
		return WID_PROCESSED; // Overlapping widgets are not drawn on the buffer, no need to repaint
	if (bFullRepaint) // If we are expecting a full repaint, don't do anything
		return WID_PROCESSED;

	if (ContainsRect(oldrect, newrect))
		tRectsToRepaint.push_back(oldrect);
	else if (ContainsRect(newrect, oldrect))
		tRectsToRepaint.push_back(newrect);
	else  {
		tRectsToRepaint.push_back(oldrect);
		tRectsToRepaint.push_back(newrect);
	}
	Repaint();

	return WID_PROCESSED;
}

///////////////////////
// A child needs to be repainted
int CGuiSkinnedLayout::DoChildNeedsRepaint(CWidget *child)
{
	// HINT: the repainting itself is done before drawing - there could be more
	// repaints queued later that could be merged etc.
	// Here we only set our Repaint flag to true and notiy our parent to repaint us
	bNeedsRepaint = true;
	if (cParent)
		cParent->DoChildNeedsRepaint(this);

	return WID_PROCESSED;
}


///////////////////
// Draw the widgets
void CGuiSkinnedLayout::DoRepaint()
{
	CHECK_BUFFER;

	bNeedsRepaint = false;

	if (bFullRepaint)  { // A full repaint?
		DoRepaintChildren();
		DoRepaintRect(getRect()); // Repaint the whole buffer
		bFullRepaint = false;
	} else if (tRectsToRepaint.size() > 0)  {
		DoRepaintChildren(); // First repaint the children
		for (std::list<SDL_Rect>::iterator r = tRectsToRepaint.begin(); r != tRectsToRepaint.end(); r++)
			DoRepaintRect(*r);
		tRectsToRepaint.clear();
	} else
		DoRepaintChildren();

}

///////////////////
// Repaints a rectangle of the layout
void CGuiSkinnedLayout::DoRepaintRect(const SDL_Rect& r)
{
	CHECK_BUFFER;

	ScopedSurfaceClip clipper(bmpBuffer.get(), r);

	// Clear the buffer
	FillSurfaceTransparent(bmpBuffer.get());;

	// Background
	cBackground.Draw(bmpBuffer.get(), 0, 0, getWidth(), getHeight());

	// Widgets in the rect
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)  {
		if (!(*w)->isCreated())
			continue;

		// Check if the widget intersects with the given rect
		if ( ! SDLRect(r).clipWith(SDLRect( (*w)->getRect() )) )
			continue;

		if ((*w)->getVisible())  {
			// Debug: draw a red rect around the focused widget
			//if ((*w) == cFocused)
			//	DrawRect(bmpBuffer.get(), (*w)->getX(), (*w)->getY(), (*w)->getX() + (*w)->getWidth(), (*w)->getY() + (*w)->getHeight(), SDL_MapRGB(bmpBuffer->format, 255, 0, 0));
			(*w)->Draw(bmpBuffer.get(), (*w)->getX(), (*w)->getY());
		}
	}

	// Border
	cBorder.Draw(bmpBuffer.get(), 0, 0, getWidth(), getHeight());
}

///////////////////
// Calls DoRepaint on widgets that need it
void CGuiSkinnedLayout::DoRepaintChildren()
{
	// Repaint the widgets
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)
	{
		// Repaint the widget if needed
		if ((*w)->needsRepaint() && (*w)->isCreated())  {
			(*w)->DoRepaint();
			DoRepaintRect((*w)->getRect());
		}
	}
}

////////////////////
// Create event
int CGuiSkinnedLayout::DoCreate()
{
	bFullRepaint = true;

	// Update the rectangles
	if (bmpBuffer.get())  {
		tBgRect = bmpBuffer->clip_rect;
		tBorderRect = tBgRect;
	}

	// Call DoCreate on the children
	for (std::list<CWidget *>::iterator w = cWidgets.begin(); w != cWidgets.end(); w++)
		(*w)->DoCreate();

	return CContainerWidget::DoCreate();
}

//////////////////////
// A child calls this when it's ready to be destroyed
int CGuiSkinnedLayout::DoChildDestroyed(CWidget *child)
{
	CContainerWidget::DoChildDestroyed(child);

	// Check
	if (child == NULL)  {
		errors << "Destroying a NULL child" << endl;
		return WID_NOT_PROCESSED;
	}

	// We cannot destroy the child now because it called us (i.e. its function is still running)
	// Just send a self-reminder
	cMainSkin->onDestroyWidget.pushToMainQueue(CGuiSkin::WidgetData(this, child));

	return WID_PROCESSED;
}

/////////////////////
// After receiving the event from the above function, this function is executed
void CGuiSkinnedLayout::DoChildDestroyEvent(CWidget *child)
{
	// Check if it's still in our list
	for (std::list<CWidget *>::iterator w = cWidgets.begin(); w != cWidgets.end(); w++)
		if (*w == child)  {
			if (!child->isDestroyed())  {
				warnings << "An attempt to remove a widget that has not been marked as destroyed" << endl;
				return;
			}

			// If the child is a focused widget, remove the focus
			if (cFocused == child)
				FocusWidget(NULL);

			// Remove the child
			SDL_Rect r = (*w)->getRect();
			bool overlapping = (*w)->isOverlapping();
			delete (*w);
			cWidgets.erase(w);

			// Now when the widget is gone, repaint the place where it was, unless it was an oveerlapping widget
			// (dialog, menu, etc.)
			if (!overlapping)
				DoRepaintRect(r);
			break;
		}
}


///////////////////
// Process all the widgets
void CGuiSkinnedLayout::Process()
{
	// TODO: get rid of this function, it currently does nothing at all

	// Call process on all children
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)  {
		(*w)->Process();
	}
}

/////////////////////
// Draw the layout to the specified surface
void CGuiSkinnedLayout::Draw(SDL_Surface *bmpDest, int drawX, int drawY)
{
	// Buffer + effects
	CWidget::Draw(bmpDest, drawX, drawY);

	// Overlapping widgets (those that are bigger than the buffer and cannot be drawn on it,
	// for example dialogs or menus)
	for( std::list<CWidget *>::iterator w = cWidgets.begin() ; w != cWidgets.end() ; w++)  {
		if ((*w)->isOverlapping())
			(*w)->Draw(bmpDest, drawX + (*w)->getX(), drawY + (*w)->getY());
	}
}


////////////////////
// Get the widget based on its name
CWidget	*CGuiSkinnedLayout::getWidgetByName(const std::string & name)
{
	for (std::list<CWidget *>::iterator w = cWidgets.begin(); w != cWidgets.end(); w++)
		if ((*w)->getName() == name)
			return *w;

	return NULL;
}

////////////////////
// Notifies about the error that occured
void CGuiSkinnedLayout::Error(int ErrorCode, const std::string& desc)
{
	errors << "GUI skin error: " << ErrorCode << " " << desc << endl;
}

///////////////////
// Mouse enter event
int CGuiSkinnedLayout::DoMouseEnter(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	return WID_PROCESSED;
}

///////////////////
// Mouse leave event
int CGuiSkinnedLayout::DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	return WID_PROCESSED;
}

////////////////////
// Mouse move event
int CGuiSkinnedLayout::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	SetGameCursor(CURSOR_ARROW); // Set default mouse cursor - widget will change it

	bool ignore_mouse_move = false;
	for( std::list<CWidget *>::reverse_iterator w = cWidgets.rbegin() ; w != cWidgets.rend() ; w++) {

		// No events for disabled or invisible widgets
		if(!(*w)->getEnabled() || !(*w)->getVisible())
			continue;

		if((*w)->InBox(x, y) && !ignore_mouse_move)  {

			// If in the previous frame the widget wasn't under the mouse and now it is, fire the "enter" event
			if (!(*w)->InBox(x - dx, y - dy) || !(*w)->isMouseOver())
				(*w)->DoMouseEnter(x - (*w)->getX(), y - (*w)->getY(), dx, dy, modstate);

			// Mouse move event
			if ((*w)->DoMouseMove(x - (*w)->getX(), y - (*w)->getY(), dx, dy, down, button, modstate) == WID_PROCESSED)
				ignore_mouse_move = true; // Ignore any other mouse enter/move events that might occur (widget overlay) and only check for a mouse leave
		} else {
			// If in the previous frame the widget was under the mouse and now it is not, fire the "leave" event
			// If the ignore_mouse_move is true, which means that another widget is under the mouse, fire the leave event as well
			if ((*w)->InBox(x - dx, y - dy))  {
				(*w)->DoMouseLeave(x - (*w)->getX(), y - (*w)->getY(), dx, dy, modstate);
			} else if (ignore_mouse_move && (*w)->InBox(x, y))
				(*w)->DoMouseLeave(x - (*w)->getX(), y - (*w)->getY(), dx, dy, modstate);
		}

	}

	// If there's a focused widget, dispatch the event to it as well
	if (cFocused)
		cFocused->DoMouseMove(x - cFocused->getX(), y - cFocused->getY(), dx, dy, down, button, modstate);

	return WID_PROCESSED;
}

////////////////////////
// Mouse up event
int CGuiSkinnedLayout::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// HINT: this event is only for focused widgets

	// Send the mouse up event to this widget even though the mouse does not have to be over it anymore
	// This is the common behavior of all windowing systems
	if (cFocused)  {
		cFocused->DoMouseUp(x - cFocused->getX(), y - cFocused->getY(), dx, dy, button, modstate);
	}

	return WID_PROCESSED;
}

//////////////////////
// Mouse down event
int CGuiSkinnedLayout::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Modal widget focused?
	if (cFocused)
		if (cFocused->isModal())  {
			cFocused->DoMouseDown(x - cFocused->getX(), y - cFocused->getY(), dx, dy, button, modstate);
			return WID_PROCESSED;
		}

	for( std::list<CWidget *>::reverse_iterator w = cWidgets.rbegin() ; w != cWidgets.rend() ; w++)  {

		// Don't process disabled or invisible widgets
		if(!(*w)->getEnabled() || !(*w)->getVisible())
			continue;

		if((*w)->InBox(x, y))  {

			// If there are some modal widgets, process only them
			if (iModalsRunning > 0 && !(*w)->isModal())
				continue;

			// Focus the widget
			FocusWidget(*w);


			if ((*w)->DoMouseDown(x - (*w)->getX(), y - (*w)->getY(), dx, dy, button, modstate) == WID_PROCESSED)
				return WID_PROCESSED;
		}
	}

	// Click on empty space
	// Take off the focus if any
	FocusWidget(NULL);

	return WID_PROCESSED;
}

/////////////////////
// Mouse wheel down event
int CGuiSkinnedLayout::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	// Modal widget focused?
	if (cFocused)
		if (cFocused->isModal())  {
			cFocused->DoMouseWheelDown(x - cFocused->getX(), y - cFocused->getY(), dx, dy, modstate);
			return WID_PROCESSED;
		}

	for( std::list<CWidget *>::reverse_iterator w = cWidgets.rbegin() ; w != cWidgets.rend() ; w++)  {

		// Don't process disabled and invisible widgets
		if(!(*w)->getEnabled() || !(*w)->getVisible())
			continue;

		if((*w)->InBox(x, y))  {

			// If there are some modal widgets, process only them
			if (iModalsRunning > 0 && !(*w)->isModal())
				continue;

			// Fire the event
			if ((*w)->DoMouseWheelDown(x - (*w)->getX(), y - (*w)->getY(), dx, dy, modstate) == WID_PROCESSED)
				return WID_PROCESSED;
		}
	}

	return WID_PROCESSED;
}

/////////////////////
// Mouse wheel up event
int CGuiSkinnedLayout::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	// Modal widget focused?
	if (cFocused)
		if (cFocused->isModal())  {
			cFocused->DoMouseWheelUp(x - cFocused->getX(), y - cFocused->getY(), dx, dy, modstate);
			return WID_PROCESSED;
		}

	for( std::list<CWidget *>::reverse_iterator w = cWidgets.rbegin() ; w != cWidgets.rend() ; w++)  {

		// Don't process disabled and invisible widgets
		if(!(*w)->getEnabled() || !(*w)->getVisible())
			continue;

		if((*w)->InBox(x, y))  {

			// If there are some modal widgets, process only them
			if (iModalsRunning > 0 && !(*w)->isModal())
				continue;

			// Fire the event
			if ((*w)->DoMouseWheelUp(x - (*w)->getX(), y - (*w)->getY(), dx, dy, modstate) == WID_PROCESSED)
				return WID_PROCESSED;
		}
	}

	return WID_PROCESSED;
}

///////////////////////
// Key down event
int CGuiSkinnedLayout::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if (cFocused)  {
		// If there are some modals but the focused widget is not modal, ignore the event
		if (iModalsRunning > 0 && !cFocused->isModal())
			return WID_NOT_PROCESSED;

		if(!cFocused->getEnabled() || !cFocused->getVisible())
			return WID_PROCESSED;

		if (cFocused->DoKeyDown(c, keysym, modstate) == WID_PROCESSED)
			return WID_PROCESSED;
	}

	return WID_PROCESSED;
}

////////////////////////
// Key up event
int CGuiSkinnedLayout::DoKeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if (cFocused)  {
		// If there are some modals but the focused widget is not modal, ignore the event
		if (iModalsRunning > 0 && !cFocused->isModal())
			return WID_NOT_PROCESSED;

		if(!cFocused->getEnabled() || !cFocused->getVisible())
			return -1;

		if (cFocused->DoKeyUp(c, keysym, modstate) == WID_PROCESSED)
			return WID_PROCESSED;
	}

	return WID_PROCESSED;
}

/////////////////////////
// Focus the first textbox on a key press
// NOTE: not currently used
void CGuiSkinnedLayout::FocusOnKeyPress(UnicodeChar c, int keysym, bool keyup)
{
	// If we don't have any focused widget, get the first textbox
	if (!cFocused)  {
		for( std::list<CWidget *>::iterator txt = cWidgets.begin() ; txt != cWidgets.end() ; txt++)  {
			if ((*txt)->getType() == wid_Textbox && (*txt)->getEnabled()) {
				cFocused = *txt;
				(*txt)->setFocused(true);
				break;
			}
		}
	}
}

////////////////////////
// Get a widget at a specified point, if no widget found, returns this pointer
CWidget * CGuiSkinnedLayout::getWidgetAtPoint(int x, int y)
{
	for( std::list<CWidget *>::reverse_iterator w = cWidgets.rbegin() ; w != cWidgets.rend() ; w++)
		if ((*w)->getEnabled() && (*w)->InBox(x, y))
			return (*w);

	return this;
}

/////////////////////////
// Load the layout definitions from a HTML file
bool CGuiSkinnedLayout::Load(const std::string& file, CGuiSkin& skin)
{
	cCSS.clear();

	cSkin = &skin;

	// Get the file contents
	std::string xml = GetFileContents(skin.getSkinFilePath(file));
	if (xml.find("<?xml") == std::string::npos)
		xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + xml;

	// Parse the document
	xmlDocPtr document = xmlReadDoc((const xmlChar *)xml.data(), 
		(ExtractDirectory(skin.getSkinFilePath(file)).c_str()), "", 0);

	if (!document)  {
		return false;
	}

	if (!document->children || !document->children->children)  {
		xmlFreeDoc(document);
		return false;
	}

	// Build the layout
	ApplyTag(document->children);

	// Apply the CSS
	ApplyCSS(cCSS);

	// Cleanup
	xmlFreeDoc(document);

	return true;
}

///////////////////////
// Apply the given selector to this layout
void CGuiSkinnedLayout::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CContainerWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
}

////////////////////////
// Read the CSS and apply it
void CGuiSkinnedLayout::ApplyCSS(CSSParser &css)
{
	CContainerWidget::ApplyCSS(css);

	// Go through the children and apply the new CSS to them
	for (std::list<CWidget *>::iterator w = cWidgets.begin(); w != cWidgets.end(); w++)
		(*w)->ApplyCSS(css);
}

//////////////////////////
// Processes special tags like style, link, script etc.
// Returns true if the given node is really a special tag
bool CGuiSkinnedLayout::ProcessSpecialTag(xmlNodePtr node, CGuiSkin& skin)
{
	// Style
	if (!xmlStrcasecmp(node->name, (const xmlChar *)"style"))  {
		cCSS.parse(xmlNodeText(node, ""), xmlGetBaseURL(node));

		return true;

	// Script
	} else if (!xmlStrcasecmp(node->name, (const xmlChar *)"script"))  {
		// TODO: scripting :)

		return true;

	// Link (currently only for CSS)
	} else if (!xmlStrcasecmp(node->name, (const xmlChar *)"link"))  {
		std::string type = xmlGetString(node, "type", "text/css");
		std::string rel = xmlGetString(node, "rel", "stylesheet");
		if (!stringcaseequal(type, "text/css") || !stringcaseequal(rel, "stylesheet")) // Make sure it's a CSS
			return true;

		std::string cssurl = xmlGetString(node, "href");
		if (cssurl.size())  {
			cCSS.parse(GetFileContents(skin.getSkinFilePath(cssurl)), ExtractDirectory(skin.getSkinFilePath(cssurl)));
		}
	}

	return false;
}

/////////////////////////
// Build a layout from the given tag (this tag is usually given by ::Load)
void CGuiSkinnedLayout::ApplyTag(xmlNodePtr node)
{
	CContainerWidget::ApplyTag(node);

	// Build the layout
	xmlNodePtr child = node->children;
	for (; child; child = child->next)  {
		std::string widget_name = xmlGetString(child, "id", STATIC);

		// Check if a widget with the same name exists
		if (widget_name != STATIC)  {
			CWidget *wid = getWidgetByName(widget_name);

			// If a widget with the same name already exists, just update it
			if (wid != NULL)  {
				wid->ApplyTag(child);
				continue;
			}
		}

		// Process non-widget tags
		if (ProcessSpecialTag(child, *cSkin))
			continue;

		// The widget does not exist, let's create it
		CWidget *new_widget = cMainSkin->CreateWidgetByTagName((const char *)child->name, this, widget_name);
		if (!new_widget) // Probably an unknown tag, just ignore it
			continue;

		new_widget->ApplyTag(child); // The widget class takes care of the rest
	}
}

}; // namespace SkinnedGUI
