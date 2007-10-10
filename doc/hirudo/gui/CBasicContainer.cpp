// CBasicContainer class implementation

#include "CBasicContainer.h"


/////////////////
// Find the specified child by ID
ContainerList::iterator CBasicContainer::FindChild(const std::string& id)
{
	// Go through the children looking for the ID
	ContainerList::iterator res = tChildren.begin();
	for (; res != tChildren.end(); res++)  {
		if ( (*res)->sID == id)
			return res;
	}

	return tChildren.end();
}

/////////////////
// Add a child
void CBasicContainer::AddChild(CBasicContainer *child)
{
	tChildren.push_back(child);
	child->tParent = this;

	// If no child is focused, set a focus to this child
	if (!tFocusedChild)  {
		child->bFocused = true;
		tFocusedChild = child;
	}
}


/////////////////
// Get the child by ID, returns NULL when not found
CBasicContainer *CBasicContainer::GetChild(const std::string& id)
{
	ContainerList::iterator i = FindChild(id);
	if (i == tChildren.end())
		return NULL;
	else
		return *i;
}

///////////////
// Removes a child
void CBasicContainer::RemoveChild(const std::string& id)
{
	ContainerList::iterator i = FindChild(id);
	if (i != tChildren.end())
		tChildren.erase(i);
}

///////////////
// Draw all the children
void CBasicContainer::DrawChildren(CSurface &dest)
{
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)
		if ( (*c)->bVisible )
			(*c)->Draw(dest);
}

//////////////
// Ask the children if they need a repaint
// HINT: if they need a repaint, our repaint property is automatically set to true
bool CBasicContainer::ChildrenNeedRepaint()
{
	// Go through the children
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)
		if ( (*c)->bRepaintRequired)  {
			bRepaintRequired = true;
			return true;
		}

	// No repaint required
	return false;
}

////////////////
// Free all children that should be freed automatically
CBasicContainer::~CBasicContainer()
{
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)  {
		if ((*c)->bFreeAutomatically)
			delete *c;
	}

	tChildren.clear();
}

//
// Events
//

#define FOREACH_CHILD for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++) \
						if ( (*c)->bEnabled && (*c)->bVisible)

bool CBasicContainer::ProcessMouseClick(int x, int y, MouseButtons buttons, KeyState keys)
{
	FOREACH_CHILD {
			if ( (*c)->InBox(x, y) )  {  // Check if in box
				// Take off focus of the currently focused widgets
				if (tFocusedChild)
					tFocusedChild->bFocused = false;

				// Click
				(*c)->DoMouseClick(x, y, buttons, keys);

				// Put the focus to this child
				tFocusedChild = *c;
				tFocusedChild->bFocused = true;

				return true;
			}
	}

	return false;
}

bool CBasicContainer::ProcessMouseDown(int x, int y, MouseButtons buttons, KeyState keys)
{
	if (tFocusedChild)  {
		tFocusedChild->DoMouseDown(x, y, buttons, keys);
		return true;
	}

	return false;
}

bool CBasicContainer::ProcessMouseMove(int x, int y, MouseButtons buttons, KeyState keys)
{
	if (tFocusedChild)  {
		if (tFocusedChild->InBox(x, y))  {
			if (!tFocusedChild->bWasMouseOver)
				tFocusedChild->DoMouseOver();
			tFocusedChild->DoMouseMove(x, y, buttons, keys);
			tFocusedChild->bWasMouseOver = true;

			return true;
		} else {
			if (tFocusedChild->bWasMouseOver)  {
				tFocusedChild->DoMouseOut();
				tFocusedChild->bWasMouseOver = false;
			}
		}
	}

	return false;

}

bool CBasicContainer::ProcessMouseWheelUp(int x, int y, MouseButtons buttons, KeyState keys)
{
	if (tFocusedChild)  {
		tFocusedChild->DoMouseWheelUp(x, y, buttons, keys);
		return true;
	}

	return false;

	// Dispatch this event to the control under the mouse or the focused control?
	/*FOREACH_CHILD  
		if ( (*c)->InBox(x, y))  {
			(*c)->DoMouseWheelUp(x, y, buttons, keys);
			return true;
		}
	*/
}

bool CBasicContainer::ProcessMouseWheelDown(int x, int y, MouseButtons buttons, KeyState keys)
{
	if (tFocusedChild)  {
		tFocusedChild->DoMouseWheelDown(x, y, buttons, keys);
		return true;
	}

	return false;

	// Dispatch this event to the control under the mouse or the focused control?
	/*FOREACH_CHILD
		if ( (*c)->InBox(x, y))  {
			(*c)->DoMouseWheelDown(x, y, buttons, keys);
			return true;
		}
	
	*/
}

bool CBasicContainer::ProcessKeyDown(UnicodeChar c, KeyState keys)
{
	if (tFocusedChild)  {
		tFocusedChild->DoKeyDown(c, keys);
		return true;
	}

	return false;
}

bool CBasicContainer::ProcessKeyUp(UnicodeChar c, KeyState keys)
{
	if (tFocusedChild)  {
		tFocusedChild->DoKeyUp(c, keys);
		return true;
	}

	return false;
}

#undef FOREACH_CHILD

//
// Default event handlers, child classes can freely override them
//

void CBasicContainer::DoMouseClick(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Dispatch to children
	if (ProcessMouseClick(x, y, buttons, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onClick event
	if (OnClick)
		OnClick(this, x, y, buttons, keys);
}
void CBasicContainer::DoMouseDown(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Dispatch to children
	if (ProcessMouseDown(x, y, buttons, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onMouseDown event
	if (OnMouseDown)
		OnMouseDown(this, x, y, buttons, keys);
}

void CBasicContainer::DoMouseMove(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Dispatch to children
	if (ProcessMouseMove(x, y, buttons, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onMouseMove event
	if (OnMouseMove)
		OnMouseMove(this, x, y, buttons, keys);
}

void CBasicContainer::DoMouseWheelUp(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Dispatch to children
	if (ProcessMouseWheelUp(x, y, buttons, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onMouseWheelUp event
	if (OnMouseWheelUp)
		OnMouseWheelUp(this, x, y, buttons, keys);
}

void CBasicContainer::DoMouseWheelDown(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Dispatch to children
	if (ProcessMouseWheelDown(x, y, buttons, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onMouseWheelDown event
	if (OnMouseWheelDown)
		OnMouseWheelDown(this, x, y, buttons, keys);
}

void CBasicContainer::DoKeyDown(UnicodeChar c, KeyState keys)
{
	// Dispatch to children
	if (ProcessKeyDown(c, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onKeyDown event
	if (OnKeyDown)
		OnKeyDown(this, c, keys);
}

void CBasicContainer::DoKeyUp(UnicodeChar c, KeyState keys)
{
	// Dispatch to children
	if (ProcessKeyUp(c, keys))
		return; // Some child took this event, we don't care anymore

	// Trigger onKeyUp event
	if (OnKeyUp)
		OnKeyUp(this, c, keys);
}