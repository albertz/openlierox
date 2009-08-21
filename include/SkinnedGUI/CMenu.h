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


#ifndef __CMENU_H__SKINNED_GUI__
#define __CMENU_H__SKINNED_GUI__

#include <list>
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CItem.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CBackground.h"


namespace SkinnedGUI {

class CMenu;
class CMenuItem;
typedef void(CWidget::*MenuClickItemHandler)(CMenu *sender, CMenuItem *item);
typedef void(CWidget::*MenuCloseHandler)(CMenu *sender);
#define SET_MNUSELECT(menu, func)	SET_EVENT(menu, OnItemClick, MenuClickItemHandler, func)
#define SET_MNUCLOSE(menu, func)	SET_EVENT(menu, OnClose, MenuCloseHandler, func)

// Menu item class
class CMenuItem : public CItem  {
public:
	CMenuItem(CMenu *parent, const std::string& name, bool checkable = false) : CItem(name)
	{ bCheckable = checkable; bEnabled = true; }

private:
	bool	bCheckable;
	bool	bChecked;
	bool	bEnabled;
	CMenu *cParent;

public:
	CItemStyle cDisabledStyle;

public:
	CItem *Clone() const  {
		CMenuItem *res = new CMenuItem(cParent, sName, bCheckable);
		CopyInfoTo(*res);
		res->cParent = cParent;
		res->bCheckable = bCheckable;
		res->bChecked = bChecked;
		res->bEnabled = bEnabled;
		res->cDisabledStyle = cDisabledStyle;

		return res;
	}

	void Draw(SDL_Surface *bmpDest, const SDL_Rect& r);
	void RepaintParent();
	int getWidth();
	int getHeight();
	CItemStyle *getCurrentStyle();
	void setParent(CMenu *p)	{ cParent = p; }
	CMenu *getParent()			{ return cParent; }
	bool isCheckable()			{ return bCheckable; }
	void setCheckable(bool _c)	{ bCheckable = _c; }
	bool isChecked()			{ return bChecked; }
	void setChecked(bool _c)	{ bChecked = _c; RepaintParent(); }
	bool isEnabled()			{ return bEnabled; }
	void setEnabled(bool _e)	{ bEnabled = _e; }
	void ApplyTag(xmlNodePtr node);
};


// Menu class
class CMenu : public CWidget {
private:

	friend class CMenuItem;

    // Attributes
	StyleVar<SmartPointer<SDL_Surface> > bmpCheck;
	StyleVar<SmartPointer<SDL_Surface> > bmpCheckBg;
	CBorder			cBorder;
	CBackground		cBackground;
	int				iSelected;
	CMenuItem		*tSelected;

	CMenuItem::CItemStyle	cNormalItem;
	CMenuItem::CItemStyle	cActiveItem;
	CMenuItem::CItemStyle	cClickedItem;
	CMenuItem::CItemStyle	cDisabledItem;

	std::list<CMenuItem *>  tItems;
	DECLARE_EVENT(OnClose, MenuCloseHandler);
	DECLARE_EVENT(OnItemClick, MenuClickItemHandler);

	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);

	void	DoRepaint();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyCSS(CSSParser& css);
	void	LoadItemStyle(CMenuItem::CItemStyle& style, const CSSParser& css, const std::string& id, const std::string& cl, const std::string& pscl);
	void	StyleItem(CMenuItem& it);
	void	ApplyTag(xmlNodePtr node);

	void	setSelected(int index);

public:
	// Events
	EVENT_SETGET(OnClose, MenuCloseHandler)
	EVENT_SETGET(OnItemClick, MenuClickItemHandler)

    // Methods
	CMenu(COMMON_PARAMS);
	CMenu(COMMON_PARAMS, int x, int y);
	~CMenu();


	void	AddItem(const CMenuItem& item);
	void	AddItem(const std::string& name, int tag = 0, bool checked = false);
	CMenuItem *getItem(int index);

	static const std::string tagName()		{ return "menu"; }
	const std::string getTagName()			{ return CMenu::tagName(); }

};

}; // namespace SkinnedGUI

#endif  //  __CMENU_H__SKINNED_GUI__
