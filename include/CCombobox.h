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


#ifndef __CCOMBOBOX_H__
#define __CCOMBOBOX_H__

#include <vector>

#include "StringUtils.h"
#include "InputEvents.h"
#include "CScrollbar.h"


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
	CBM_SETIMAGE,
	CBM_ISDROPPED,
	CBM_SETSORTED,
	CBM_SETUNIQUE
};



// Item structure
class cb_item_t { public:
	std::string	sIndex;
	std::string	sName;
	SDL_Surface *tImage;
};



class CCombobox : public CWidget {
public:
	// Constructor
	CCombobox() {
		iSelected = 0;
		iGotScrollbar = false;
		iDropped = false;
		iArrowDown = false;
        iLastDropped = false;
		iDropTime = 0;
		iNow = 0;
		iCanSearch = true;
		iKeySelectedItem = -1;
		bSorted = false;
		bUnique = false;
		iType = wid_Combobox;
		iVar = NULL;
		sVar = NULL;
	}


private:
	// Attributes

	// Items
	std::vector<cb_item_t> tItems;
	int 			iSelected;
	int				iGotScrollbar;
	int				iDropped;
	int				iArrowDown;
    int             iLastDropped;
	int				iDropTime;
	int				iNow;
	int				iKeySelectedItem;

	// Stuff
	int				iCanSearch;
	bool			bSorted;
	bool			bUnique;

	// Scrollbar
	CScrollbar		cScrollbar;

	int				*iVar;
	std::string		*sVar;
	CGuiSkin::CallbackHandler cClick;

private:
	cb_item_t* getItemRW(int index);

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
	int		KeyUp(UnicodeChar c, int keysym)	{ iCanSearch = true; return CMB_NONE; }
	int		KeyDown(UnicodeChar c, int keysym);

	void	Draw(SDL_Surface *bmpDest);
	void	LoadStyle(void) {}

	void	Sort(bool ascending);

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param);

    void    clear(void);
	int		addItem(const std::string& sindex, const std::string& name);
	int		addItem(int index, const std::string& sindex, const std::string& name);
	const std::vector<cb_item_t>& getItems()	{ return tItems; }
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
	void	setImage(SDL_Surface *img, int ItemIndex);
	int		getSelectedIndex();
	const cb_item_t* getSelectedItem();
	int		getDropped(void) { return iDropped; }
	void	setSorted(bool _s)  { bSorted = _s; }
	bool	getSorted()	{ return bSorted; }
	void	setUnique(bool _u)  { bUnique = _u; }
	bool	getUnique()			{ return bUnique; }
	int getItemHeight();
	
	const cb_item_t* getLastItem();

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );
	void	ProcessGuiSkinEvent(int iEvent);
};

#endif  //  __CCOMBOBOX_H__
