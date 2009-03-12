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


#ifndef __CCOMBOBOX_H__DEPRECATED_GUI__
#define __CCOMBOBOX_H__DEPRECATED_GUI__

#include <list>

#include "StringUtils.h"
#include "InputEvents.h"
#include "DeprecatedGUI/CScrollbar.h"

namespace DeprecatedGUI {

// Combo box events
enum {
	CMB_NONE=-1,
	CMB_CHANGED
};

// Combobox messages
enum {
	CBS_ADDITEM,
	CBS_ADDSITEM,
	CBM_GETCOUNT,
	CBM_GETCURINDEX,
	CBS_GETCURSINDEX,
	CBM_GETCURITEM,
    CBS_GETCURNAME,
	CBM_CLEAR,
	CBM_SETCURSEL,
    CBS_SETCURSINDEX,
    CBM_SETCURINDEX,
	CBM_ISDROPPED,
	CBM_SETSORTED,
	CBM_SETUNIQUE
};

// Sorting directions
enum  {
	SORT_DESC = -1,
	SORT_NONE = 0,
	SORT_ASC = 1
};


// Item structure
class cb_item_t { public:
	std::string	sIndex;
	std::string	sName;
	SmartPointer<SDL_Surface> tImage;
	int iTag;
};



class CCombobox : public CWidget {
public:
	// Constructor
	CCombobox() {
		iSelected = 0;
		bGotScrollbar = false;
		bDropped = false;
		bArrowDown = false;
        bLastDropped = false;
		iDropTime = Time();
		iNow = Time();
		bCanSearch = true;
		iKeySelectedItem = -1;
		iSortDirection = SORT_NONE;
		bUnique = false;
		iType = wid_Combobox;
		iVar = NULL;
		sVar = NULL;
	}


private:
	// Attributes

	// Items
	std::list<cb_item_t> tItems;
	int 			iSelected;
	bool			bGotScrollbar;
	bool			bDropped;
	bool			bArrowDown;
    bool			bLastDropped;
	Time			iDropTime;
	Time			iNow;
	int				iKeySelectedItem;

	// Stuff
	bool			bCanSearch;
	int				iSortDirection;
	bool			bUnique;

	// Scrollbar
	CScrollbar		cScrollbar;

	int				*iVar;
	std::string		*sVar;
	CGuiSkin::CallbackHandler cClick;

private:
	cb_item_t* getItemRW(int index);
	void	Sort(bool ascending);
	void	Unique();
	std::list<cb_item_t>::iterator lowerBound(const cb_item_t& item, int *index, bool *equal);
	std::list<cb_item_t>::iterator upperBound(const cb_item_t& item, int *index, bool *equal);

public:
	// Methods

	void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ bCanSearch = true; return CMB_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);

	void	Draw(SDL_Surface * bmpDest);
	void	LoadStyle(void) {}
	
	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param);

    void    clear(void);
	int		addItem(const std::string& sindex, const std::string& name, const SmartPointer<SDL_Surface> img = NULL, int tag = 0);
	int		addItem(int index, const std::string& sindex, const std::string& name, const SmartPointer<SDL_Surface> img = NULL, int tag = 0);
	const std::list<cb_item_t>& getItems()	{ return tItems; }
	const cb_item_t* getItem(int index) const;
	int getItemIndex(const cb_item_t* item);	
	int		getItemsCount();
	const cb_item_t* getItem(const std::string& name) const;
	const cb_item_t* getSIndexItem(const std::string& sIndex) const;
	int getIndexBySIndex(const std::string& szName);
	int getIndexByName(const std::string& szName);
	void	setCurItem(int index);
	void	setCurItem(const cb_item_t* item);
    void    setCurSIndexItem(const std::string& szString);
    void    setCurItemByName(const std::string& szString);
    bool	selectNext();
    bool	selectPrev();
    int		findItem(UnicodeChar startLetter);
	void	setImage(SmartPointer<SDL_Surface> img, int ItemIndex);
	int		getSelectedIndex();
	const cb_item_t* getSelectedItem();
	bool	getDropped(void) { return bDropped; }
	void	setSorted(int sort_direction);
	int		getSorted();
	void	setUnique(bool _u);
	bool	getUnique();
	int getItemHeight();
	
	const cb_item_t* getLastItem();

	static CWidget * WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );
	void	ProcessGuiSkinEvent(int iEvent);
};

} // namespace DeprecatedGUI

#endif  //  __CCOMBOBOX_H__DEPRECATED_GUI__
