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
	CBM_ISDROPPED
};


// Item structure
class cb_item_t { public:
	int			iIndex;
	std::string	sIndex;
	std::string	sName;
	int			iSelected;
	SDL_Surface *tImage;

	cb_item_t	*tNext;
};



class CCombobox : public CWidget {
public:
	// Constructor
	CCombobox() {
		tItems = NULL;
		tSelected = NULL;
		iItemCount = 0;
		iGotScrollbar = false;
		iDropped = false;
		iArrowDown = false;
        iLastDropped = false;
		iDropTime = 0;
		iNow = 0;
		iCanSearch = true;
		iKeySelectedItem = -1;

	}


private:
	// Attributes

	// Items
	cb_item_t		*tItems;
	cb_item_t		*tSelected;
	int				iItemCount;
	int				iGotScrollbar;
	int				iDropped;
	int				iArrowDown;
    int             iLastDropped;
	int				iDropTime;
	int				iNow;
	int				iKeySelectedItem;

	// Stuff
	int				iCanSearch;

	// Scrollbar
	CScrollbar		cScrollbar;


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
	int		KeyUp(UnicodeChar c)					{ iCanSearch = true; return CMB_NONE; }
	int		KeyDown(UnicodeChar c);

	void	Draw(SDL_Surface *bmpDest);
	void	LoadStyle(void) {}

	void	Sort(bool ascending);

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param);

    void    clear(void);
	void	addItem(int index, const std::string& sindex, const std::string& name);
	inline cb_item_t* getItems()	{ return tItems; }
	cb_item_t* getItem(int index);
	int		getItemsCount();
	cb_item_t* getItem(const std::string& name);
	void	setCurItem(int index);
    void    setCurSIndexItem(const std::string& szString);
    void    setCurIndexItem(int nIndex);
	void	setImage(SDL_Surface *img, int ItemIndex);
	int		getDropped(void) { return iDropped; }
	int		getSelectedIndex(void);
};


#endif  //  __CCOMBOBOX_H__
