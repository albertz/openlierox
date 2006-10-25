/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
	}

	CButton(int imgid, SDL_Surface *image) {
		iImageID = imgid;
		bmpImage = image;
		iMouseOver = false;
		iType = wid_Button;
        iGoodWidth = 250;
	}


private:
	// Attributes

	int			iMouseOver;
	SDL_Surface	*bmpImage;
	int			iImageID;
    int         iGoodWidth;

public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ iMouseOver=true; return BTN_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return BTN_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ iMouseOver=true; return BTN_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return BTN_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return BTN_NONE; }
	int		KeyDown(int c)						{ return BTN_NONE; }
	int		KeyUp(int c)						{ return BTN_NONE; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }

	// Draw the button
	void	Draw(SDL_Surface *bmpDest);
    void	Draw2(SDL_Surface *bmpDest);



};




#endif  //  __CBUTTON_H__
