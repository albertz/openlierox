/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Menu
// Created 28/6/03
// Jason Boettcher


#include <assert.h>
#include "LieroX.h"
#include "debug.h"
#include "GfxPrimitives.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "SkinnedGUI/CMenu.h"
#include "XMLutils.h"


namespace SkinnedGUI {

////////////////////
// Draw the menu item
void CMenuItem::Draw(SDL_Surface *bmpDest, const SDL_Rect &r)
{
	CItemStyle *style = getCurrentStyle();

	// Background
	style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);

	int cur_x = style->cBorder.getLeftW() + ITEM_SPACING;
	int bg_w = cParent->bmpCheckBg.get().get() ? cParent->bmpCheckBg->w : 0;
	int check_w = cParent->bmpCheck.get().get() ? cParent->bmpCheck->w : 0;
	int img_w = tImage.get() ? tImage->w : 0;

	// Check & image
	if (bCheckable && bChecked)  {
		// Check background
		if (cParent->bmpCheckBg.get().get())  {
			DrawImageAdv(bmpDest, cParent->bmpCheckBg, 0, 0, ITEM_SPACING, r.y + (r.h - cParent->bmpCheckBg->h)/2,
				MIN(r.w - ITEM_SPACING, cParent->bmpCheckBg->w), MIN(r.h, cParent->bmpCheckBg->h));
			bg_w = cParent->bmpCheckBg->w;
		}

		// Image/check
		if (cParent->bmpCheck.get().get() && !tImage.get()) {
			DrawImageAdv(bmpDest, cParent->bmpCheck,  0, 0, ITEM_SPACING, r.y + (r.h - cParent->bmpCheck->h)/2,
				MIN(r.w - ITEM_SPACING, cParent->bmpCheck->w), MIN(r.h, cParent->bmpCheck->h));

			check_w = cParent->bmpCheck->w;
		}
	}

	// Image
	DrawItemImage(bmpDest, r);

	cur_x += MAX(img_w, MAX(check_w, bg_w)) + IMAGE_SPACING;

	// Text
	SDL_Rect tr = { r.x + cur_x, r.y, r.w - cur_x, r.h};
	DrawGameText(bmpDest, sName, style->cFont, CTextProperties(&tr, algLeft, (TextVAlignment)(iVAlign.get())));

	// Border
	style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

///////////////////
// Repaint the parent menu
void CMenuItem::RepaintParent()
{
	if (cParent)
		cParent->Repaint();
}

/////////////////////
// Get the item width
int CMenuItem::getWidth()
{
	CItemStyle *style = getCurrentStyle();
	int w = 2 * ITEM_SPACING + GetTextWidth(style->cFont, sName) + IMAGE_SPACING;

	// HINT: we calculate with the check width even for non-checkable items because the menu can contain
	// checkable items and it would break the alignment
	if (tImage.get())  {
		if (cParent->bmpCheckBg.get().get())
			w += MAX(cParent->bmpCheckBg->w, tImage->w);
		else
			w += tImage->w;
	} else {
		if (cParent->bmpCheck.get().get())  {
			if (cParent->bmpCheckBg.get().get())
				w += MAX(cParent->bmpCheckBg->w, cParent->bmpCheck->w);
			else
				w += cParent->bmpCheck->w;
		}
	}

	return w;
}

////////////////////////
// Get item height
int CMenuItem::getHeight()
{
	CItemStyle *style = getCurrentStyle();
	int bg_h = cParent->bmpCheckBg.get().get() ? cParent->bmpCheckBg->h : 0;
	int check_h = cParent->bmpCheck.get().get() ? cParent->bmpCheck->h : 0;
	int img_h = tImage.get() ? tImage->h : 0;

	return MAX(GetTextHeight(style->cFont, sName), MAX(bg_h, MAX(check_h, img_h)));
}

//////////////////////
// Get the current style
CItem::CItemStyle *CMenuItem::getCurrentStyle()
{
	if (!bEnabled)
		return &cDisabledStyle;
	else
		return CItem::getCurrentStyle();
}

//////////////////////
// Apply a tag to the menu item
void CMenuItem::ApplyTag(xmlNodePtr node)
{
	CItem::ApplyTag(node);
	bCheckable = xmlGetBool(node, "checkable");
	bChecked = xmlGetBool(node, "checked");
	if (xmlPropExists(node, "checked")) // If there's the checked property, make the item checkable
		bCheckable = true;
	bEnabled = xmlGetBool(node, "enabled", true);
}

///////////////////
// CMenu constructor
CMenu::CMenu(const std::string& name, CContainerWidget *parent, int x, int y) : CWidget(name, parent)
{
	bModal = true;
	bOverlap = true;
	Resize(x, y, 0, 0);
	setSelected(-1);

	CLEAR_EVENT(OnClose);
	CLEAR_EVENT(OnItemClick);
}

CMenu::CMenu(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	bModal = true;
	bOverlap = true;
	setSelected(-1);

	CLEAR_EVENT(OnClose);
	CLEAR_EVENT(OnItemClick);	
}

//////////////////
// Destructor
CMenu::~CMenu()
{
	for (std::list<CMenuItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)
		delete (*it);
	tItems.clear();
}

////////////////
// Apply the given selector
void CMenu::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "check-image")
			bmpCheck.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
		else if (it->getName() == prefix + "check-background-image")
			bmpCheckBg.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
	}
}

//////////////////
// Load style for an item
void CMenu::LoadItemStyle(CMenuItem::CItemStyle& style, const CSSParser& css, const std::string& id, const std::string& cl, const std::string& pscl)
{
	const CSSParser::Selector& sel = css.getStyleForElement("item", id, cl, pscl, getMyContext());
	style.ApplySelector(sel);
}

//////////////////////
// Apply the given CSS
void CMenu::ApplyCSS(CSSParser& css)
{
	CWidget::ApplyCSS(css);
	LoadItemStyle(cNormalItem, css, "", "", "");
	LoadItemStyle(cActiveItem, css, "", "", "hover");
	LoadItemStyle(cClickedItem, css, "", "", "down");
	LoadItemStyle(cDisabledItem, css, "", "", "disabled");

	// Re-style the items if necessary
	if (&css != cLayoutCSS)  {
		for (std::list<CMenuItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)
			StyleItem(**it);
	}
}

////////////////////
// Apply the given tag
void CMenu::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	// Menu properties
	if (xmlPropExists(node, "checkimage"))
		bmpCheck.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), xmlGetString(node, "checkimage"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "checkbackground"))
		bmpCheckBg.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), xmlGetString(node, "checkbackground"))), TAG_ATTR_PRIORITY);

	// Items
	xmlNodePtr child = node->children;
	while (child)  {
		if (!xmlStrcasecmp(node->name, (const xmlChar *)"menuitem"))  {
			CMenuItem newit(this, "");
			newit.ApplyTag(node);
			AddItem(newit);
		}
	}
}

////////////////////
// Style the item according to the current CSS
void CMenu::StyleItem(CMenuItem& it)
{
	if ((it.getCSSClass().size() == 0 && it.getCSSID().size() == 0) || (cLayoutCSS == NULL))  {
		it.cNormalStyle = cNormalItem;
		it.cActiveStyle = cActiveItem;
		it.cClickedStyle = cClickedItem;
		it.cDisabledStyle = cDisabledItem;
	} else {
		LoadItemStyle(it.cNormalStyle, *cLayoutCSS, "", "", "");
		LoadItemStyle(it.cActiveStyle, *cLayoutCSS, "", "", "hover");
		LoadItemStyle(it.cClickedStyle, *cLayoutCSS, "", "", "down");
		LoadItemStyle(it.cDisabledStyle, *cLayoutCSS, "", "", "disabled");
	}	
}

/////////////////
// Set the selected item based on its index
void CMenu::setSelected(int index)
{
	// Bounds check
	if (index >= (int)tItems.size())
		return;

	// Remove focus from the old selected item, if any
	if (tSelected)  {
		tSelected->setActive(false);
		tSelected->setDown(false);
	}

	// Check
	if (index < 0 || tItems.empty())  {
		tSelected = NULL;
		iSelected = -1;

		return;
	}

	// Set the item
	std::list<CMenuItem *>::iterator it = tItems.begin();
	std::advance(it, index);
	tSelected = *it;
	iSelected = index;

	// Tell the item
	tSelected->setActive(true);
}

///////////////////
// Add an item to the menu
void CMenu::AddItem(const CMenuItem& item)
{
	// Add the item to the list
	CMenuItem *newit = new CMenuItem(this, "");
	*newit = item;
	newit->setParent(this); // Safety

	// Style
	StyleItem(*newit);

	tItems.push_back(newit);

	// Adjust width
	Resize(getX(), getY(), MAX(getWidth(), newit->getWidth() + cBorder.getLeftW() + cBorder.getRightW()), getHeight() + newit->getHeight());
}

///////////////////
// Get an item based on its index
CMenuItem *CMenu::getItem(int index)
{
	// Check bounds
	if (index < 0 || index >= (int)tItems.size())
		return NULL;

	// Find the right item
	std::list<CMenuItem *>::iterator it = tItems.begin();
	std::advance(it, index);
	return (*it);
}

//////////////////
// Repaint event
void CMenu::DoRepaint()
{
	CHECK_BUFFER;

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Items
	int cur_y = cBorder.getTopW();
	for (std::list<CMenuItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)  {
		(*it)->Draw(bmpBuffer.get(), MakeRect(cBorder.getLeftW(), cur_y, getWidth() - cBorder.getLeftW() - cBorder.getRightW(), (*it)->getHeight()));
		cur_y += (*it)->getHeight();
	}

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}


///////////////////
// Move move event
int CMenu::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	if(!RelInBox(x, y))  {
		setSelected(-1);
		CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
        return WID_PROCESSED;
	}

	// Go through the items and check if some is selected
	int cur_y = cBorder.getTopW();
    std::list<CMenuItem *>::iterator it = tItems.begin();
	for(int i = 0; it != tItems.end(); it++, i++) {

		// Selected?
		if( y > cur_y && y < cur_y + (*it)->getHeight()) {
			setSelected(i);
			(*it)->setDown(down);
            break;
        }

		cur_y += (*it)->getHeight();
    }

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
    return WID_PROCESSED;
}


///////////////////
// Mouse Up event
int CMenu::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
    // Close?
	if(!RelInBox(x, y))  {
		CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
		CALL_EVENT(OnClose, (this));
		Destroy();
        return WID_PROCESSED;
	}

	// Check if any of the items has been clicked
	int cur_y = cBorder.getTopW();
    std::list<CMenuItem *>::iterator it = tItems.begin();
	for(; it != tItems.end(); it++) {

		// In box?
		if( y >= cur_y && y < cur_y + (*it)->getHeight() )  {

			// Ignore on disabled items
			if (!(*it)->isEnabled())
				break;

			if ((*it)->isCheckable())
				(*it)->setChecked(!(*it)->isChecked());

			// Callback
			CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
			CALL_EVENT(OnItemClick, (this, *it));
			CALL_EVENT(OnClose, (this));

			Destroy();
			return WID_PROCESSED;
		}

		cur_y += (*it)->getHeight();
    }

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
    return WID_PROCESSED;
}


///////////////////
// Mouse down event
int CMenu::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);

    // Close
	if(!RelInBox(x, y))  {
		setSelected(-1);
		Destroy();
		CALL_EVENT(OnClose, (this));
        return WID_PROCESSED;
	}

    return WID_PROCESSED;
}

////////////////////
// Key down event
int CMenu::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	switch (keysym)  {
		// Down arrow
		case SDLK_DOWN:
			if (iSelected == -1)
				setSelected(0);
			else
				setSelected((iSelected + 1) % tItems.size());
		break;

		// Up arrow
		case SDLK_UP:
			if (iSelected == 0)
				setSelected(tItems.size() - 1);
			else
				setSelected(iSelected - 1);
		break;

		// Escape
		case SDLK_ESCAPE:
			// Close
			setSelected(-1);
			Destroy();
			CALL_EVENT(OnClose, (this));
		break;

		// Enter
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (iSelected >= 0)  {
				CALL_EVENT(OnItemClick, (this, tSelected));
				CALL_EVENT(OnClose, (this));

				Destroy();
			}
		break;
	}

	CWidget::DoKeyDown(c, keysym, modstate);
	return WID_NOT_PROCESSED;
}

}; // namespace SkinnedGUI
