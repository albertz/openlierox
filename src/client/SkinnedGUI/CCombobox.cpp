/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Combo box
// Created 3/9/02
// Jason Boettcher


#include <assert.h>
#include <algorithm>

#include "LieroX.h"

#include "SkinnedGUI/CCombobox.h"
#include "SkinnedGUI/CListview.h"
#include "GuiPrimitives.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "FindFile.h"
#include "XMLutils.h"
#include "CodeAttributes.h"


namespace SkinnedGUI {

#define ARROW_W 7
#define ARROW_H 4

//
// Item class
//

////////////////
// Draw the combobox item
void CComboItem::Draw(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	CItemStyle *style = getCurrentStyle();

	// Background
	style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);

	// Image
	DrawItemImage(bmpDest, r);

	// Text
	DrawItemText(bmpDest, r);

	// Border
	style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

////////////////////
// Repaint the parent
void CComboItem::RepaintParent()
{
	if (cParent)  {
		cParent->Repaint();
		if (cParent->getItemList())
			cParent->getItemList()->Repaint();
	}
}

//
// Combobox item list
//

////////////////
// Constructor
CComboItemList::CComboItemList(CCombobox *parent) : CContainerWidget("_CitemLst_" + parent->getName(), parent->getParent())
{
	iDisplayCount = 6; // Default, TODO: make it customizable?
	cCombo = parent;
	cScrollbar = new CScrollbar(parent->getName() + "_scroll", this, scrVertical);
	cActiveItem = cCombo->tItems.end();
	iActiveItem = -1;
	sCSSClass = cCombo->getCSSClass();
	cBackground.setColor(Color(255, 255, 255), DEFAULT_PRIORITY); // Default background color is black

	// A 1px black border as a default value
	CBorder::BorderLineSettings sett;
	sett.clDark.set(Color(0, 0, 0), DEFAULT_PRIORITY);
	sett.clLight.set(Color(0, 0, 0), DEFAULT_PRIORITY);
	sett.iThickness.set(1, DEFAULT_PRIORITY);
	cBorder.BorderBottom = cBorder.BorderLeft = cBorder.BorderRight = cBorder.BorderTop = sett;
}

CComboItemList::~CComboItemList()
{
	delete cScrollbar;
}

/////////////////////
// Adjusts the scrollbar according to the selected item
void CComboItemList::Readjust()
{
	// Resize if needed
	int item_height = cCombo->getItemHeight();

	if (item_height)  {
		if ((int)cCombo->tItems.size() < iDisplayCount)  {
			Resize(getX(), getY(), getWidth(), item_height * cCombo->tItems.size() +  + cBorder.getTopW() + cBorder.getBottomW());
			cScrollbar->setVisible(false);
			cScrollbar->setValue(0);
		} else {
			Resize(getX(), getY(), getWidth(), item_height * iDisplayCount + cBorder.getTopW() + cBorder.getBottomW());
			cScrollbar->setVisible(true);
			cScrollbar->setMax((int)cCombo->tItems.size());
			cScrollbar->setItemsperbox(iDisplayCount);
		}
	} else {
		cScrollbar->setVisible(false);
	}

	Resize(cCombo->getX(), cCombo->getY() + cCombo->getHeight(), getWidth() == 0 ? cCombo->getWidth() : getWidth(), getHeight() == 0 ? 20 : getHeight());

	// Adjust the scrollbar
	if (cScrollbar->getVisible() && cScrollbar->isCreated())
		if (cScrollbar->getValue() + cScrollbar->getItemsperbox() <= cCombo->iSelected)
			cScrollbar->setValue(cCombo->iSelected);

	cScrollbar->Resize(getWidth() - cScrollbar->getWidth() - cBorder.getLeftW(), cBorder.getTopW(), cScrollbar->getWidth(), MAX(1, getHeight() - cBorder.getTopW() - cBorder.getBottomW()));
}

//////////////////
// Applies the given CSS selector
void CComboItemList::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	CContainerWidget::ApplySelector(sel, prefix + "list-");
	cBorder.ApplySelector(sel, prefix + "list-");
	cBackground.ApplySelector(sel, prefix + "list-");
}

////////////////////////
// Sets the currently selected item
void CComboItemList::setActiveItem(const std::list<CComboItem>::iterator& it, int index, bool down)
{
	iActiveItem = index;

	if (cActiveItem != cCombo->tItems.end())  {
		cActiveItem->setActive(false);
		cActiveItem->setDown(false);
	}

	// Repaint only if it's necessary
	if (cActiveItem != it)
		Repaint();

	cActiveItem = it;
	if (cActiveItem != cCombo->tItems.end())  {
		cActiveItem->setActive(true);
		cActiveItem->setDown(down);
	}

	// Readjust the scrollbar
	if (cScrollbar->getVisible())
		if (index >= 0 && index < (int)cCombo->tItems.size())  {
			if (index >= cScrollbar->getValue() + cScrollbar->getItemsperbox())
				cScrollbar->setValue(MAX(0, index + 2 - cScrollbar->getItemsperbox()));
			else if (index < cScrollbar->getValue())
				cScrollbar->setValue(index);
		}

}

///////////////////////
// Returns an item based on its y coordinate
int CComboItemList::getItemByCoord(int y)
{
	return MAX(0, cScrollbar->getValue() + (y - cBorder.getTopW()) / cCombo->getItemHeight());
}

///////////////////
// Draw the item list
void CComboItemList::DoRepaint()
{
	CHECK_BUFFER;

	CContainerWidget::DoRepaint();

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	int client_width = getWidth() - cBorder.getLeftW() - cBorder.getRightW() - (cScrollbar->getVisible() ? cScrollbar->getWidth() : 0);

	// Items
	int index = 0;
	int count = 0;
	int y = cBorder.getTopW();
	int item_height = cCombo->getItemHeight();
	int max_y = getHeight() - cBorder.getTopW();
	for(std::list<CComboItem>::iterator item = cCombo->tItems.begin(); item != cCombo->tItems.end(); item++, count++, index++) {
		if(count < cScrollbar->getValue())
			continue;

		item->Draw(bmpBuffer.get(), MakeRect(cBorder.getLeftW(), y, client_width, item_height));

		y += item_height;
		if(y + item_height > max_y)
			break;
	}

	if (cScrollbar->needsRepaint())
		cScrollbar->DoRepaint();

	// Scrollbar
	if (cScrollbar->getVisible())
		cScrollbar->Draw(bmpBuffer.get(), getWidth() - cBorder.getRightW() - cScrollbar->getWidth(), cBorder.getTopW());

	// Border
	cBorder.Draw(bmpBuffer.get(), 0, 0, getWidth(), getHeight());
}

///////////////////
// Mouse over event
int CComboItemList::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	if(cScrollbar->getVisible() && cScrollbar->getGrabbed())
		cScrollbar->DoMouseMove(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, down, button, modstate);

	if (RelInBox(x, y))  {
		if (!cScrollbar->getVisible() || (x < getWidth() - cBorder.getRightW() - cScrollbar->getWidth()))  {
			if (cCombo->getItemsCount() > 0 && cCombo->getItemHeight() > 0)  {
				int selected = getItemByCoord(y);

				// Update the selected item
				std::list<CComboItem>::iterator it = cCombo->tItems.begin();
				SafeAdvance(it, selected, cCombo->tItems.end());
				setActiveItem(it, selected, down);

			}
		}
	}

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);

	return WID_PROCESSED;
}

/////////////////
// Create event
int CComboItemList::DoCreate()
{
	CContainerWidget::DoCreate();
	cScrollbar->DoCreate();
	Readjust();

	return WID_PROCESSED;
}


///////////////////
// Mouse down event
int CComboItemList::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if(cScrollbar->getVisible() && cScrollbar->InBox(x, y)) {
		cScrollbar->DoMouseDown(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, button, modstate);
		CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	}

	if(RelInBox(x, y)) {
		if (cCombo->getItemsCount() > 0 && cCombo->getItemHeight() > 0)  {
			int selected = getItemByCoord(y);

			// Update the selected item
			std::list<CComboItem>::iterator it = cCombo->tItems.begin();
			SafeAdvance(it, selected, cCombo->tItems.end());
			setActiveItem(it, selected, true);
		}
	}

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);

	return WID_PROCESSED;
}

///////////////////
// Mouse up event
int CComboItemList::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if(cScrollbar->InBox(x, y) && cScrollbar->getVisible()) {
		cScrollbar->DoMouseUp(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, button, modstate);
		CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	}

	if (RelInBox(x, y))  {
		Destroy(); // Close the floating list window

		// Set the current item
		if (cCombo->getItemsCount() > 0 && cCombo->getItemHeight() > 0)  {
			int selected = getItemByCoord(y);

			std::list<CComboItem>::iterator it = cCombo->tItems.begin();
			SafeAdvance(it, selected, cCombo->tItems.end());
			if (it != cCombo->tItems.end())  {
				it->setActive(false);
				it->setDown(false);
				cCombo->setCurItem(&*it);
			}

			// Unselect any previously selected item
			setActiveItem(cCombo->tItems.end(), -1, false);
		}
	}

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

/////////////////////
// Focus lost event
int CComboItemList::DoLoseFocus(CWidget *new_focused)
{
	CWidget::DoLoseFocus(new_focused);

	Destroy();

	return WID_PROCESSED;
}

///////////////////
// Mouse wheel down event
int CComboItemList::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if(cScrollbar->getVisible())
		cScrollbar->DoMouseWheelDown(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, modstate);

	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse wheel up event
int CComboItemList::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if(cScrollbar->getVisible())
		cScrollbar->DoMouseWheelUp(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, modstate);

	CWidget::DoMouseWheelUp(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

////////////////////
// Scrollbar repaint
int CComboItemList::DoChildNeedsRepaint(CWidget *child)
{
	CContainerWidget::DoChildNeedsRepaint(child);
	Repaint();
	return WID_PROCESSED;
}

//////////////////
// Key down event
int CComboItemList::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	int index = cCombo->findItem(c);
	if(index >= 0) {
		cCombo->setCurItem(index);

		CContainerWidget::DoKeyDown(c, keysym, modstate);
		return WID_NOT_PROCESSED;
	}


	// Handle key up/down
	switch (keysym)  {
	case SDLK_DOWN:
		if (iActiveItem >= 0 && iActiveItem < (int)cCombo->tItems.size() - 1)  {
			std::list<CComboItem>::iterator tmp = cActiveItem;
			tmp++;
			setActiveItem(tmp, MAX(0, iActiveItem + 1), false);
		} else if (iActiveItem == -1 && cCombo->tItems.size())  {
			setActiveItem(cCombo->tItems.begin(), 0, false);
		}
		break;

	case SDLK_UP:
		if (iActiveItem > 0 && cCombo->tItems.size() > 0)  {
			std::list<CComboItem>::iterator tmp = cActiveItem;
			tmp--;
			setActiveItem(tmp, iActiveItem - 1, false);
		} else if (iActiveItem == -1 && cCombo->tItems.size())  {
			std::list<CComboItem>::iterator tmp = cCombo->tItems.end();
			tmp--;
			setActiveItem(tmp, (int)cCombo->tItems.size() - 1, false);
		}
		break;

	case SDLK_RETURN:
		if (cActiveItem != cCombo->tItems.end())
			cCombo->setCurItem(&*cActiveItem);
		Destroy();

	case SDLK_ESCAPE:
		Destroy();
	break;
	}

	CContainerWidget::DoKeyDown(c, keysym, modstate);

	return WID_NOT_PROCESSED;
}

////////////////
// Destroy event
int CComboItemList::DoDestroy(bool immediate)
{
	CContainerWidget::DoDestroy(immediate);
	cCombo->OnItemListClose();
	cScrollbar->DoDestroy(immediate);
	setActiveItem(cCombo->tItems.end(), -1, false); // Unselect any item

	return WID_PROCESSED;
}

//
// Combobox
//

///////////////////
// Draw the combo box
void CCombobox::DoRepaint()
{
	CHECK_BUFFER;

	CContainerWidget::DoRepaint();

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Active item
	if (cSelectedItem)  {
		int x = cBorder.getLeftW() + ITEM_SPACING;

		// Image
		if (cSelectedItem->getImage().get())  {
			DrawImageAdv(bmpBuffer.get(), cSelectedItem->getImage(), 0, 0, x,
				MAX(cBorder.getTopW(), cBorder.getTopW() + (getHeight() - cSelectedItem->getImage()->h)/2),
				MIN(cSelectedItem->getImage()->w, getWidth() - bmpExpand->w/3 - cBorder.getLeftW() - cBorder.getRightW()),
				MIN(cSelectedItem->getImage()->h, getHeight() - cBorder.getTopW() - cBorder.getBottomW()));
			x += cSelectedItem->getImage()->w + IMAGE_SPACING;
		}

		// Text
		int expand_w = bmpExpand.get().get() ? bmpExpand->w/3 : 0;
		SDL_Rect r = {x, cBorder.getTopW(), getWidth() - x - expand_w - cBorder.getRightW(),
			getHeight() - cBorder.getTopW() - cBorder.getBottomW()};

		cText.tFontRect = &r;
		DrawGameText(bmpBuffer.get(), cSelectedItem->getName(), cFont, cText);
	}

	// Button
	if (bmpExpand.get().get())  {
		int sx = 0;
		if (bMouseOver)
			sx = bmpExpand->w / 3;
		if (bMouseDown || cItemList)
			sx = bmpExpand->w * 2 / 3;
		DrawImageAdv(bmpBuffer.get(), bmpExpand.get(), sx, 0, getWidth() - cBorder.getRightW() - bmpExpand->w/3, cBorder.getTopW(), bmpExpand->w / 3, bmpExpand->h);
	} else {
		int w = getHeight() - cBorder.getTopW() - cBorder.getBottomW();
		DrawSimpleButton(bmpBuffer.get(), getWidth() - cBorder.getRightW() - w, cBorder.getTopW(), w, w, iExpandFace, iExpandLight, iExpandShadow, bMouseDown || cItemList);
		DrawSimpleArrow(bmpBuffer.get(), getWidth() - cBorder.getRightW() - (w + ARROW_W) / 2, cBorder.getTopW() + (w - ARROW_H)/2, ARROW_W, ARROW_H, ardDown, iExpandArrow);
	}

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

/////////////////////
// Makes sure the size is correct
void CCombobox::UpdateSize()
{
	if (!cSelectedItem)
		Resize(getX(), getY(), getWidth() == 0 ? 100 : getWidth(), getHeight() == 0 ? 16 : getHeight());
	else
		Resize(getX(), getY(), getWidth() == 0 ? cSelectedItem->getWidth() : getWidth(), getHeight() == 0 ? cSelectedItem->getHeight() : getHeight());
}

////////////////
// Create event
int CCombobox::DoCreate()
{
	UpdateSize();
	CWidget::DoCreate();

	return WID_PROCESSED;
}

//////////////////
// Applies the given CSS selector
void CCombobox::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "arrow-image")
			bmpExpand.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
		else if (it->getName() == prefix + "expand-light-color" || it->getName() == prefix + "expand-light-colour")
			iExpandLight.set(it->getFirstValue().getColor(iExpandLight), it->getPriority());
		else if (it->getName() == prefix + "expand-shadow-color" || it->getName() == prefix + "expand-shadow-colour")
			iExpandShadow.set(it->getFirstValue().getColor(iExpandShadow), it->getPriority());
		else if (it->getName() == prefix + "expand-face-color" || it->getName() == prefix + "expand-face-colour")
			iExpandFace.set(it->getFirstValue().getColor(iExpandFace), it->getPriority());
		else if (it->getName() == prefix + "expand-arrow-color" || it->getName() == prefix + "expand-arrow-colour")
			iExpandArrow.set(it->getFirstValue().getColor(iExpandArrow), it->getPriority());
	}
}

////////////////////
// Applies the rules from the given CSS
void CCombobox::ApplyCSS(CSSParser& css)
{
	CWidget::ApplyCSS(css);

	// Load the item styles
	LoadItemStyle(css, cNormalItem, "", "", "");
	LoadItemStyle(css, cActiveItem, "", "", "hover");
	LoadItemStyle(css, cClickedItem, "", "", "down");

	// Go through the items and apply the new style
	for (std::list<CComboItem>::iterator it = tItems.begin(); it != tItems.end(); it++)  {
		if (it->getCSSClass().size() == 0 && it->getCSSID().size() == 0 && !it->hasInlineStyle())  {
			it->cNormalStyle = cNormalItem;
			it->cActiveStyle = cActiveItem;
			it->cClickedStyle = cClickedItem;
		} else if (cLayoutCSS) {
			LoadItemStyle(*cLayoutCSS, it->cNormalStyle, it->getCSSID(), it->getCSSClass(), "");
			LoadItemStyle(*cLayoutCSS, it->cActiveStyle, it->getCSSID(), it->getCSSClass(), "hover");
			LoadItemStyle(*cLayoutCSS, it->cClickedStyle, it->getCSSID(), it->getCSSClass(), "down");
		}
	}
}

/////////////////////
// Loads an item style from the CSS
void CCombobox::LoadItemStyle(const CSSParser &css, CItem::CItemStyle &style, const std::string& id, const std::string& cl, const std::string &psclass)
{
	const CSSParser::Selector& sel = css.getStyleForElement("option", id, cl, psclass, getMyContext());
	style.ApplySelector(sel);
}

/////////////////////////
// Read the information from a xml node
void CCombobox::ApplyTag(xmlNodePtr node)
{
	CContainerWidget::ApplyTag(node);

	// Read the attributes
	std::string sort_dir = xmlGetString(node, "sort");
	if (stringcaseequal(sort_dir, "ascending"))
		iSortDirection = sort_Ascending;
	else if (stringcaseequal(sort_dir, "descending"))
		iSortDirection = sort_Descending;
	else if (stringcaseequal(sort_dir, "none"))
		iSortDirection = sort_None;

	std::string expand_path = xmlGetString(node, "arrow");
	if (expand_path.size())
		bmpExpand.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), expand_path), true), TAG_ATTR_PRIORITY);

	// Read items
	xmlNodePtr child = node->children;
	while (child)  {
		if (!xmlStrcasecmp(child->name, (const xmlChar *)"option"))  {
			// Create a new item
			CComboItem newit(this, "");
			newit.ApplyTag(child);
			AddItem(newit);
		}

		child = child->next;
	}
}








static INLINE int compare_items(const CComboItem& item1, const CComboItem& item2) {
	// Swap the two items?
	bool failed1,failed2;
	int nat_cmp1 = from_string<int>(item1.getName(), failed1);
	int nat_cmp2 = from_string<int>(item2.getName(), failed2);

	// First try, if we compare numbers
	if (!failed1 && !failed2)  {
		if(nat_cmp1 == nat_cmp2)
			return item1.getName().size() - item2.getName().size(); // because from_string("123456") == from_string("123456abcd")
		else
			return nat_cmp1 - nat_cmp2;
	// String comparison
	} else {
		return stringcasecmp(item1.getName(), item2.getName());
	}
}


static INLINE bool less_items(const CComboItem& item1, const CComboItem& item2)  {
	return compare_items(item1, item2) < 0;
}


static INLINE bool greater_items(const CComboItem& item1, const CComboItem& item2)  {
	return compare_items(item1, item2) > 0;
}

static INLINE bool equal_items(const CComboItem& item1, const CComboItem& item2) {
	return compare_items(item1, item2) == 0;
}

bool operator<(const CComboItem& item1, const CComboItem& item2) { return less_items(item1, item2); }
bool operator>(const CComboItem& item1, const CComboItem& item2) { return greater_items(item1, item2); };

//////////////////////
// Sorts te items in the combobox
// HINT: private
void CCombobox::Sort(bool ascending)
{
#if !defined(_MSC_VER) || _MSC_VER > 1200
	if (ascending)
		tItems.sort(less_items);
	else
		tItems.sort(greater_items);
#else
	// A very dirty workaround for msvc 6
	tItems.sort();
	if (!ascending)
		tItems.reverse();

#endif
}

///////////////////////
// Removes duplicate entries (based on sName)
// HINT: private
void CCombobox::Unique() {
	if (tItems.size() < 2)
		return;

	std::list<CComboItem>::iterator new_end;
	if (iSortDirection == sort_None)  {
		new_end = std::unique(tItems.begin(), tItems.end(), equal_items);
	} else {
		std::list<CComboItem>::iterator cur, next;
		next = new_end = cur = tItems.begin();
		next++;
		while (next != tItems.end())  {
			if (!equal_items(*cur, *next))  {
				*new_end = *cur;
				new_end++;
			}

			cur = next;
			next++;
		}
	}
	tItems.erase(new_end, tItems.end());

	if (cItemList)
		cItemList->Readjust();
}

////////////////////////
// Enables/disables duplicate removing
// Affects both existing and future items
void CCombobox::setUnique(bool _u)
{
	if (!bUnique && _u)
		Unique();
	bUnique = _u;
}

////////////////////////
// Enables/disables combobox sorting
// Affects both existing and future items
void CCombobox::setSorted(int sort_direction)
{
	if (iSortDirection != sort_direction && sort_direction != sort_None)
		Sort(sort_direction == sort_Ascending);
	iSortDirection = sort_direction;
}

///////////////////
// Create the combo box
CCombobox::CCombobox(COMMON_PARAMS) : CContainerWidget(name, parent)
{
	iSelected = -1;
	iItemHeight = 0;
	iSortDirection = sort_None;
	cSelectedItem = NULL;
	cItemList = NULL;
	bUnique = false;
	iType = wid_Combobox;
	iExpandArrow.set(Color(0, 0, 0), DEFAULT_PRIORITY);
	iExpandFace.set(Color(191, 191, 191), DEFAULT_PRIORITY);
	iExpandLight.set(Color(255, 255, 255), DEFAULT_PRIORITY);
	iExpandShadow.set(Color(127, 127, 127), DEFAULT_PRIORITY);
	cBackground.setColor(Color(255, 255, 255), DEFAULT_PRIORITY);
	cFont.iColor.set(Color(0, 0, 0), DEFAULT_PRIORITY);
	CLEAR_EVENT(OnChange);
	bmpExpand.set(NULL, DEFAULT_PRIORITY);
}


///////////////////
// Destroy the combo box
CCombobox::~CCombobox()
{
	tItems.clear();
}



///////////////////
// Mouse down event
int CCombobox::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if(RelInBox(x, y))  {
		if (!cItemList)
			cItemList = new CComboItemList(this);
		else
			cItemList->Destroy();
	}

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);

	return WID_PROCESSED;
}

///////////////////////
// Mouse up event
int CCombobox::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);

	if (cItemList && !RelInBox(x, y))  {
		cItemList->Destroy();
	}


	return WID_PROCESSED;
}

/////////////////////
// The item list has been closed
void CCombobox::OnItemListClose()
{
	cItemList = NULL; // Freed by the gui layout
	Repaint();
}

/////////////////////
// Switch to next item in the list
void CCombobox::selectNext()
{
	if ((size_t)iSelected + 1 < tItems.size())
		setCurItem(iSelected + 1);
}

//////////////////////
// Switch to previous item in the list
void CCombobox::selectPrev()
{
	if(iSelected > 0)
		setCurItem(iSelected - 1);
}

///////////////////
// Mouse wheel down event
int CCombobox::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if(!cItemList) // Not dropped
		selectNext();

	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse wheel up event
int CCombobox::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if(!cItemList)
		selectPrev();

	CWidget::DoMouseWheelUp(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

////////////////////
// Lose focus event
int CCombobox::DoLoseFocus(CWidget *new_focused)
{
	CContainerWidget::DoLoseFocus(new_focused);

	// Close the item list
	if (cItemList && new_focused != cItemList)  {
		cItemList->Destroy();
		Repaint();
	}

	return WID_PROCESSED;
}

////////////////////
// Key down event
int CCombobox::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	CContainerWidget::DoKeyDown(c, keysym, modstate);

	// Pass the event down to the item list if it's opened
	if (cItemList)
		cItemList->DoKeyDown(c, keysym, modstate);

	return WID_NOT_PROCESSED;
}

////////////////////
// Find an item based on its start letter
int CCombobox::findItem(UnicodeChar startLetter) {
	if(startLetter <= 31) return -1;

	bool letterAlreadySelected = false;
	int first = -1; // save first found index here
	int index = 0; // current index for loop
	startLetter = UnicodeToLower(startLetter);
	for(std::list<CComboItem>::const_iterator it = tItems.begin(); it != tItems.end(); it++, index++) {
		UnicodeChar c = GetUnicodeFromUtf8(it->getName(), 0);
		c = UnicodeToLower(c);
		if(c == startLetter) {
			if(first < 0) first = index;
			if(index > iSelected && letterAlreadySelected) return index;
			if(index == iSelected) letterAlreadySelected = true;
		}
	}

	return first;
}

////////////////////////
// Returns the lower bound iterator for the item
std::list<CComboItem>::iterator CCombobox::lowerBound(const CComboItem& item, int *index, bool *equal)
{
	if (tItems.size() == 0) return tItems.end();

	size_t n = tItems.size();
	std::list<CComboItem>::iterator result = tItems.begin();
	*equal = false;
	*index = 0;

	// HINT: check if we're lucky and the last item is the lower bound (this is surprisingly pretty often when adding items)
	{
		const int res = compare_items(*tItems.rbegin(), item);
		if (res <= 0)  {
			result = tItems.end();
			*index = (int)tItems.size();
			*equal = (res == 0);
			return result;
		}
	}

	// Binary search
	while (n)  {
		size_t mid = n / 2;
		std::list<CComboItem>::iterator mid_value = result;
		std::advance(mid_value, mid);
		const int res = compare_items(*mid_value, item);
		if (res < 0)  {
			result = ++mid_value;
			++mid;
			n -= mid;
			*index += (int)mid;
		} else if (res > 0)
			n = mid;
		else  { // Exact match
			*index += (int)mid;
			*equal = true;
			return mid_value;
		}
	}

	return result;
}

////////////////////////
// Returns the upper bound iterator for the item
std::list<CComboItem>::iterator CCombobox::upperBound(const CComboItem& item, int *index, bool *equal)
{
	if (tItems.size() == 0) return tItems.end();

	size_t n = tItems.size();
	std::list<CComboItem>::iterator result = tItems.begin();
	*equal = false;
	*index = 0;

	// HINT: check if we're lucky and the first item is the upper bound (this is surprisingly pretty often when adding items)
	{
		const int res = compare_items(*tItems.begin(), item);
		if (res <= 0)  {
			result = tItems.begin();
			*index = 0;
			*equal = (res == 0);
			return result;
		}
	}

	// Binary search
	while (n)  {
		size_t mid = n / 2;
		std::list<CComboItem>::iterator mid_value = result;
		std::advance(mid_value, mid);
		const int res = compare_items(*mid_value, item);
		if (res > 0)  {
			result = ++mid_value;
			++mid;
			n -= mid;
			*index += (int)mid;
		} else if (res < 0)
			n = mid;
		else  { // Exact match
			*index += (int)mid;
			*equal = true;
			return mid_value;
		}
	}

	return result;
}

/////////////////
// Add item
int	CCombobox::AddItem(const std::string& name)
{
	return AddItem(CComboItem(this, name));
}

int	CCombobox::AddItem(const std::string& sindex, const std::string& name)
{
	return AddItem(CComboItem(this, sindex, name));
}

int CCombobox::AddItem(int tag, const std::string& sindex, const std::string& name)
{
	CComboItem it(this, sindex, name);
	it.setTag(tag);
	return AddItem(it);
}

///////////////////
// Add an item to the combo box
int CCombobox::AddItem(const CComboItem& item1)
{
	//item.setParent(this); // Safety
	CComboItem item(item1);
	int index = (int)tItems.size();
	iItemHeight = MAX(iItemHeight, item.getHeight());

	//
	// Add it to the list
	//

	CComboItem * itemPtr = NULL;

	// First item
	if (tItems.size() == 0)  {
		tItems.push_back(item);
		itemPtr = & tItems.back();

	// Not first item
	} else switch (iSortDirection)  {

	// Ascending sorting
	case sort_Ascending:  {
		bool equal;
		std::list<CComboItem>::iterator it = lowerBound(item, &index, &equal);

		if (!bUnique || !equal)
			itemPtr = & (* tItems.insert(it, item));
	} break;

	// Descending sorting
	case sort_Descending:  {
		bool equal;
		std::list<CComboItem>::iterator it = upperBound(item, &index, &equal);

		if (!bUnique || !equal)  {
			itemPtr = & (* tItems.insert(it, item));
		}
	} break;


	// Not sorted
	default:
		if (bUnique)
			if (getItem(item.getName()))
				return 0;

		index = (int)tItems.size();
		tItems.push_back(item);
		itemPtr = & tItems.back();
	}

	// Style the item
	if (itemPtr->getCSSClass().size() == 0 && itemPtr->getCSSID().size() == 0 && !itemPtr->hasInlineStyle())  {
		itemPtr->cNormalStyle = cNormalItem;
		itemPtr->cActiveStyle = cActiveItem;
		itemPtr->cClickedStyle = cClickedItem;
	} else if (cLayoutCSS) {
		LoadItemStyle(*cLayoutCSS, itemPtr->cNormalStyle, itemPtr->getCSSID(), itemPtr->getCSSClass(), "");
		LoadItemStyle(*cLayoutCSS, itemPtr->cActiveStyle, itemPtr->getCSSID(), itemPtr->getCSSClass(), "hover");
		LoadItemStyle(*cLayoutCSS, itemPtr->cActiveStyle, itemPtr->getCSSID(), itemPtr->getCSSClass(), "down");
	}

	// current selection invalid
	if (iSelected < 0 || (size_t)iSelected >= tItems.size())  {
		// select this item
		setCurItem(index);
	}

    if (cItemList)
		cItemList->Readjust();

	return index;
}


///////////////////
// Set the current item based on count
void CCombobox::setCurItem(int index)
{
	if (iSelected == index)  {
		Repaint();
		return;
	}

	// Call the user code
	bool cancel = false;
	CALL_EVENT(OnChange, (this, getItemRW(index), index, cancel));
	if (cancel)
		return;

	iSelected = index;
	cSelectedItem = getItemRW(index);
	if (cItemList)
		cItemList->Readjust();

	Repaint();
}

///////////////////
// Set the current item based on item pointer
void CCombobox::setCurItem(const CComboItem *it)
{
	setCurItem(getItemIndex(it));
}


///////////////////
// Set the current item based on string index
void CCombobox::setCurSIndexItem(const std::string& szString)
{
	int index = getIndexBySIndex(szString);
	if(index >= 0)
		setCurItem(index);
}

void CCombobox::setCurItemByName(const std::string& szString)
{
	int index = getIndexByName(szString);
	if(index >= 0)
		setCurItem(index);
}

///////////////////////
// Get the item index based on its name
// If there are more items with the same name, the first found is returned
int CCombobox::getIndexByName(const std::string& szString) {
	int index = 0;

	// If sorted, use faster binary search
	if (iSortDirection == sort_None)  {
		for(std::list<CComboItem>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
			if( stringcasecmp(i->getName(), szString) == 0 ) {
				return index;
			}
		}

	// Not sorted
	} else {
		int index;
		bool found;
		std::list<CComboItem>::const_iterator i = lowerBound(CComboItem(this, szString), &index, &found);
		return found ? index : -1;
	}
    return -1;
}

/////////////////////
// Get the item index based on its sindex
int CCombobox::getIndexBySIndex(const std::string& szString) {
	int index = 0;
	for(std::list<CComboItem>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
		if( stringcasecmp(i->getSIndex(), szString) == 0 ) {
            return index;
        }
    }
    return -1;
}


///////////////////
// Set the image for the specified item
void CCombobox::setImage(SmartPointer<SDL_Surface> img, int ItemIndex)
{
	CComboItem* item = getItemRW(ItemIndex);
	if(item) item->setImage(img);
}


///////////////////
// Clear the data
void CCombobox::Clear()
{
	tItems.clear();
	setCurItem((const SkinnedGUI::CComboItem *)NULL);
	if (cItemList)
		cItemList->Readjust();
}


///////////////
// Get the item based on its index property
const CComboItem* CCombobox::getItem(int index) const
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<CComboItem>::const_iterator it = tItems.begin();
	std::advance(it, index);
	return &*it;
}

//////////////
// Same as the above, just enables the write access
CComboItem* CCombobox::getItemRW(int index)
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<CComboItem>::iterator it = tItems.begin();
	std::advance(it, index);
	return &*it;
}

/////////////
// Get the number of items
int	CCombobox::getItemsCount() {
	return (int)tItems.size();
}

/////////////
// Get the item based on its displayed name
const CComboItem* CCombobox::getItem(const std::string& name) const {
	for(std::list<CComboItem>::const_iterator it = tItems.begin(); it != tItems.end(); it++) {
		if(stringcasecmp(it->getName(), name) == 0)
			return &*it;
	}

	return NULL;
}

/////////////
// Get the item based on its string index
const CComboItem* CCombobox::getSIndexItem(const std::string& sIndex) const {
	// TODO: make it faster by using a sorted list (or map)
	for(std::list<CComboItem>::const_iterator it = tItems.begin(); it != tItems.end(); it++) {
		if(stringcaseequal(it->getSIndex(), sIndex)) // TODO: why case insensitive?
			return &*it;
	}
	return NULL;
}

//////////////////
// Get the index of the selected item (-1 if no item selected)
int CCombobox::getSelectedIndex()
{
	return iSelected;
}

///////////////////
// Get the last item added (can return NULL)
const CComboItem* CCombobox::getLastItem() {
	if(tItems.empty())
		return NULL;
	else
		return &(*tItems.rbegin());
}

////////////////
// Returns the selected item (can return NULL)
const CComboItem* CCombobox::getSelectedItem() {
	return cSelectedItem;
}

////////////////////
// Returns the list-index of the given item
int CCombobox::getItemIndex(const CComboItem* item) {
	if (item == NULL)
		return -1;

	int index = 0;
	for(std::list<CComboItem>::iterator it = tItems.begin(); it != tItems.end(); it++, index++) {
		if(&*it == item)
			return index;
	}

	return -1;
}

}; // namespace SkinnedGUI
