/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Widget class
// Created 5/6/02
// Jason Boettcher


#include "SkinnedGUI/CWidget.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "XMLutils.h"
#include "SDLEvents.h"


namespace SkinnedGUI {

const std::string STATIC = "Static";

/////////////////
// Create the widget
CWidget::CWidget(COMMON_PARAMS)
{
	iType = wid_None;
	iX.set(0, DEFAULT_PRIORITY);
	iY.set(0, DEFAULT_PRIORITY);
	iWidth.set(0, DEFAULT_PRIORITY);
	iHeight.set(0, DEFAULT_PRIORITY);
	iMinWidth = 0;
	iMinHeight = 0;
	bFocused = false;
	bEnabled = true;
	bVisible.set(true, DEFAULT_PRIORITY);
	iOpacity.set(SDL_ALPHA_OPAQUE, DEFAULT_PRIORITY);
	bModal = false;
	bOverlap = false;
	bNeedsRepaint = false;
	bDestroyed = false;
	bDestroying = false;
	bCreated = false;
	bMouseOver = false;
	bMouseDown = false;
	sName = name;
	bmpBuffer = NULL;
	cParent = parent;
	cLayoutCSS = NULL;
	if (parent)
		parent->Add(this); // Add this widget to the parent (if any)

	// Clear the events
	CLEAR_EVENT(OnMouseEnter);
	CLEAR_EVENT(OnMouseLeave);
	CLEAR_EVENT(OnMouseMove);
	CLEAR_EVENT(OnMouseUp);
	CLEAR_EVENT(OnMouseDown);
	CLEAR_EVENT(OnWheelUp);
	CLEAR_EVENT(OnWheelDown);
	CLEAR_EVENT(OnKeyDown);
	CLEAR_EVENT(OnKeyUp);
}

///////////////////
// Resize the widget
void CWidget::Resize(int x, int y, int w, int h)
{
	const int old_x = getX();
	const int old_y = getY();
	const int old_w = getWidth();
	const int old_h = getHeight();

	w = MAX(w, iMinWidth);
	h = MAX(h, iMinHeight);

	iX.set(x, HIGHEST_PRIORITY);
	iY.set(y, HIGHEST_PRIORITY);
	iWidth.set(w, HIGHEST_PRIORITY);
	iHeight.set(h, HIGHEST_PRIORITY);

	ReallocBuffer(w, h);
	Repaint();

	// Notify the parent about this resize
	if (cParent)
		cParent->DoChildResize(this, MakeRect(old_x, old_y, old_w, old_h), getRect());
}

////////////////////
// Resizes the internal buffer
void CWidget::ReallocBuffer(int w, int h)
{
	if (bmpBuffer.get()) {
		if (bmpBuffer->w == w && bmpBuffer->h == h)
			return; // No need to resize
		else
			bmpBuffer = NULL;
	}

	bmpBuffer = gfxCreateSurfaceAlpha(w, h);
	bNeedsRepaint = true;
}


///////////////////
// Returns true if a point is inside this widget
// Coordinates are relative to this widget
bool CWidget::RelInBox(int x, int y)
{
	return (x >= 0 && x < iWidth)  && (y >= 0 && y < iHeight);
}

///////////////////
// Returns true if a point is inside this widget
bool CWidget::InBox(int x, int y)
{
	return (x >= iX && x < iX + iWidth)  && (y >= iY && y < iY + iHeight);
}

/////////////////////
// Draw the widget
void CWidget::Draw(SDL_Surface *bmpDest, int drawX, int drawY)
{
	if (bmpBuffer.get())
		SetPerSurfaceAlpha(bmpBuffer.get(), iOpacity); // Set the opacity
	else
		return;

	SmartPointer<SDL_Surface> draw = bmpBuffer;
	int sx = 0;
	int sy = 0;

	std::list<CWidgetEffect *>::iterator it;

	// Apply create effects
	for (it = tCreateEffects.begin(); it != tCreateEffects.end(); it++)  {
		if ((*it)->isRunning())
			draw = (*it)->Apply(draw, sx, sy);
	}

	// Apply persistent effects
	for (it = tPersistentEffects.begin(); it != tPersistentEffects.end(); it++)  {
		if ((*it)->isRunning())
			draw = (*it)->Apply(draw, sx, sy);
	}

	// Apply destroy effects
	for (it = tDestroyEffects.begin(); it != tDestroyEffects.end(); it++)  {
		if ((*it)->isRunning())
			draw = (*it)->Apply(draw, sx, sy);
	}

	// Draw
	if (draw.get())
		DrawImageAdv(bmpDest, draw, sx, sy, drawX, drawY, draw->w, draw->h);
	else
		printf("CWidget::Draw: Warning: nothing to draw!\n");

	SetPerSurfaceAlpha(bmpBuffer.get(), 255); // Reset the alpha to full opacity
}

///////////////////////
// Destroy the widget
void CWidget::Destroy()
{
	if (!bDestroyed && !bDestroying)  { // Avoid multiple destroying
		DoDestroy(false);
		Repaint();
		bDestroying = false;
	}
}

///////////////////////
// Repaint the widget
void CWidget::Repaint()
{
	// Send the repaint message
	if (!bNeedsRepaint)  { // Send only one repaint per frame (more don't make sense anyway)
		SendSDLUserEvent(&OnNeedRepaint, EventData(this));
		bNeedsRepaint = true;

		// Notify the parent
		if (cParent)
			cParent->DoChildNeedsRepaint(this);
	}
}

///////////////////
// Set the parent
void CWidget::setParent(CContainerWidget *l)
{
	if (cParent != NULL)
		cParent->DeregisterWidget(this);
	cParent = l;

	if (cParent)
		cParent->Add(this);
}

////////////////////
// Returns a selector representing this widget
CSSParser::Selector CWidget::getCSSSelector()
{
	std::string psclass = "";
	if (bMouseOver)
		psclass = "hover";
	if (bMouseDown)
		psclass = "down";
	if (!bEnabled)
		psclass = "disabled";
	return CSSParser::Selector(getTagName(), sName == STATIC ? "" : sName, sCSSClass, psclass, "");
}

//////////////////
// Returns the current CSS context
CSSParser::Selector::Context CWidget::getMyContext()
{
	CSSParser::Selector::Context result;
	CContainerWidget *p = cParent;
	while (p)  {
		result.addFrontSelector(p->getCSSSelector());
		p = p->cParent;
	}

	return result;
}

//////////////////
// Reads and applies a CSS style
void CWidget::ApplyCSS(CSSParser& css)
{
	cLayoutCSS = &css;
	const CSSParser::Selector& us = getCSSSelector();
	ApplySelector(css.getStyleForElement(us.getElement(), us.getID(), us.getClass(), us.getPseudoClass(), us.getContext()));
}

//////////////////
// Applies the given CSS selector
void CWidget::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	for (std::list<CSSParser::Attribute>::const_iterator it = sel.getAttributes().begin(); it != sel.getAttributes().end(); it++)  {
		// Widget's default CSS
		if (it->getName() == prefix + "x")
			iX.set(it->getFirstValue().getInteger(), it->getPriority());
		else if (it->getName() == prefix + "y")
			iY.set(it->getFirstValue().getInteger(), it->getPriority());
		else if (it->getName() == prefix + "width")
			iWidth.set(it->getFirstValue().getInteger(), it->getPriority());
		else if (it->getName() == prefix + "height")
			iHeight.set(it->getFirstValue().getInteger(), it->getPriority());
		else if (it->getName() == prefix + "opacity")
			iOpacity.set(it->getFirstValue().getInteger(255), it->getPriority());
		else if (it->getName() == prefix + "visibility" || it->getName() == prefix + "display")
			bVisible.set((it->getFirstValue().getString() != "hidden" && it->getFirstValue().getString() != "none"), it->getPriority());
	}

	ReallocBuffer(iWidth, iHeight);
	Repaint();
}

////////////////////
// Apply a XML node to the widget
void CWidget::ApplyTag(xmlNodePtr node)
{
	if (xmlPropExists(node, "x"))
		iX.set(xmlGetInt(node, "x", iX), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "y"))
		iY.set(xmlGetInt(node, "y", iY), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "width"))
		iWidth.set(xmlGetInt(node, "width", iWidth), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "height"))
		iHeight.set(xmlGetInt(node, "height", iHeight), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "opacity"))
		iOpacity.set(xmlGetInt(node, "opacity", iOpacity), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "visible"))
		bVisible.set(xmlGetBool(node, "visible", bVisible), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "enabled"))
		bEnabled = xmlGetBool(node, "enabled", bEnabled);

	// CSS class
	sCSSClass = xmlGetString(node, "class");

	// Parse & apply the inline styles
	std::string inline_css = xmlGetString(node, "style");
	if (inline_css.size())  {
		CSSParser::Selector inline_sel;
		if (CSSParser().parseInSelector(inline_sel, inline_css, TAG_CSS_PRIORITY))  {
			inline_sel.setBaseURL(xmlGetBaseURL(node));
			ApplySelector(inline_sel);
		}
	}

	Repaint();
}

//////////////////
// Defines default Mouse Enter behavior of the widget
int CWidget::DoMouseEnter(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	bMouseOver = true;
	CALL_EVENT(OnMouseEnter, (this, x, y, dx, dy, modstate));
	if (cLayoutCSS)  {
		ApplyCSS(*cLayoutCSS);
		Repaint();
	}

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Leave behavior of the widget
int CWidget::DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	bMouseOver = false;
	CALL_EVENT(OnMouseLeave, (this, x, y, dx, dy, modstate));
	if (cLayoutCSS)  {
		ApplyCSS(*cLayoutCSS);
		Repaint();
	}

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Move behavior of the widget
int CWidget::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	CALL_EVENT(OnMouseMove, (this, x, y, dx, dy, down, button, modstate));
	bMouseOver = RelInBox(x, y);

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Up behavior of the widget
int CWidget::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bMouseDown = false;
	CALL_EVENT(OnMouseUp, (this, x, y, dx, dy, button, modstate));
	if (cLayoutCSS)  {
		ApplyCSS(*cLayoutCSS);
		Repaint();
	}

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Down behavior of the widget
int CWidget::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bMouseDown = true;
	CALL_EVENT(OnMouseDown, (this, x, y, dx, dy, button, modstate));
	if (cLayoutCSS)  {
		ApplyCSS(*cLayoutCSS);
		Repaint();
	}

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Wheel Up behavior of the widget
int CWidget::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	CALL_EVENT(OnWheelUp, (this, x, y, dx, dy, modstate));

	return WID_PROCESSED;
}

//////////////////
// Defines default Mouse Wheel Down behavior of the widget
int CWidget::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	CALL_EVENT(OnWheelDown, (this, x, y, dx, dy, modstate));

	return WID_PROCESSED;
}

//////////////////
// Defines default Key Down behavior of the widget
int CWidget::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	CALL_EVENT(OnKeyDown, (this, c, keysym, modstate));

	return WID_NOT_PROCESSED;
}

//////////////////
// Defines default Key Up behavior of the widget
int CWidget::DoKeyUp(UnicodeChar c, int keysym,  const ModifiersState& modstate)
{
	CALL_EVENT(OnKeyUp, (this, c, keysym, modstate));

	return WID_NOT_PROCESSED;
}

////////////////////
// Focus event
int CWidget::DoFocus(CWidget *prev_focused)
{
	return WID_PROCESSED;
}

///////////////////
// Focus has been taken off this widget
int CWidget::DoLoseFocus(CWidget *new_focused)
{
	return WID_PROCESSED;
}

///////////////////
// Defines the default Destroy behavior of the widget
int CWidget::DoDestroy(bool immediate)
{
	// If immediate close, omit the destroy effects
	if (immediate || tDestroyEffects.size() == 0)  {
		bDestroyed = true;
		if (cParent)
			cParent->DoChildDestroyed(this);
		return WID_NOT_PROCESSED;
	}

	// Run the destroy effects
	for (std::list<CWidgetEffect *>::iterator it = tDestroyEffects.begin(); it != tDestroyEffects.end(); it++)
		(*it)->Run();

	return WID_PROCESSED;
}

//////////////////
// Effect finished event
void CWidget::DoEffectFinished(CWidgetEffect *e)
{
	// Check if it's a destroy effect
	for (std::list<CWidgetEffect *>::iterator it = tDestroyEffects.begin(); it != tDestroyEffects.end(); it++)  {
		if (e == (*it))  {
			tDestroyEffects.erase(it);

			// If there are no destroy effects left and we are in "destroying" state, set the "destroyed" flag
			bDestroyed = bDestroying && tDestroyEffects.size() == 0;
			if (bDestroyed && cParent)
				cParent->DoChildDestroyed(this);

			return;
		}
	}

	// Create effect?
	for (std::list<CWidgetEffect *>::iterator it = tCreateEffects.begin(); it != tCreateEffects.end(); it++)  {
		if (e == (*it))  {
			tCreateEffects.erase(it);

			return;
		}
	}

	// Persistent effect?
	for (std::list<CWidgetEffect *>::iterator it = tPersistentEffects.begin(); it != tPersistentEffects.end(); it++)  {
		if (e == (*it))  {
			tPersistentEffects.erase(it);

			return;
		}
	}

}

////////////////////
// Repaint event
void CWidget::DoRepaint()
{
	// The buffer has to be cleared before
	if (bmpBuffer.get())
		FillSurfaceTransparent(bmpBuffer.get());
	bNeedsRepaint = false;
}

///////////////////
// Defines the default Create behavior of the widget
int CWidget::DoCreate()
{
	bCreated = true;

	// Run the create effects
	for (std::list<CWidgetEffect *>::iterator it = tCreateEffects.begin(); it != tCreateEffects.end(); it++)
		(*it)->Run();

	// Run the permanent effects
	for (std::list<CWidgetEffect *>::iterator it = tPersistentEffects.begin(); it != tPersistentEffects.end(); it++)
		(*it)->Run();

	// Reallocate the buffer
	ReallocBuffer(getWidth(), getHeight());

	// Repaint
	Repaint();

	return WID_PROCESSED;
}

////////////////////
// Parent resize event
int CWidget::DoParentResize(int& new_parent_w, int& new_parent_h)
{
	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
