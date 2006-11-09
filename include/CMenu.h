/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Menu
// Created 28/6/03
// Jason Boettcher


#ifndef __CMENU_H__
#define __CMENU_H__


// Event types
enum {
	MNU_NONE=-1,
	MNU_LOSTFOCUS=0,
	MNU_USER        // Must be last
};


// Messages
enum {
	MNM_ADDITEM,
    MNM_REDRAWBUFFER
};


// Menu item structure
typedef struct mnu_item_s {
    int     nID;
    char    szName[64];
    int     nSelected;

    struct mnu_item_s   *psNext;

} mnu_item_t;


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
	int		KeyDown(int c)						{ return MNU_NONE; }
	int		KeyUp(int c)						{ return MNU_NONE; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2);
    
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}


    void    addItem(int nID, char *szName);

};


#endif  //  __CMENU_H__


