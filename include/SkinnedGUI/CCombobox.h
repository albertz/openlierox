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


#ifndef __CCOMBOBOX_H__SKINNED_GUI__
#define __CCOMBOBOX_H__SKINNED_GUI__

#include <list>
#include "GfxPrimitives.h"
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CScrollbar.h"
#include "SkinnedGUI/CButton.h"
#include "SkinnedGUI/CItem.h"


namespace SkinnedGUI {

class CCombobox;

// Combobox item
class CComboItem : public CItem  {
private:
	CCombobox *cParent;

public:
	CComboItem(CCombobox *parent, const std::string& name) : CItem(name) { cParent = parent; }
	CComboItem(CCombobox *parent, const std::string& sindex, const std::string& name) : CItem(sindex, name) { cParent = parent; }
	CComboItem(CCombobox *parent, const std::string& name, SmartPointer<SDL_Surface> image) : CItem(name, image) { cParent = parent; }
	CComboItem(CCombobox *parent, const std::string& sindex, SmartPointer<SDL_Surface> image, const std::string& name) : CItem(sindex, image, name) { cParent = parent; }

	void RepaintParent();

public:
	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);

	void setParent(CCombobox *p)	{ cParent = p; }
	CCombobox *getParent()			{ return cParent; }
};

// Combobox events
class CCombobox;
typedef void(CWidget::*ComboChangeHandler)(CCombobox *sender, const CComboItem *newitem, int newindex, bool& cancel);
#define SET_CMBCHANGE(combobox, func)	SET_EVENT(combobox, OnChange, ComboChangeHandler, func)

// The floating window
class CComboItemList : public CContainerWidget  {
public:
	CComboItemList(CCombobox *parent);
	~CComboItemList();

	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	void	DoRepaint();
	int		DoCreate();
	int		DoDestroy(bool immediate);
	int		DoLoseFocus(CWidget *new_focused);
	int		DoChildNeedsRepaint(CWidget *child);

	CCombobox *getCombo()  { return cCombo; }

	void setDisplayCount(int _d)  { iDisplayCount = _d; }
	int getDisplayCount()  { return iDisplayCount; }

	void	Readjust();

	static const std::string tagName()	{ return "select"; } // For the skinner we appear to be one widget
	const std::string getTagName()		{ return CComboItemList::tagName(); }
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");

private:
	CCombobox *cCombo;
	CScrollbar *cScrollbar;
	CBorder cBorder;
	CBackground cBackground;
	int iDisplayCount;

	std::list<CComboItem>::iterator	cActiveItem;
	int	iActiveItem;

private:
	void	setActiveItem(const std::list<CComboItem>::iterator& it, int index, bool down);
	int		getItemByCoord(int y);
};

class CCombobox : public CContainerWidget {
public:
	// Constructor
	CCombobox(COMMON_PARAMS);
	~CCombobox();


private:
	// Attributes

	// Items
	std::list<CComboItem> tItems;
	int 			iSelected;
	CComboItem		*cSelectedItem;
	CComboItemList	*cItemList;
	int				iItemHeight;

	// Stuff
	int				iSortDirection;
	bool			bUnique;

	// Appearance
	CBackground cBackground;
	CBorder cBorder;
	CFontStyle cFont;
	CTextProperties cText;

	StyleVar<SmartPointer<SDL_Surface> > bmpExpand;
	StyleVar<Color>		iExpandFace;
	StyleVar<Color>		iExpandLight;
	StyleVar<Color>		iExpandShadow;
	StyleVar<Color>		iExpandArrow;

	CComboItem::CItemStyle cNormalItem;
	CComboItem::CItemStyle cActiveItem;
	CComboItem::CItemStyle cClickedItem;


	// Events
	DECLARE_EVENT(OnChange, ComboChangeHandler);

private:
	CComboItem* getItemRW(int index);
	void	Sort(bool ascending);
	void	Unique();
	std::list<CComboItem>::iterator lowerBound(const CComboItem& item, int *index, bool *equal);
	std::list<CComboItem>::iterator upperBound(const CComboItem& item, int *index, bool *equal);

	friend class CComboItemList;
	void OnItemListClose();

	void UpdateSize();

	void LoadItemStyle(const CSSParser& css, CComboItem::CItemStyle& style, const std::string& id, const std::string& cl, const std::string& psclass);

public:
	// Events
	EVENT_SETGET(OnChange, ComboChangeHandler)

	// Methods

	// Events
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		DoLoseFocus(CWidget *new_focused);
	void	DoRepaint();
	int		DoCreate();

	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyCSS(CSSParser& css);
	void	ApplyTag(xmlNodePtr node);

    void    Clear();
	int		AddItem(const std::string& name);
	int		AddItem(const std::string& sindex, const std::string& name);
	int		AddItem(int tag, const std::string& sindex, const std::string& name);
	int		AddItem(const CComboItem& item);


	CComboItemList *getItemList()	{ return cItemList; }
	const std::list<CComboItem>& getItems()	{ return tItems; }
	const CComboItem* getItem(int index) const;
	int getItemIndex(const CComboItem* item);	
	int		getItemsCount();
	const CComboItem* getItem(const std::string& name) const;
	const CComboItem* getSIndexItem(const std::string& sIndex) const;
	int		getIndexBySIndex(const std::string& szName);
	int		getIndexByName(const std::string& szName);
	void	setCurItem(int index);
	void	setCurItem(const CComboItem* item);
    void    setCurSIndexItem(const std::string& szString);
    void    setCurItemByName(const std::string& szString);
    void	selectNext();
    void	selectPrev();
    int		findItem(UnicodeChar startLetter);
	void	setImage(SmartPointer<SDL_Surface> img, int ItemIndex);
	int		getSelectedIndex();
	const CComboItem* getSelectedItem();
	bool	getDropped() { return cItemList != NULL; }
	void	setSorted(int sort_direction);
	int		getSorted();
	void	setUnique(bool _u);
	bool	getUnique();
	int		getItemHeight()  { return iItemHeight; }
	static const std::string tagName()	{ return "select"; }
	const std::string getTagName()	{ return CCombobox::tagName(); }
	
	const CComboItem* getLastItem();


	//static CWidget * WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );
	//void	ProcessGuiSkinEvent(int iEvent);
};

}; // namespace SkinnedGUI

#endif  //  __CCOMBOBOX_H__SKINNED_GUI__
