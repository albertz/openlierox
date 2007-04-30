/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
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
	LBS_SETTEXT
};


class CLabel : public CWidget {
public:
	// Constructor
	CLabel(const tString& text, Uint32 col) {
		sText = text;
		iColour = col;
		iType = wid_Label;
	}


private:
	// Attributes

	tString	sText;
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

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const tString& sStr, DWORD Param) { if (iMsg == LBS_SETTEXT) {sText = sStr;} return 0; }
	DWORD SendMessage(int iMsg, tString *sStr, DWORD Param)  { return 0; }

	void	ChangeColour(Uint32 col)			{ iColour = col; }

	// Draw the label
	void	Draw(SDL_Surface *bmpDest) {
				tLX->cFont.Draw(bmpDest, iX, iY, iColour,sText); 
	}

	void	LoadStyle(void) {}

};



#endif  //  __CLABEL_H__
