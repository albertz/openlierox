/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Text box
// Created 30/6/02
// Jason Boettcher


#ifndef __CTEXTBOX_H__
#define __CTEXTBOX_H__


#define		MAX_TEXTLENGTH		256

// Event types
enum {
	TXT_NONE=-1,
	TXT_CHANGE=0,
	TXT_MOUSEOVER,
	TXT_ENTER
};


// Messages
enum {
	TXM_GETTEXT=0,
	TXM_SETTEXT,
	TXM_SETFLAGS,
	TXM_SETMAX,
	TXM_GETTEXTLENGTH
};


// Flags
#define		TXF_PASSWORD	0x0001


class CTextbox : public CWidget {
public:
	// Constructor
	CTextbox() {
		Create();
		iType = wid_Textbox;
		iFlags = 0;
		fBlinkTime = 0;
		iDrawCursor = 1;
		iScrollPos = 0;
		iSelLength = 0;
		iSelStart = 0;
		sSelectedText[0] = '\0';
		iHoldingMouse = false;
		iLastCurpos = 0;
		fTimeHolding = 0;
		iLastMouseX = 0;
		fScrollTime = 0;  // We can scroll
	}


private:
	// Attributes

	char	sText[MAX_TEXTLENGTH];

	int		iScrollPos;
	int		iCurpos;
	int		iLength;
	int		iFlags;
	int		iSelLength;
	int		iSelStart;
	char	sSelectedText[MAX_TEXTLENGTH];

	int		iMax;

	int		iHolding;
	float	fTimePushed;
	int		iLastchar;

	int		iHoldingMouse;
	float	fTimeHolding;
	int		iLastCurpos;
	int		iLastMouseX;
	float	fScrollTime;

	float	fBlinkTime;
	int		iDrawCursor;


public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return TXT_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return TXT_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return TXT_NONE; }
	int		KeyDown(int c);
	int		KeyUp(int c);

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2);

	void	Backspace(void);
	void	Delete(void);
	void	Insert(char c);

	char	*getText(void)						{ return sText; }
	void	setText(char *buf);

    void    PasteText(void);
	void	CopyText(void);


};




#endif  //  __CTEXTBOX_H__
