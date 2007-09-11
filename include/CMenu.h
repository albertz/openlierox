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


#ifndef __CMENU_H__
#define __CMENU_H__


#include "InputEvents.h"


// Event types
enum {
	MNU_NONE=-1,
	MNU_LOSTFOCUS=0,
	MNU_USER        // Must be last
};


// Messages
enum {
	MNS_ADDITEM,
    MNM_REDRAWBUFFER
};


// Menu item structure
// TODO: use std::vector/list
class mnu_item_t { public:
    int     nID;
	std::string  szName;
    int     nSelected;

    mnu_item_t   *psNext;

};


class CMenu : public CWidget {
private:
    // Attributes

    int     m_nPosX, m_nPosY;
    int     m_nWidth, m_nHeight;

    mnu_item_t  *m_psItemList;


public:
    // Methods

    CMenu(int nPosX, int nPosY);

    void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return MNU_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return MNU_NONE; }
	int		KeyDown(UnicodeChar c, int keysym)	{ return MNU_NONE; }
	int		KeyUp(UnicodeChar c, int keysym)	{ return MNU_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }
    
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}


    void    addItem(int nID, const std::string& szName);

};


#endif  //  __CMENU_H__


