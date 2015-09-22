/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui Listview class
// Created 16/6/02
// Jason Boettcher


#include "LieroX.h"

//#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "FindFile.h"
#include "Cursor.h"
#include "XMLutils.h"
#include "SkinnedGUI/CListview.h"


namespace SkinnedGUI {

#define MIN_COL_WIDTH 5

//
// Listview subitem
//

/////////////////////
// Repaint the parent listview
void CListviewSubitem::RepaintParent()
{
	if (cItem)
		cItem->RepaintParent();
}

////////////////
// Draw the image subitem
void CImageSubitem::Draw(SDL_Surface *bmpDest, const SDL_Rect &r)
{
	CItemStyle *style = getCurrentStyle();

	// Background
	if (bUseCustomStyle)
		style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);
	
	// Image
	DrawItemImage(bmpDest, r);

	// Border
	if (bUseCustomStyle)
		style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

/////////////////
// Draw the text subitem
void CTextSubitem::Draw(SDL_Surface *bmpDest, const SDL_Rect &r)
{
	CItemStyle *style = getCurrentStyle();

	// Background
	if (bUseCustomStyle)
		style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);
	
	// Image
	DrawItemText(bmpDest, r);

	// Border
	if (bUseCustomStyle)
		style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

////////////////////
// Constructor
CWidgetSubitem::CWidgetSubitem(CListviewItem *item, CWidget *widget) : CListviewSubitem(sub_Widget)
{
	cItem = item;
	cWidget = widget;

	// Move the widget to the listview
	if (cItem)
		if (cItem->getParent())
			widget->setParent(cItem->getParent());
}

///////////////////
// Destructor
CWidgetSubitem::~CWidgetSubitem()
{
	if (cWidget)
		delete cWidget;
}

///////////////////
// Get the widget rectangle
SDL_Rect CWidgetSubitem::getWidgetRect(int item_h)
{
	CItemStyle *style = getCurrentStyle();
	SDL_Rect r = { (Sint16)style->cBorder.getLeftW(), 0, (Uint16)cWidget->getWidth(), (Uint16)cWidget->getHeight()};
	switch (iVAlign)  {
	case algTop:
		r.y = style->cBorder.getTopW();
		break;
	case algMiddle:
		r.y = style->cBorder.getTopW() + (item_h - cWidget->getHeight())/2;
		break;
	case algBottom:
		r.y = item_h - cWidget->getHeight() - style->cBorder.getBottomW();
		break;
	}

	return r;
}

/////////////////////
// Draw the widget subitem
void CWidgetSubitem::Draw(SDL_Surface *bmpDest, const SDL_Rect &r)
{
	CItemStyle *style = getCurrentStyle();

	// Background
	if (bUseCustomStyle)
		style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);
	
	// Widget
	if (cWidget)  {
		ScopedSurfaceClip(bmpDest, r);

		// Draw according to the valign
		switch (iVAlign)  {
			case algTop:
				cWidget->Draw(bmpDest, r.x + style->cBorder.getLeftW(), r.y + style->cBorder.getTopW());
			break;

			case algMiddle:
				cWidget->Draw(bmpDest, r.x + style->cBorder.getLeftW(), r.y + style->cBorder.getTopW() + (r.h - cWidget->getHeight())/2);
			break;

			case algBottom:
				cWidget->Draw(bmpDest, r.x + style->cBorder.getLeftW(), r.y + r.h - cWidget->getHeight() - style->cBorder.getBottomW());
			break;
		}
	}

	// Border
	if (bUseCustomStyle)
		style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

////////////////////
// Get the height
int CWidgetSubitem::getHeight()
{
	CItemStyle *style = getCurrentStyle();
	if (cWidget)
		return cWidget->getHeight() + style->cBorder.getTopW() + style->cBorder.getBottomW();
	else
		return 0;
}

/////////////////////
// Get the width
int CWidgetSubitem::getWidth()
{
	CItemStyle *style = getCurrentStyle();
	if (cWidget)
		return cWidget->getWidth() + style->cBorder.getLeftW() + style->cBorder.getRightW();
	else
		return 0;
}

//
// Item
//

////////////////
// Destroy
CListviewItem::~CListviewItem()
{
	// Free the subitems
	for (std::list<CListviewSubitem *>::iterator sub = tSubitems.begin(); sub != tSubitems.end(); sub++)
		delete (*sub);

	tSubitems.clear();
}

//////////////////
// Repaint the parent
void CListviewItem::RepaintParent()
{
	if (cParent)
		cParent->Repaint();
}

//////////////////
// Get the subitem based on its index
CListviewSubitem *CListviewItem::getSubitem(int index) const
{
	std::list<CListviewSubitem *>::const_iterator it = tSubitems.begin();
	SafeAdvance(it, (size_t)index, tSubitems.end());
	if (it == tSubitems.end())
		return NULL;
	else
		return (*it);
}

////////////////
// Append a subitem
void CListviewItem::AppendSubitem(const CListviewSubitem &s)
{
	CListviewSubitem *newsub = (CListviewSubitem *)s.Clone();
	newsub->setParentItem(this);
	tSubitems.push_back(newsub);
	if (cParent)
		cParent->Repaint(); // Repaint the parent
}

void CListviewItem::AppendSubitem(const std::string &text)
{
	AppendSubitem((const CListviewSubitem &)CTextSubitem(this, text));
}

void CListviewItem::AppendSubitem(CWidget *widget)
{
	AppendSubitem(CWidgetSubitem(this, widget));
}

void CListviewItem::AppendSubitem(SDL_Surface *image)
{
	AppendSubitem(CImageSubitem(this, image));
}

////////////////////
// Draw the item
void CListviewItem::Draw(SDL_Surface *bmpDest, const SDL_Rect &r)
{
	// Save the clipping rect
	ScopedSurfaceClip clip(bmpDest, r);

	CItemStyle *style = getCurrentStyle();

	// Background
	style->cBackground.Draw(bmpDest, r.x, r.y, r.w, r.h);

	// Draw the subitems
	std::list<CListviewSubitem *>::iterator subit = tSubitems.begin();
	std::vector<CListviewColumn *>::const_iterator colit = cParent->getColumns().begin();
	int cur_x = r.x + style->cBorder.getLeftW();

	for ( ; subit != tSubitems.end() && colit != cParent->getColumns().end(); colit++, subit++)  {
		(*subit)->Draw(bmpDest, MakeRect(cur_x, r.y + style->cBorder.getTopW(), (*colit)->getWidth(), 
				r.h - style->cBorder.getTopW() - style->cBorder.getBottomW()));

		cur_x += (*colit)->getWidth();
	}

	// If no subitems, draw the text
	if (tSubitems.size() == 0)  {
		DrawItemText(bmpDest, r);
	}

	// Border
	style->cBorder.Draw(bmpDest, r.x, r.y, r.w, r.h);
}

///////////////////////
// Get the current style for drawing
CListviewItem::CItemStyle *CListviewItem::getCurrentStyle()
{
	if (bDown)
		return &cClickedStyle;
	if (bActive)
		return &cActiveStyle;
	if (bSelected)
		return &cSelectedStyle;
	if (bSelectedInactive)
		return &cSelectedInactiveStyle;
	return &cNormalStyle;
}

//
// Column
//

////////////////
// Create
CListviewColumn::CListviewColumn(CListview *parent)
{
	cParent = parent;
	sText = "";
	iWidth = getAutoWidth();
	iSortDirection = sort_None;
	bMouseOver = bMouseDown = false;
	
}

CListviewColumn::CListviewColumn(CListview *parent, const std::string &text)
{
	cParent = parent;
	sText = text;
	iWidth = getAutoWidth();
	iSortDirection = sort_None;
	bMouseOver = bMouseDown = false;
}

CListviewColumn::CListviewColumn(CListview *parent, const std::string &text, int width)
{
	cParent = parent;
	sText = text;
	iWidth = (width > 0) ? width : getAutoWidth();
	iSortDirection = sort_None;
	bMouseOver = bMouseDown = false;
}

//////////////////
// Apply the given selector to the column
void CListviewColumn::CColumnStyle::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	cBackground.ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	// Go through the attributes
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "arrow-image")
			bmpSortArrow.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
	}
}

////////////////////
// Adjusts the size of the column header according to the current style
void CListviewColumn::ReadjustSize()
{
	CColumnStyle *style = getCurrentStyle();
	cParent->setColumnHeight(MAX(cParent->getColumnHeight(), GetTextHeight(getCurrentStyle()->cFont, sText) + style->cBorder.getTopW() +
		style->cBorder.getBottomW()));
	cParent->ReadjustScrollbar();
}

////////////////////
// Get the style based on the column state
CListviewColumn::CColumnStyle *CListviewColumn::getCurrentStyle()
{
	if (bMouseDown)
		return &cClickedStyle;
	if (bMouseOver)
		return &cActiveStyle;
	return &cNormalStyle;
}

/////////////////////
// Get the column width automatically
int CListviewColumn::getAutoWidth()
{
	if (!cParent)
		return 0;

	CColumnStyle *style = getCurrentStyle();

	// Preferred width of the column
	int preferred_w = GetTextWidth(style->cFont, sText) + 20; // 20 - spacing

	// Width of all columns in the listview
	int all_column_w = 0;
	for (std::vector<CListviewColumn *>::iterator it = cParent->getColumns().begin();
		it != cParent->getColumns().end(); it++)  {
			all_column_w += (*it)->getWidth();
	}

	// Available width for the columns
	int avail_width = cParent->getWidth() - cParent->cBorder.getLeftW() - cParent->cBorder.getRightW();


	// Check if there's some space left
	if (all_column_w < avail_width)  {
		if (preferred_w <= avail_width - all_column_w)
			return preferred_w;
		else
			return avail_width - all_column_w;
	} else {
		// Just resize one of the last columns to make space
		int retval = MIN_COL_WIDTH;
		std::vector<CListviewColumn *>::reverse_iterator i = cParent->getColumns().rbegin();
		while (i != cParent->getColumns().rend())
			if ((*i)->getWidth() > MIN_COL_WIDTH)  {
				int new_w = MAX((*i)->getWidth() / 2, MIN_COL_WIDTH);
				retval = (*i)->getWidth() - new_w;
				(*i)->setWidth(new_w);
				break;
			}

		return retval;
	}
}

////////////////////////
// Draw the column header
void CListviewColumn::Draw(SDL_Surface *bmpDest, int x, int y, int w, int h)
{
	CColumnStyle *style = getCurrentStyle();

	// Background
	style->cBackground.Draw(bmpDest, x, y, w, h);

	// Text
	SDL_Rect r = { (Sint16)(x + style->cBorder.getLeftW()), (Sint16)(y + style->cBorder.getTopW()),
		(Uint16)(w - ((style->bmpSortArrow.get().get() && iSortDirection != sort_None) ? style->bmpSortArrow->w : 0) - 
		style->cBorder.getLeftW() - style->cBorder.getRightW()), 
		(Uint16)(h -	style->cBorder.getTopW() - style->cBorder.getBottomW()) };

	style->cText.tFontRect = &r;
	DrawGameText(bmpDest, sText, style->cFont, style->cText);

	// Sort arrow
	SDL_Rect r1(MakeRect(x + w - 3 - style->bmpSortArrow->w, y, w, h));
	SDL_Rect r2(MakeRect(0, 0, style->bmpSortArrow->w, style->bmpSortArrow->h));
	if (style->bmpSortArrow.get().get() && iSortDirection != sort_None)
		DrawImageAdv(bmpDest, style->bmpSortArrow.get(), r1, r2);

	// Border
	style->cBorder.Draw(bmpDest, x, y, w, h);
}

//
// Listview
//

///////////////
// Add a column
void CListview::AddColumn(const CListviewColumn &col)
{
	CListviewColumn *newcol = new CListviewColumn(this);
	*newcol = col;
	newcol->setParent(this); // Safety

	// Style
	if ((newcol->getCSSClass().size() == 0 && newcol->getCSSID().size() == 0) || (cLayoutCSS == NULL))  { // Precached?
		newcol->cNormalStyle = cNormalColumn;
		newcol->cActiveStyle = cActiveColumn;
		newcol->cClickedStyle = cClickedColumn;
	} else {
		LoadColumnStyle(newcol->cNormalStyle, *cLayoutCSS, newcol->getCSSID(), newcol->getCSSClass(), "");
		LoadColumnStyle(newcol->cActiveStyle, *cLayoutCSS, newcol->getCSSID(), newcol->getCSSClass(), "hover");
		LoadColumnStyle(newcol->cActiveStyle, *cLayoutCSS, newcol->getCSSID(), newcol->getCSSClass(), "down");
	}

	tColumns.push_back(newcol);

	ReadjustItemArea();
	Repaint();
}

void CListview::AddColumn(const std::string& text)
{
	AddColumn(CListviewColumn(this, text));
}

void CListview::AddColumn(const std::string& text, int width)
{
	AddColumn(CListviewColumn(this, text, width));
}

////////////////
// Add an item
void CListview::AddItem(CListviewItem& item)
{
	CListviewItem *newit = new CListviewItem(this);
	*newit = item;
	newit->setParent(this);  // Safety

	// Style
	if ((newit->getCSSClass().size() == 0 && newit->getCSSID().size() == 0) || (cLayoutCSS == NULL))  {
		// Kind of a cache - style for items without any special CSS specified
		newit->cNormalStyle = cNormalItem;
		newit->cActiveStyle = cActiveItem;
		newit->cClickedStyle = cClickedItem;
		newit->cSelectedStyle = cSelectedItem;
		newit->cSelectedInactiveStyle = cSelectedInactiveItem;
	} else { // The item has a special style or class specified
		LoadItemStyle(newit->cNormalStyle, *cLayoutCSS, "item", newit->getCSSID(), newit->getCSSClass(), "");
		LoadItemStyle(newit->cActiveStyle, *cLayoutCSS, "item", newit->getCSSID(), newit->getCSSClass(), "hover");
		LoadItemStyle(newit->cClickedStyle, *cLayoutCSS, "item", newit->getCSSID(), newit->getCSSClass(), "down");
		LoadItemStyle(newit->cSelectedStyle, *cLayoutCSS, "item", newit->getCSSID(), newit->getCSSClass(), "selected");
		LoadItemStyle(newit->cSelectedInactiveStyle, *cLayoutCSS, "item", newit->getCSSID(), newit->getCSSClass(), "selected-inactive");
	}

	// Style the subitems
	for (std::list<CListviewSubitem *>::iterator subit = newit->getSubitems().begin(); subit != newit->getSubitems().end(); subit++)  {
		if ((*subit)->getCSSClass().size() == 0 && (*subit)->getCSSID().size() == 0)  { // Precached?
			(*subit)->cNormalStyle = cNormalSubitem;
			(*subit)->cActiveStyle = cActiveSubitem;
			(*subit)->cClickedStyle = cClickedSubitem;
		} else { // Some kind of a special item
			LoadItemStyle((*subit)->cNormalStyle, *cLayoutCSS, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "");
			LoadItemStyle((*subit)->cActiveStyle, *cLayoutCSS, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "hover");
			LoadItemStyle((*subit)->cClickedStyle, *cLayoutCSS, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "down");			
		}
	}


	tItems.push_back(newit);

	ReadjustScrollbar();
	Repaint();
}

void CListview::AddItem(const std::string& sindex, int tag)
{
	CListviewItem it(this);
	it.setSIndex(sindex);
	it.setTag(tag);
	AddItem(it);
}

/////////////////
// Add a subitem
void CListview::AddSubitem(CListviewSubitem& sub)
{
	if (getLastItem())  {
		getLastItem()->AppendSubitem(sub);
		// HINT: the item calls repaint on us automatically
	}
}

void CListview::AddTextSubitem(const std::string& text)
{
	if (getLastItem())  {
		getLastItem()->AppendSubitem(text);
		// HINT: the item calls repaint on us automatically
	}	
}

void CListview::AddImageSubitem(SDL_Surface *image)
{
	if (getLastItem())  {
		getLastItem()->AppendSubitem(image);
		// HINT: the item calls repaint on us automatically
	}
}

void CListview::AddWidgetSubitem(CWidget *wid)
{
	if (getLastItem())  {
		getLastItem()->AppendSubitem(wid);
		// HINT: the item calls repaint on us automatically
	}
}

////////////////////
// Applies a CSS to the listview
void CListview::ApplyCSS(CSSParser& css)
{
	CContainerWidget::ApplyCSS(css);

	// Pre-load default styles
	LoadItemStyle(cNormalItem, css, "item", "", "", "");
	LoadItemStyle(cActiveItem, css, "item", "", "", "hover");
	LoadItemStyle(cClickedSubitem, css, "item", "", "", "down");
	LoadItemStyle(cSelectedItem, css, "item", "", "", "selected");
	LoadItemStyle(cSelectedInactiveItem, css, "item", "", "", "selected-inactive");
	LoadItemStyle(cNormalSubitem, css, "subitem", "", "", "");
	LoadItemStyle(cActiveSubitem, css, "subitem", "", "", "hover");
	LoadItemStyle(cClickedItem, css, "subitem", "", "", "down");
	LoadColumnStyle(cNormalColumn, css, "", "", "");
	LoadColumnStyle(cActiveColumn, css, "", "", "hover");
	LoadColumnStyle(cClickedColumn, css, "", "", "down");

	// Apply it to the items
	for (std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)  {
		// Item
		if ((*it)->getCSSClass().size() == 0 && (*it)->getCSSID().size() == 0)  {
			(*it)->cNormalStyle = cNormalItem;
			(*it)->cActiveStyle = cActiveItem;
			(*it)->cClickedStyle = cClickedItem;
			(*it)->cSelectedStyle = cSelectedItem;
			(*it)->cSelectedInactiveStyle = cSelectedInactiveItem;
		} else {
			LoadItemStyle((*it)->cNormalStyle, css, "item", (*it)->getCSSID(), (*it)->getCSSClass(), "");
			LoadItemStyle((*it)->cActiveStyle, css, "item", (*it)->getCSSID(), (*it)->getCSSClass(), "hover");
			LoadItemStyle((*it)->cClickedStyle, css, "item", (*it)->getCSSID(), (*it)->getCSSClass(), "down");
			LoadItemStyle((*it)->cSelectedStyle, css, "item", (*it)->getCSSID(), (*it)->getCSSClass(), "selected");
			LoadItemStyle((*it)->cSelectedInactiveStyle, css, "item", (*it)->getCSSID(), (*it)->getCSSClass(), "selected-inactive");
		}

		// Subitems
		for (std::list<CListviewSubitem *>::iterator subit = (*it)->getSubitems().begin(); subit != (*it)->getSubitems().end(); subit++)  {
			if ((*subit)->getCSSClass().size() == 0 && (*subit)->getCSSClass().size() == 0)  {
				(*subit)->cNormalStyle = cNormalSubitem;
				(*subit)->cActiveStyle = cActiveSubitem;
				(*subit)->cClickedStyle = cClickedSubitem;
			} else {
				LoadItemStyle((*subit)->cNormalStyle, css, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "");
				LoadItemStyle((*subit)->cActiveStyle, css, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "hover");
				LoadItemStyle((*subit)->cClickedStyle, css, "subitem", (*subit)->getCSSID(), (*subit)->getCSSClass(), "down");
			}
		}
	}

	// Apply it to the columns
	for (std::vector<CListviewColumn *>::iterator it = tColumns.begin(); it != tColumns.end(); it++)  {
		if ((*it)->getCSSClass().size() == 0 && (*it)->getCSSID().size())  {
			(*it)->cNormalStyle = cNormalColumn;
			(*it)->cActiveStyle = cActiveColumn;
			(*it)->cClickedStyle = cClickedColumn;
		} else {
			LoadColumnStyle(cNormalColumn, css, (*it)->getCSSID(), (*it)->getCSSClass(), "");
			LoadColumnStyle(cActiveColumn, css, (*it)->getCSSID(), (*it)->getCSSClass(), "hover");
			LoadColumnStyle(cClickedColumn, css, (*it)->getCSSID(), (*it)->getCSSClass(), "down");
		}
	}
}

//////////////////
// Apply the given selector to the listview
void CListview::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CContainerWidget::ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);

	// Go through the attributes
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "show-selection")
			bShowSelect.set(it->getFirstValue().getBool(), it->getPriority());
		else if (it->getName() == prefix + "column-headers")
			bShowColumnHeaders.set(it->getFirstValue().getBool(), it->getPriority());
		else if (it->getName() == prefix + "scroll-always")
			bAlwaysVisibleScrollbar.set(it->getFirstValue().getBool(), it->getPriority());
	}
}

///////////////////////
// Apply the given tag
void CListview::ApplyTag(xmlNodePtr node)
{
	CContainerWidget::ApplyTag(node);

	// Listview properties
	if (xmlPropExists(node, "showselect"))
		bShowSelect.set(xmlGetBool(node, "showselect", bShowSelect), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "scrollalways"))
		bAlwaysVisibleScrollbar.set(xmlGetBool(node, "scrollalways"), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "showcolumns"))
		bShowColumnHeaders.set(xmlGetBool(node, "showcolumns"), TAG_ATTR_PRIORITY);

	// Columns and items
	xmlNodePtr child = node->children;
	while (child)  {
		// Column
		if (!xmlStrcasecmp(child->name, (const xmlChar *)"column"))  {
			std::string text = xmlGetString(child, "caption");
			int width = xmlGetInt(child, "width", -1);
			if (width == -1)
				AddColumn(text);
			else
				AddColumn(text, width);
		}

		// Item
		else if (!xmlStrcasecmp(child->name, (const xmlChar *)"item"))  {
			CListviewItem item(this);
			item.ApplyTag(child);
			AddItem(item);
		}

		child = child->next;
	}

}

///////////////////
// Loads an item style
void CListview::LoadItemStyle(CItem::CItemStyle &style, const CSSParser &css, const std::string &element, const std::string &id, const std::string cl, const std::string &psclass)
{
	CSSParser::Selector sel = css.getStyleForElement(element, id, cl, psclass, getMyContext());
	style.ApplySelector(sel);
}

////////////////////
// Load a column style
void CListview::LoadColumnStyle(CListviewColumn::CColumnStyle &style, const CSSParser &css, const std::string &id, const std::string cl, const std::string &psclass)
{
	CSSParser::Selector sel = css.getStyleForElement("column", id, cl, psclass, getMyContext());
	style.ApplySelector(sel);
}

///////////////////
// Draw the list view
void CListview::DoRepaint()
{
	CHECK_BUFFER;
	CContainerWidget::DoRepaint();

	int cur_y = cBorder.getTopW();

	// Repaint all the subwidgets that need it
	// TODO: some list of widgets so we don't have to iterate through everything?
	for (std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)
		for (std::list<CListviewSubitem *>::iterator sub = (*it)->getSubitems().begin(); sub != (*it)->getSubitems().end(); sub++)
			if ((*sub)->getType() == sub_Widget)
				if (((CWidgetSubitem *)(*sub))->getWidget()->needsRepaint())
					((CWidgetSubitem *)(*sub))->getWidget()->DoRepaint();

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Columns
	if (bShowColumnHeaders)  {
		int cur_x = cBorder.getLeftW();
		for (std::vector<CListviewColumn *>::iterator it = tColumns.begin(); it != tColumns.end(); it++)  {
			(*it)->Draw(bmpBuffer.get(), cur_x, cur_y, (*it)->getWidth(), iColumnHeight);
			cur_x += (*it)->getWidth();
		}

		cur_y += iColumnHeight;
	}

	// Items
	int i = 0;
	int scroll_w = ((cScrollbar->getVisible() || bAlwaysVisibleScrollbar) ? cScrollbar->getWidth() : 0);
	for (std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++, i++)  {
		// Item above the displayed area
		if (i < cScrollbar->getValue())
			continue;

		// Draw the item
		(*it)->Draw(bmpBuffer.get(), MakeRect(cBorder.getLeftW(), cur_y, getWidth() - cBorder.getLeftW() -
			cBorder.getRightW() - scroll_w, (*it)->getHeight()));

		cur_y += (*it)->getHeight();

		if (cur_y >= getHeight() - cBorder.getBottomW())
			break;
	}

	// Scrollbar
	if (cScrollbar->getVisible())  {
		if (cScrollbar->needsRepaint())
			cScrollbar->DoRepaint();
		cScrollbar->Draw(bmpBuffer.get(), cScrollbar->getX(), cScrollbar->getY());
	}

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

//////////////////////
// Lose focus event
int CListview::DoLoseFocus(CWidget *new_focused)
{
	if (tSelected)  {
		tSelected->setSelected(false);
		tSelected->setSelectedInactive(true);
		Repaint();
	}

	CContainerWidget::DoLoseFocus(new_focused);
	return WID_PROCESSED;
}

////////////////////
// Focus event
int CListview::DoFocus(CWidget *prev_focused)
{
	if (tSelected)  {
		tSelected->setSelectedInactive(false);
		tSelected->setSelected(true);
		Repaint();
	}

	CContainerWidget::DoFocus(prev_focused);
	return WID_PROCESSED;
}

////////////////////
// Readjusts the item area
void CListview::ReadjustItemArea()
{
	tItemArea.x = cBorder.getLeftW();
	tItemArea.y = cBorder.getTopW() + (bShowColumnHeaders ? iColumnHeight : 0);
	tItemArea.w = getWidth() - tItemArea.x - cBorder.getRightW();
	tItemArea.h = getHeight() - tItemArea.y - cBorder.getBottomW();

	cScrollbar->Resize(tItemArea.w - cScrollbar->getWidth(), tItemArea.y, cScrollbar->getWidth(), tItemArea.h);

	ReadjustScrollbar();
	Repaint();
}

///////////////////
// Re-adjust the scrollbar
void CListview::ReadjustScrollbar()
{
	// Go through each item and find the average height
	int count = 0;
	int size = 0;

	for(std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++) {
		size += (*it)->getHeight();
		count++;
	}


	if(count == 0) {
		cScrollbar->setItemsperbox(0);
		cScrollbar->setValue(0);
		cScrollbar->setVisible(false);

		// Return to prevent a divide-by-zero
		return;
	}

	// Find the average
	size /= count;

	int h = tItemArea.h / size;

	cScrollbar->setItemsperbox( h );
    cScrollbar->setMax(count);
    cScrollbar->setValue(0);

	cScrollbar->setVisible(count > h);
}


///////////////////
// Remove an item from the list
void CListview::RemoveItem(int index)
{
	// Check
	if (index < 0 || index >= (int)tItems.size())
		return;

	// Get the item
	std::list<CListviewItem *>::iterator it = tItems.begin();
	std::list<CListviewItem *>::iterator next = it, prev = it;
	std::advance(it, index);
	if (it != tItems.end())  {
		// If the item was mouse over, clear the item
		if ((*it) == tMouseOverItem)
			tMouseOverItem = NULL;

		next = it++;
		prev = ((it == tItems.begin()) ? it : it--);
		delete (*it); // Free it
		tItems.erase(it);
	}  else
		return; // Index out of the list

	// If the item was selected, select the next one
	if (iSelected == index)  {
		if (tItems.empty()) // No items left, remove the selection
			setSelected(-1);
		else { // There are still some items in the list
			if (next == tItems.end()) // Deleted last item, select the new last item
				setSelected(prev, (int)tItems.size());
			else
				setSelected(next, index); // Deleted item somewhere in the list, select the item coming after it
		}
	}

	// Readjust the scrollbar
	ReadjustScrollbar();

	Repaint();
}

///////////////////////
// Get the tag property of an item
int CListview::getItemTag(int index)
{
	CListviewItem *it = getItem(index);
	if (!it)  {
		warnings << "Tried to get an item tag for an invalid index, zero was returned" << endl;
		return 0;
	}

	return it->getTag();
}

////////////////////////
// Internal function for setting the selected subitem
// The caller is responsible for the valid input
void CListview::setSelectedSub(CListviewSubitem *sub, int index)
{
	tSelectedSub = sub;
	iSelectedSub = index;
}

////////////////////////
// Internal function for setting the selected item
// The caller is responsible for the valid input
void CListview::setSelected(const std::list<CListviewItem *>::iterator& item, int index)
{
	if (iSelected == index)
		return;

	// Call the OnChange event
	bool cancel = false;
	CALL_EVENT(OnChange, (this, ((index < 0 || item == tItems.end()) ? NULL : *item), index, cancel));
	if (cancel)
		return;

	iSelected = index;
	if (iSelected >= 0 && item != tItems.end())  {
		// Take off focus from the old item
		if (tSelected)  {
			tSelected->setSelected(false);
			tSelected->setSelectedInactive(false);
		}

		tSelected = (*item);
		if (getFocused())
			tSelected->setSelected(true);
		else
			tSelected->setSelectedInactive(true);

	} else  {
		iSelected = -1;
		tSelected = NULL;
	}

	// Repaint
	Repaint();
}

////////////////////////
// Set the selected item based on its index
void CListview::setSelected(int _s)
{
	// Remove selection?
	if (_s == -1)  {
		setSelected(tItems.end(), -1);
		return;
	}

	// Check
	if (_s < 0 || _s >= (int)tItems.size())
		return;

	// Select the item
	std::list<CListviewItem *>::iterator it = tItems.begin();
	std::advance(it, _s);
	setSelected(it, _s);
}

/////////////////////////
// Get the item based on its index
// Returns NULL when the index is out of range
CListviewItem *CListview::getItem(int index)
{
	// Check
	if (index < 0 || index >= (int)tItems.size())
		return NULL;

	// Get the item
	if (index > (int)tItems.size() / 2)  { // Start from end if it's "closer"
		std::list<CListviewItem *>::reverse_iterator it = tItems.rbegin();
		std::advance(it, (int)tItems.size() - index);
		return (*it);
	} else {
		std::list<CListviewItem *>::iterator it = tItems.begin();
		std::advance(it, index);
		return (*it);
	}
}

/////////////////////////
// Get the item based on its string index
// Returns NULL if no matching sindex found
CListviewItem *CListview::getItem(const std::string& sindex)
{
	for (std::list<CListviewItem *>::iterator it = tItems.begin();
		it != tItems.end(); it++)
		if ((*it)->getSIndex() == sindex)
			return  (*it);

	return NULL;
}


//////////////////////////
// Get the subitem from an item based on indexes
// Returns NULL if the indexes are not valid
CListviewSubitem *CListview::getSubItem(int item_index, int subitem_index)
{
	CListviewItem *it = getItem(item_index);
	if (!it)
		return NULL;
	return it->getSubitem(subitem_index);
}

///////////////
// Sorts the list by the current sorting column
// Useful, when you're re-filling the list or adding new items
void CListview::ReSort()
{
	// Find the column
	std::vector<CListviewColumn *>::iterator it = tColumns.begin();
	int i = 0;
	for (; it != tColumns.end(); it++, i++)
		if ((*it)->getSortDirection() != sort_None)
			break;

	// Not found
	if (it == tColumns.end())
		return;

	// Sort
	SortBy(i, (*it)->getSortDirection() == sort_Ascending);
}

////////////////
// Predicate used for sorting the list
	class CListviewSortPredicate  { 
	private:
		bool bAscending;
		int iSubitemIndex;

	public:
		CListviewSortPredicate(bool asc, int index) : bAscending(asc), iSubitemIndex(index) {}

		int operator ()(CListviewItem *item1, CListviewItem *item2)  {
			// Get the correct subitems
			CListviewSubitem *s1; 
			CListviewSubitem *s2;
			if (bAscending)  {
				s1 = item1->getSubitem(iSubitemIndex);
				s2 = item2->getSubitem(iSubitemIndex);
			} else { // Swapped
				s1 = item2->getSubitem(iSubitemIndex);
				s2 = item1->getSubitem(iSubitemIndex);
			}

			// Special case - one or both of the subitems are NULL
			if (s1 == NULL)  {
				if (s2 == NULL)
					return 0;
				else
					return -1;
			}
			if (s2 == NULL && s1 != NULL)
				return 1;

			// Swap the two items?
			bool failed1,failed2;
			int nat_cmp1 = from_string<int>(s1->getName(), failed1);
			int nat_cmp2 = from_string<int>(s2->getName(), failed2);

			// First try, if we compare numbers
			if (!failed1 && !failed2)  {
				if(nat_cmp1 == nat_cmp2)
					return s1->getName().size() - s2->getName().size(); // because from_string("123456") == from_string("123456abcd")
				else
					return nat_cmp1 - nat_cmp2;
			// String comparison
			} else {
				return stringcasecmp(s1->getName(), s2->getName());
			}
		}

	};

///////////////
// Sorts the listview by specified column, ascending or descending
void CListview::SortBy(int column, bool ascending)
{
	// Check
	if (column < 0 || column >= (int)tColumns.size())
		return;

	tItems.sort(CListviewSortPredicate(ascending, column));
}


///////////////////
// Clear the items
void CListview::Clear()
{
	// Free the items
	for (std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)
		delete (*it);
	tItems.clear();
	tMouseOverItem = NULL;

	// Remove selection
	setSelected(-1);

	// Hide the scrollbar
	ReadjustScrollbar();

	// Special container widget things
	tFocusedSubWidget = NULL;
	tMouseOverSubWidget = NULL;

	// Repaint
	Repaint();
}


///////////////////
// Create event
CListview::CListview(COMMON_PARAMS) : CContainerWidget(name, parent)
{
	setSelected(-1);
	setSelectedSub(NULL, -1);
	iType = wid_Listview;
	fLastMouseUp = AbsTime();
	iColumnHeight = 0;
    bShowSelect.set(true, DEFAULT_PRIORITY);
	bShowColumnHeaders.set(true, DEFAULT_PRIORITY);
	tMouseOverItem = NULL;
	tItemArea = MakeRect(0, 0, 0, 0);
	iGrabbed = 0;
	iSavedScrollbarPos = 0;
	iLastChar = 0;
	bAlwaysVisibleScrollbar.set(false, DEFAULT_PRIORITY);
	tFocusedSubWidget = NULL;
	tMouseOverSubWidget = NULL;
	CLEAR_EVENT(OnChange);
	CLEAR_EVENT(OnItemClick);
	CLEAR_EVENT(OnDoubleClick);

	cScrollbar = new CScrollbar(STATIC, this, scrVertical);
}


///////////////////
// Destroy event
CListview::~CListview()
{
	// Free the scrollbar
	delete cScrollbar;

	// Free the columns
	for (std::vector<CListviewColumn *>::iterator col = tColumns.begin(); col != tColumns.end(); col++)
		delete (*col);
	tColumns.clear();

	// Free the items
	for (std::list<CListviewItem *>::iterator it = tItems.begin(); it != tItems.end(); it++)
		delete (*it);
	tItems.clear();
}

//////////////////
// Create event
int CListview::DoCreate()
{
	CContainerWidget::DoCreate();

	// Adjust the size if needed
	Resize(getX(), getY(), getWidth() == 0 ? 100 : getWidth(), getHeight() == 0 ? 100 : getHeight());

	cScrollbar->DoCreate();
	ReadjustItemArea();

	return WID_PROCESSED;
}

///////////////////
// Mouse leave event
int CListview::DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState &modstate)
{
	CContainerWidget::DoMouseLeave(x, y, dx, dy, modstate);
	if (tMouseOverItem)  {
		tMouseOverItem->setActive(false);
		tMouseOverItem->setDown(false);
		tMouseOverItem = NULL;
	}

	return WID_PROCESSED;
}


///////////////////
// Mouse over event
int	CListview::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	if(cScrollbar->getVisible() && (cScrollbar->getGrabbed() || cScrollbar->InBox(x, y)))  {
		cScrollbar->DoMouseMove(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, down, button, modstate);

		CContainerWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
		return WID_PROCESSED;
	}

	// Check that the mouse is inside the listview
	if (!RelInBox(x, y))  {
		if (tMouseOverItem)  {
			tMouseOverItem->setActive(false);
			tMouseOverItem->setDown(false);
			tMouseOverItem = NULL;
		}

		CContainerWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
		return WID_PROCESSED;
	}

	// Reset the cursor
	SetGameCursor(CURSOR_ARROW);

	// Go through the columns and check, if the mouse is in the space between two columns
	if (bShowColumnHeaders && tColumns.size() > 1)  {
		std::vector<CListviewColumn *>::iterator it = tColumns.begin() + 1;
		std::vector<CListviewColumn *>::iterator prev = tColumns.begin();

		if(y > cBorder.getTopW() && y < tItemArea.y)  {
			int cur_x = cBorder.getLeftW() + tColumns[0]->getWidth();

			// Go through the columns
			for (; it != tColumns.end(); it++, prev++)  {
				if (x >= cur_x && x <= cur_x + 4)  { // Between the columns
					SetGameCursor(CURSOR_RESIZE);
					break;
				} else if (x >= cur_x && x < cur_x + (*it)->getWidth() - 2)  { // In the column
					if (!(*it)->isMouseDown())  {
						(*it)->setMouseDown(down);
						Repaint();
					}
					break;
				}

				cur_x += (*it)->getWidth() - 2;
			}
		} else {
			// Reset the click state of all headers
			for (; it != tColumns.end(); it++)  {
				if ((*it)->isMouseDown())  {
					(*it)->setMouseDown(false);
					Repaint();
				}
			}
		}

		// Is any of the columns grabbed? Move it
		if (iGrabbed > 0 && down)  {

			// Resize the two columns
			int w1, w2;
			w1 = w2 = 0;
			if (iGrabbed > 1)
				w1 = tColumns[iGrabbed - 1]->getWidth() + dx;
			w2 = tColumns[iGrabbed]->getWidth() - dx;

			// Resize only if they both will have at least minimal width
			if (w1 > MIN_COL_WIDTH && w2 > MIN_COL_WIDTH)  {
				tColumns[iGrabbed - 1]->setWidth(w1);
				tColumns[iGrabbed]->setWidth(w2);
				Repaint();
			}

			SetGameCursor(CURSOR_RESIZE);

			CContainerWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
			return WID_PROCESSED;
		}
	}

	// Go through items and subitems, processing the widgets
	tMouseOverSubWidget = NULL;
	if (tMouseOverItem)  {
		tMouseOverItem->setActive(false);
		tMouseOverItem->setDown(false);
		tMouseOverItem = NULL;
		Repaint();
	}

	int cur_y = tItemArea.y;
	int count = 0;
	for(std::list<CListviewItem *>::iterator item = tItems.begin(); // Go through the items
		item != tItems.end(); item++) {

			// Don't process invisible items
			if (!(*item)->isVisible())
				continue;

			if (count++ < cScrollbar->getValue())
				continue;

			// Reset the style
			(*item)->setActive(false);
			(*item)->setDown(false);

			// Check if the item is under the mouse
			if (y < cur_y || y >= cur_y + (*item)->getHeight())  {
				cur_y += (*item)->getHeight();
				if (cur_y >= tItemArea.y + tItemArea.h)
					break;
				continue;
			}

			// Change the style
			(*item)->setActive(true);
			(*item)->setDown(down);
			tMouseOverItem = (*item);

			std::list<CListviewSubitem *>::iterator subitem = (*item)->getSubitems().begin();
			std::vector<CListviewColumn *>::iterator col = tColumns.begin();
			int cur_x = tItemArea.x;

			for (; col != tColumns.end() &&
				subitem != (*item)->getSubitems().end(); subitem++, col++)  { // Go through the subitems

					// Don't process invisible subitems
					if (!(*subitem)->isVisible())
						continue;

					// Process the widget
					if ((*subitem)->getType() == sub_Widget)  {
						CWidgetSubitem *wsub = (CWidgetSubitem *)(*subitem);
						CWidget *w = wsub->getWidget();
						SDL_Rect r = wsub->getWidgetRect((*item)->getHeight());
						r.x += cur_x;
						r.y += cur_y;

						if(PointInRect(x, y, r))  {
							tMouseOverSubWidget = w;

							// If in the previous frame the widget wasn't under the mouse and now it is, fire the "enter" event
							if (!w->InBox(x - dy, y - dy))
								w->DoMouseEnter(x, y, dx, dy, modstate);

							// Mouse move event
							if (w->DoMouseMove(x, y, dx, dy, down, button, modstate) == WID_PROCESSED)  {
								CContainerWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
								return WID_PROCESSED;
							}
						} else {
							// If in the previous frame the widget was under the mouse and now it is not, fire the "leave" event
							if (PointInRect(x - dy, y - dy, r))
								w->DoMouseLeave(x, y, dx, dy, modstate);
						}
					}

					cur_x += (*col)->getWidth();
			}

			Repaint();

			cur_y += (*item)->getHeight();
			if (cur_y >= tItemArea.y + tItemArea.h)
				break;
	}

	CContainerWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse down event
int	CListview::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Scrollbar
	if(cScrollbar->InBox(x, y)) {
		cScrollbar->DoMouseDown(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, button, modstate);

		CContainerWidget::DoMouseDown(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	}

	//
	// Column headers
	//
	iGrabbed = -1; // No column grabbed yet
	if (bShowColumnHeaders)  {

		// Not grabbed
		if( y >= cBorder.getTopW() && y < tItemArea.y)  {
			int cur_x = cBorder.getLeftW();
			std::vector<CListviewColumn *>::iterator col = tColumns.begin();
			int i = 0;

			for (col++; col != tColumns.end(); col++, i++)  {
				// Reset
				(*col)->setMouseDown(false);
				(*col)->setMouseOver(false);

				// If in the area between two columns, grab
				if (x >= cur_x-2 && x <= cur_x + 2)  {
					iGrabbed = i;
					break;
				}

				// Click
				else if (x >= cur_x + 2 && x <= cur_x + (*col)->getWidth() - 2 && iGrabbed <= 0)  {
					SetGameCursor(CURSOR_ARROW);
					(*col)->setMouseDown(true);
					(*col)->setMouseOver(true);
				}

				cur_x += (*col)->getWidth();
			}

			CContainerWidget::DoMouseDown(x, y, dx, dy, button, modstate);
			return WID_PROCESSED;
		}
	}

	setSelectedSub(NULL, -1);
	bool clicked_widget = false;

	// Go through items and subitems, processing the widgets
	tMouseOverSubWidget = NULL;
	int i = 0;

	int cur_y = tItemArea.y;
	int count = 0;
	for(std::list<CListviewItem *>::iterator item = tItems.begin(); // Go through the items
		item != tItems.end(); item++, i++) {
			// Ignore invisible items
			if (!(*item)->isVisible())
				continue;

			// Check if the item is in the display area
			if (count++ < cScrollbar->getValue())
				continue;

			// Select the item after a click
			if (y >= cur_y && y < cur_y + (*item)->getHeight())
				setSelected(item, i);

			std::list<CListviewSubitem *>::iterator subitem = (*item)->getSubitems().begin();
			std::vector<CListviewColumn *>::iterator col = tColumns.begin();
			int j = 0;
			int cur_x = tItemArea.x;

			for (; col != tColumns.end() &&
				subitem != (*item)->getSubitems().end(); subitem++, col++, j++)  { // Go through the subitems

					// Don't process hidden subitems
					if (!(*subitem)->isVisible())
						continue;

					// Process the widget
					if ((*subitem)->getType() == sub_Widget)  {
						CWidgetSubitem *wsub = (CWidgetSubitem *)(*subitem);
						CWidget *w = wsub->getWidget();
						SDL_Rect r = wsub->getWidgetRect((*item)->getHeight());
						r.x += cur_x;
						r.y += cur_y;

						if(PointInRect(x, y, r))  {
							clicked_widget = true;
							tMouseOverSubWidget = w;

							// Focus the widget
							if (w != tFocusedSubWidget)  {
								tFocusedSubWidget->setFocused(false);
								tFocusedSubWidget = w;
								w->setFocused(true);
							}
						}

					}

					// Clicked in the subitem?
					if (PointInRect(x, y, MakeRect(cur_x, cur_y, (*col)->getWidth(), (*item)->getHeight())))  {
						setSelectedSub(*subitem, j);
					}

					cur_x += (*col)->getWidth();
			}

			cur_y += (*item)->getHeight();
	}

	// Remove the focus from any focused widget if no widget was clicked
	if (!clicked_widget)
		if (tFocusedSubWidget)  {
			tFocusedSubWidget->setFocused(false);
			tFocusedSubWidget = NULL;
			Repaint();
		}

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse up event
int	CListview::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if(cScrollbar->isMouseDown() && cScrollbar->getVisible())  {
		cScrollbar->DoMouseUp(x - cScrollbar->getX(), y - cScrollbar->getY(), dx, dy, button, modstate);
		CContainerWidget::DoMouseUp(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	} else {
		fLastMouseUp = AbsTime();
	}

	// Column headers
	if (bShowColumnHeaders)  {
		if( y >= cBorder.getTopW() && y < tItemArea.y && tLX->currentTime-fLastMouseUp >= 0.15f && iGrabbed <= 0)  {
			fLastMouseUp = tLX->currentTime;
			iGrabbed = -1;

			int cur_x = cBorder.getLeftW();
			std::vector<CListviewColumn *>::iterator col = tColumns.begin();
			for (int i = 0; col != tColumns.end(); col++, i++)  {
				(*col)->setMouseDown(false);

				// Mouse up over the column
				if (x >= cur_x && x < cur_x + (*col)->getWidth())  {
					bool asc = ((*col)->getSortDirection() != sort_Ascending);
					(*col)->setSortDirection(asc ? sort_Ascending : sort_Descending);
					SortBy(i, asc);
				} else
					(*col)->setSortDirection(sort_None);

				cur_x += (*col)->getWidth();
			}

			CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
			return WID_PROCESSED;
		}

		iGrabbed = 0;
	}

	setSelectedSub(NULL, -1);



	//
	// Process the items
	//

	// Send the event to the focused widget
	if (tFocusedSubWidget)  {
		tFocusedSubWidget->DoMouseUp(x, y, dx, dy, button, modstate);
	}

	int count = 0;
	int cur_y = tItemArea.y;
	std::list<CListviewItem *>::iterator item = tItems.begin();
	for(; item != tItems.end(); item++) {
		if (!(*item)->isVisible())
			continue;

		if(count++ < cScrollbar->getValue())
			continue;

		// Find the max height
		int h = (*item)->getHeight();

		if(y > cur_y && y < cur_y + h) {
			bool doubleclick = false;
			if(tSelected && button == /*MBT_LEFT*/0) {
				if(tSelected == (*item)) {
					if(tLX->currentTime - fLastMouseUp < 0.5f) {
						doubleclick = true;
						fLastMouseUp = AbsTime();

						CALL_EVENT(OnDoubleClick, (this, tSelected, count - 1));
					}
				}
			}

			// Go through the sub items, check which one was clicked on
			std::list<CListviewSubitem *>::iterator sub = (*item)->getSubitems().begin();
			std::vector<CListviewColumn *>::iterator col = tColumns.begin();
			int cur_x = tItemArea.x;
			int i=0;
			for(; sub != (*item)->getSubitems().end() && col != tColumns.end(); col++, sub++,i++) {

				if((*sub)->isVisible())
					if(x >= cur_x && x < cur_x + (*col)->getWidth())
						setSelectedSub(*sub, i);

				// HINT: mouseup can receive only focused widget => no need to test events here

				cur_x += (*col)->getWidth();
			}


			setSelected(item, count - 1);

			if(!doubleclick)
				fLastMouseUp = tLX->currentTime;

			return WID_PROCESSED;
		}

		// Check that we don't overflow
		cur_y += (*item)->getHeight();
		if(cur_y >= tItemArea.y + tItemArea.h)
			break;
	}

	CContainerWidget::DoMouseUp(x, y, dx, dy, button, modstate); 
	return WID_PROCESSED;
}

///////////////////
// Mouse wheel down event
int	CListview::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	CContainerWidget::DoMouseWheelDown(x, y, dx, dy, modstate);

	// Any sub-widget is active?
	if (tMouseOverSubWidget)  {
		if (tMouseOverSubWidget->DoMouseWheelDown(x, y, dx, dy, modstate) == WID_PROCESSED)  {
			return WID_PROCESSED;
		}
	}

	// Scrollbar
	if(cScrollbar->getVisible())  {
		cScrollbar->DoMouseWheelDown(x, y, dx, dy, modstate);
	}

	return WID_PROCESSED;
}

///////////////////
// Mouse wheel up event
int	CListview::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	CContainerWidget::DoMouseWheelUp(x, y, dx, dy, modstate);

	// Any sub-widget is active?
	if (tMouseOverSubWidget)  {
		if (tMouseOverSubWidget->DoMouseWheelUp(x, y, dx, dy, modstate) == WID_PROCESSED)  {
			return WID_PROCESSED;
		}
	}

	// Scrollbar
	if(cScrollbar->getVisible())  {
		cScrollbar->DoMouseWheelUp(x, y, dx, dy, modstate);
	}

	return WID_PROCESSED;
}

/////////////////
// Key down event
int CListview::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	// Any sub-widget is active?
	if (tFocusedSubWidget)  {
		if (tFocusedSubWidget->DoKeyDown(c, keysym, modstate) == WID_PROCESSED)  {
			return WID_PROCESSED;
		}
	}

	if (c >= 31)  {
		// TODO: handle normal characters
		// share some code with dropdownbox here
		//notes << "Normal keys are currently ignored for listview" << endl;
		//return LV_NONE; // don't return here, else we would ignore SDLK_DOWN etc.
	}

	switch (keysym)  {

	// Down arrow
	case SDLK_DOWN:
		setSelected(iSelected + 1);
	break;


	// Up arrow
	case SDLK_UP:
		setSelected(iSelected - 1);
	break;

	// Home
	case SDLK_HOME:
		setSelected(0);
	break;

	// End
	case SDLK_END:
		setSelected(tItems.size() - 1);
	break;
	}

	CWidget::DoKeyDown(c, keysym, modstate); // This calls user-defined actions
	return WID_NOT_PROCESSED;
}

////////////////
// Key up event
int CListview::DoKeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	// Any sub-widget is active?
	if (tFocusedSubWidget)  {
		if (tFocusedSubWidget->DoKeyUp(c, keysym, modstate) == WID_PROCESSED)  {
			return WID_PROCESSED;
		}
	}

	CContainerWidget::DoKeyUp(c, keysym, modstate);
	return WID_NOT_PROCESSED;
}

///////////////////
// Child repaint event
int CListview::DoChildNeedsRepaint(CWidget *child)
{
	CContainerWidget::DoChildNeedsRepaint(child);
	Repaint();

	return WID_PROCESSED;
}

///////////////////
// Scroll to the last item
void CListview::scrollLast()
{
	if (cScrollbar->getVisible())  {
		cScrollbar->setValue(cScrollbar->getMax());
		cScrollbar->UpdatePos();
	}
}

}; // namespace SkinnedGUI
