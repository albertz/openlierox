/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Button
// Created 30/6/02
// Jason Boettcher


#ifndef __CBUTTON_H__
#define __CBUTTON_H__


// Event types
enum {
	BTN_NONE=-1,
	BTN_MOUSEUP=0,
	BTN_MOUSEOVER
};


class CButton : public CWidget {
public:
	// Constructor
	CButton() {
		iMouseOver = false;
		iType = wid_Button;
        iGoodWidth = 250;
		bRedrawMenu = true;
		bFreeSurface = false;
	}

	CButton(int imgid, SDL_Surface *image) {
		iImageID = imgid;
		bmpImage = image;
		iMouseOver = false;
		bRedrawMenu = true;
		iType = wid_Button;
        iGoodWidth = 250;
		bFreeSurface = false;
	}

	CButton(const std::string& path) {
		iImageID = 0;
		bmpImage = LoadImage(path,1);
		iMouseOver = false;
		bRedrawMenu = true;
		iType = wid_Button;
        iGoodWidth = 250;
		bFreeSurface = true;
	}


private:
	// Attributes

	int			iMouseOver;
	SDL_Surface	*bmpImage;
	int			iImageID;
    int         iGoodWidth;
	bool		bFreeSurface;
	bool		bRedrawMenu;

public:
	// Methods

	void	Create(void);
	void	Destroy(void) { if(bFreeSurface && bmpImage) SDL_FreeSurface(bmpImage); }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ iMouseOver=true; return BTN_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return BTN_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ iMouseOver=true; return BTN_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return BTN_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return BTN_NONE; }
	int		KeyDown(int c)						{ return BTN_NONE; }
	int		KeyUp(int c)						{ return BTN_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }

	inline void	setRedrawMenu(bool _r)  { bRedrawMenu = _r; }
	inline bool	getRedrawMenu(void)	 { return bRedrawMenu; }

	// Draw the button
	void	Draw(SDL_Surface *bmpDest);
    void	Draw2(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}



};




#endif  //  __CBUTTON_H__
