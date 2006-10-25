/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Label
// Created 30/6/02
// Jason Boettcher


#ifndef __CLABEL_H__
#define __CLABEL_H__


// Label events
enum {
	LBL_NONE=-1
};

// Label messages
enum {
	LBM_SETTEXT
};


class CLabel : public CWidget {
public:
	// Constructor
	CLabel(char *text, Uint32 col) {
		strcpy(sText,text);
		iColour = col;
		iType = wid_Label;
	}


private:
	// Attributes

	char	sText[64];
	Uint32	iColour;


public:
	// Methods

	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return LBL_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return LBL_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return LBL_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return LBL_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return LBL_NONE; }
	int		KeyDown(int c)						{ return LBL_NONE; }
	int		KeyUp(int c)						{ return LBL_NONE; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ 
							if(iMsg==LBM_SETTEXT) strcpy(sText, (char *)Param1); return 0;
						}

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest) {
				tLX->cFont.Draw(bmpDest, iX, iY, iColour,"%s", sText); 
	}

};



#endif  //  __CLABEL_H__
