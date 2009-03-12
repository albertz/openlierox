/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui list view class
// Created 30/5/02
// Jason Boettcher


#ifndef __CLISTVIEW_H__SKINNED_GUI__
#define __CLISTVIEW_H__SKINNED_GUI__

#include <string>
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CScrollbar.h"
#include "SkinnedGUI/CItem.h"


namespace SkinnedGUI {

// Listview events
class CListview;
class CListviewItem;
typedef void(CWidget::*ListviewChangeHandler)(CListview *sender, CListviewItem *newitem, int newindex, bool& cancel);
typedef void(CWidget::*ListviewItemClickHandler)(CListview *sender, CListviewItem *item, int index);
typedef void(CWidget::*ListviewDoubleClickHandler)(CListview *sender, CListviewItem *item, int index);
#define SET_LSVCHANGE(listview, func)		SET_EVENT(listview, OnChange, ListviewChangeHandler, func)
#define SET_LSVITEMCLICK(listview, func)	SET_EVENT(listview, OnItemClick, ListviewItemClickHandler, func)
#define SET_LSVDBLCLICK(listview, func)		SET_EVENT(listview, OnDoubleClick, ListviewDoubleClickHandler, func)


// Sub-item types
enum SubitemType {
	sub_Image = 0,
	sub_Text,
	sub_Widget
};

class CListviewItem;

// Subitem structures
class CListviewSubitem : public CItem  {
public:
	CListviewSubitem(SubitemType type) : CItem("") { cItem = NULL; iType = type; }
	//CListviewSubitem(const CListviewSubitem& oth) : CItem("")	{ operator=(oth); }
	virtual ~CListviewSubitem() {}
protected:
	SubitemType iType;
	CListviewItem *cItem;
	bool bUseCustomStyle; // If false, the style of the parent item will be used
	bool bVisible;

	// Operators
	virtual CListviewSubitem& operator=(const CListviewSubitem& i2)  {
		CItem::operator =(i2);
		if (&i2 != this)  {
			iType = i2.iType;
			cItem = i2.cItem;
			bUseCustomStyle = i2.bUseCustomStyle;
			bVisible = i2.bVisible;
		}
		return *this;
	}

	void RepaintParent();

private:
	friend class CListviewItem;
	void setParentItem(CListviewItem *parent)	{ cItem = parent; }

public:
	static CListviewSubitem *Clone(const CListviewSubitem *orig);

	CListviewItem *getParentItem() const	{ return cItem; }
	SubitemType	getType() const				{ return iType; }

	bool useCustomStyle()					{ return bUseCustomStyle; }
	void setCustomStyle(bool _s)			{ bUseCustomStyle = _s; RepaintParent(); }

	bool isVisible()						{ return bVisible; }
	void setVisible(bool _v)				{ bVisible = _v; RepaintParent(); }
};

// Image subitem
class CImageSubitem : public CListviewSubitem  {
public:
	CImageSubitem(const CImageSubitem *oth) : CListviewSubitem(sub_Image) { *this = *oth; }
	CImageSubitem(CListviewItem *item, SDL_Surface *image) : CListviewSubitem(sub_Image)  
		{ cItem = item; setImage(image); }
	CImageSubitem(const CImageSubitem& sub) : CListviewSubitem(sub_Image)	{ operator=(sub); }

	// Operators
	CImageSubitem& operator= (const CImageSubitem& s2)  {
		CListviewSubitem::operator =(s2);
		return *this;
	}

	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);
};

// Text subitem
class CTextSubitem : public CListviewSubitem  {
public:
	CTextSubitem(CTextSubitem *oth) : CListviewSubitem(sub_Text) { *this = *oth; }
	CTextSubitem(CListviewItem *item, const std::string& text) : CListviewSubitem(sub_Text)
		{ cItem = item; setName(text); }
	CTextSubitem(const CTextSubitem& sub) : CListviewSubitem(sub_Text)	{ operator=(sub); }

	// Operators
	CTextSubitem& operator= (const CTextSubitem& s2)  {
		CListviewSubitem::operator =(s2);
		return *this;
	}

	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);
};

// Widget subitem
class CWidgetSubitem : public CListviewSubitem  {
public:
	CWidgetSubitem(CWidgetSubitem *oth) : CListviewSubitem(sub_Widget) { *this = *oth; }
	CWidgetSubitem(CListviewItem *item, CWidget *widget);
	CWidgetSubitem(const CWidgetSubitem& sub) : CListviewSubitem(sub_Widget)	{ operator=(sub); }
	~CWidgetSubitem();

private:
	CWidget *cWidget;

public:
	CWidgetSubitem& operator=(const CWidgetSubitem& i2)  {
		CListviewSubitem::operator =(i2);
		if (&i2 != this)  {
			cWidget = i2.cWidget;
		}
		return *this;
	}


	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);
	SDL_Rect getWidgetRect(int item_h);

	// Attributes
	CWidget *getWidget()		{ return cWidget; }
	int getWidth();
	int getHeight();
};

// Item structure
class CListviewItem : public CItem {
public:
	CListviewItem(CListview *parent) : CItem("") 
		{ cParent = parent; bVisible = true; bSelected = false; bSelectedInactive = false; }
	CListviewItem(const CListviewItem& it) : CItem("")  { operator=(it); }

	~CListviewItem();

private:
	CListview *cParent;
	std::list<CListviewSubitem *> tSubitems;
	bool bVisible;
	bool bSelected;
	bool bSelectedInactive;

private:
	friend class CListview;
	void setParent(CListview *parent)  { cParent = parent; }

public:
	CItemStyle	cSelectedStyle;
	CItemStyle	cSelectedInactiveStyle; // For selected item when the listview is not focused

public:
	// Operators
	CListviewItem& operator=(const CListviewItem& i2)  {
		CItem::operator =(i2);
		if (&i2 != this)  {
			tSubitems = i2.tSubitems;
			cParent = i2.cParent;
			bVisible = i2.bVisible;
			bSelected = i2.bSelected;
			bSelectedInactive = i2.bSelectedInactive;
			cSelectedStyle = i2.cSelectedStyle;
			cSelectedInactiveStyle = i2.cSelectedInactiveStyle;
		}
		return *this;
	}

	// Attributes
	std::list<CListviewSubitem *>& getSubitems()	{ return tSubitems; }
	CListviewSubitem *getSubitem(int index) const;
	CListview *getParent()		{ return cParent; }
	void setVisible(bool _v)	{ bVisible = _v; RepaintParent(); }
	bool isVisible()			{ return bVisible; }
	void setSelected(bool _s)	{ bSelected = _s; }
	bool isSelected()			{ return bSelected; }
	void setSelectedInactive(bool _s)	{ bSelectedInactive = _s; }
	bool isSelectedInactive()	{ return bSelectedInactive; }
	CItemStyle *getCurrentStyle();

	// Methods
	void AppendSubitem(const std::string& text);
	void AppendSubitem(SDL_Surface *image);
	void AppendSubitem(CWidget *widget);
	void AppendSubitem(const CListviewSubitem& s);
	void InsertSubitem(const CListviewSubitem& s, int index);
	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);
	void RepaintParent();
};


// Listview column
enum SortDirection  {
	sort_None = -1,
	sort_Descending = 0,
	sort_Ascending = 1
};

class CListviewColumn  {
public:
	CListviewColumn(CListview *parent);
	CListviewColumn(CListview *parent, const std::string& text);
	CListviewColumn(CListview *parent, const std::string& text, int width);

private:
	CListview *cParent;
	std::string sText;
	std::string sCSSClass;
	std::string sCSSID;
	class CColumnStyle  { public:
		CColumnStyle() { bmpSortArrow.set(NULL, DEFAULT_PRIORITY); }

		CBackground cBackground;
		CBorder cBorder;
		CFontStyle cFont;
		CTextProperties cText;
		StyleVar<SmartPointer<SDL_Surface> > bmpSortArrow;

		CColumnStyle& operator =(const CColumnStyle& s2)  {
			if (&s2 != this)  {
				bmpSortArrow = s2.bmpSortArrow;
				cBackground = s2.cBackground;
				cBorder = s2.cBorder;
				cFont = s2.cFont;
			}
			return *this;
		}

		void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	};

	SortDirection iSortDirection;
	int iWidth;
	bool bMouseOver;
	bool bMouseDown;

public:
	CColumnStyle cNormalStyle;
	CColumnStyle cActiveStyle;
	CColumnStyle cClickedStyle;

private:
	int getAutoWidth();
	void ReadjustSize();
	CColumnStyle *getCurrentStyle();

private:
	friend class CListview;
	void setParent(CListview *parent)  { cParent = parent; }

public:
	// Operators
	CListviewColumn& operator =(const CListviewColumn& c2)  {
		if (&c2 != this)  {
			cParent = c2.cParent;
			sText = c2.sText;
			cNormalStyle = c2.cNormalStyle;
			cActiveStyle = c2.cActiveStyle;
			cClickedStyle = c2.cClickedStyle;
			iSortDirection = c2.iSortDirection;
			bMouseOver = c2.bMouseOver;
			bMouseDown = c2.bMouseDown;
			sCSSClass = c2.sCSSClass;
			sCSSID = c2.sCSSID;
		}
		return *this;
	}

	// Methods
	void Draw(SDL_Surface *bmpDest, int x, int y, int w, int h);

	// Attributes
	void setMouseOver(bool _o)				{ bMouseOver = _o;  }
	void setMouseDown(bool _d)				{ bMouseDown = _d;  }
	bool isMouseOver() const				{ return bMouseOver; }
	bool isMouseDown() const				{ return bMouseDown; }
	int getWidth() const					{ return iWidth; }
	void setWidth(int _w)					{ iWidth = _w; }
	SortDirection getSortDirection() const	{ return iSortDirection; }
	void setSortDirection(SortDirection _s)	{ iSortDirection = _s; }
	const std::string& getCSSClass() const	{ return sCSSClass; }
	void setCSSClass(const std::string& c)	{ sCSSClass = c; }
	const std::string& getCSSID() const		{ return sCSSID; }
	void setCSSID(const std::string& i)		{ sCSSID = i; }
};

// Listview control class
class CListview: public CContainerWidget {
public:
	// Constructors
	CListview(COMMON_PARAMS);
	~CListview();


private:
	// Attributes
	StyleVar<bool>          bShowSelect;
	StyleVar<bool>			bAlwaysVisibleScrollbar;
	StyleVar<bool>			bShowColumnHeaders;

	SDL_Rect		tItemArea;  // Rect of the area with items

	// Columns
	std::vector<CListviewColumn *> tColumns;
	int				iGrabbed; // Index of the grabbed column
	int				iColumnHeight;

	// Items
	std::list<CListviewItem *> tItems;
	CListviewItem		*tSelected;
    int					iSelected;
	CListviewSubitem	*tSelectedSub;
	int					iSelectedSub;

	// TODO: get rid of this (add the doubleclick event to CWidget)
	AbsTime			fLastMouseUp;

	// Scrollbar
	CScrollbar		*cScrollbar;
	int				iSavedScrollbarPos;

	// Style
	CListviewColumn::CColumnStyle	cNormalColumn;
	CListviewColumn::CColumnStyle	cActiveColumn;
	CListviewColumn::CColumnStyle	cClickedColumn;
	CListviewItem::CItemStyle		cNormalItem;
	CListviewItem::CItemStyle		cActiveItem;
	CListviewItem::CItemStyle		cClickedItem;
	CListviewItem::CItemStyle		cSelectedItem;
	CListviewItem::CItemStyle		cSelectedInactiveItem;
	CListviewSubitem::CItemStyle	cNormalSubitem;
	CListviewSubitem::CItemStyle	cActiveSubitem;
	CListviewSubitem::CItemStyle	cClickedSubitem;

	// Other
	UnicodeChar		iLastChar;
	CWidget			*tFocusedSubWidget;
	CWidget			*tMouseOverSubWidget;
	CListviewItem	*tMouseOverItem;

	friend class CListviewColumn;
	CBorder			cBorder;
	CBackground		cBackground;

	// Events
	DECLARE_EVENT(OnChange, ListviewChangeHandler);
	DECLARE_EVENT(OnDoubleClick, ListviewDoubleClickHandler);
	DECLARE_EVENT(OnItemClick, ListviewItemClickHandler);

private:

	void	ReadjustItemArea();
	void	ReadjustScrollbar();

	int DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int DoKeyUp(UnicodeChar c, int keysym,  const ModifiersState& modstate);
	int	DoLoseFocus(CWidget *new_focused);
	int DoFocus(CWidget *prev_focused);

	void DoRepaint();
	int DoCreate();
	int DoChildNeedsRepaint(CWidget *child);

	void setSelected(const std::list<CListviewItem *>::iterator& item, int index);
	void setSelectedSub(CListviewSubitem *sub, int index);

	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void ApplyCSS(CSSParser& css);
	void LoadItemStyle(CItem::CItemStyle& style, const CSSParser& css, const std::string& element, const std::string& id, const std::string cl, const std::string& psclass);
	void LoadColumnStyle(CListviewColumn::CColumnStyle& style, const CSSParser& css, const std::string& id, const std::string cl, const std::string& psclass);
	void ApplyTag(xmlNodePtr node);

public:
	// Publish some of the default events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnMouseDown, MouseHandler)
	EVENT_SETGET(OnChange, ListviewChangeHandler)
	EVENT_SETGET(OnItemClick, ListviewItemClickHandler)
	EVENT_SETGET(OnDoubleClick, ListviewDoubleClickHandler)

	// Methods
	void	Clear();

	void	SortBy(int column, bool ascending);
	void	ReSort();

	void	AddColumn(const std::string& text);
	void	AddColumn(const std::string& text, int width);
	void	AddColumn(const CListviewColumn& col);

	void	AddItem(const std::string& sindex, int tag = 0);
	void	AddItem(CListviewItem& item);

	void	AddSubitem(CListviewSubitem& sub);
	void	AddTextSubitem(const std::string& text);
	void	AddImageSubitem(SDL_Surface *image);
	void	AddWidgetSubitem(CWidget *wid);

	void	RemoveItem(int index);


	// Getters and setters
	int		getItemTag(int index);

	size_t	getItemCount() const	{ return tItems.size(); }

    void    scrollLast();

	int		getSelectedIndex() const	{ return iSelected; }
	void	setSelected(int _s);

	std::string getSelectedSIndex()		{ return tSelected ? tSelected->getSIndex() : ""; }

	CListviewSubitem	*getSelectedSubitem()	{ return tSelectedSub; }
	CListviewItem		*getSelected()			{ return tSelected; }

	std::list<CListviewItem *>&			getItems()		{ return tItems; }
	std::vector<CListviewColumn *>&		getColumns()	{ return tColumns; }

	CListviewItem		*getLastItem(void)	{ return tItems.empty() ? NULL : *tItems.rbegin(); }
	CListviewItem*		getItem(int index);
	CListviewItem*		getItem(const std::string& sindex);
	CListviewSubitem	*getSubItem(int item_index, int subitem_index);

	void	setColumnHeight(int _h)  { iColumnHeight = _h; ReadjustItemArea(); }
	int		getColumnHeight()  { return iColumnHeight; }

    void    setShowSelect(bool s)   { bShowSelect.set(s, HIGHEST_PRIORITY); }
	bool	getShowSelect()	const	{ return bShowSelect; }

	void	setShowColumnHeaders(bool s)	{ bShowColumnHeaders.set(s, HIGHEST_PRIORITY); ReadjustItemArea(); }
	bool	getShowColumnHeaders()			{ return bShowColumnHeaders; }

	void	SaveScrollbarPos(void)    { iSavedScrollbarPos = cScrollbar->getValue(); }
	void	RestoreScrollbarPos(void) { cScrollbar->setValue(iSavedScrollbarPos); iSavedScrollbarPos = 0; }

	static const std::string tagName()	{ return "list"; }
	const std::string getTagName()		{ return CListview::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CLISTVIEW_H__SKINNED_GUI__
