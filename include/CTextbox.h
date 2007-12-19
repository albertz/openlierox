/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Text box
// Created 30/6/02
// Jason Boettcher


#ifndef __CTEXTBOX_H__
#define __CTEXTBOX_H__


#include "InputEvents.h"
#include "Cursor.h"


// Event types
enum {
	TXT_NONE=-1,
	TXT_CHANGE=0,
	TXT_MOUSEOVER,
	TXT_ENTER
};


// Messages
enum {
	TXS_GETTEXT=0,
	TXS_SETTEXT,
	TXM_SETFLAGS,
	TXM_SETMAX,
	TXM_GETTEXTLENGTH
};


// Flags
#define		TXF_PASSWORD	0x0001
#define		TXF_NOUNICODE	0x0002


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
		sSelectedText = "";
		iHoldingMouse = false;
		iLastCurpos = 0;
		fTimeHolding = 0;
		iLastMouseX = 0;
		fLastRepeat = -9999;
		fLastClick = -9999;
		fScrollTime = 0;  // We can scroll
		bVar = NULL;
		iVar = NULL;
		fVar = NULL;
		sVar = NULL;
	}


private:
	// Attributes

	std::string	sText;

	// these are related to the size of the string (sText.size()), NOT the displayed size
	size_t	iScrollPos;
	size_t	iCurpos;
	int		iSelLength; // if < 0, selection to the left, otherwise to the right
	size_t	iSelStart;
	
	Uint32	iFlags;
	std::string	sSelectedText;

	size_t	iMax;

	int		iHolding;
	float	fTimePushed;
	UnicodeChar		iLastchar;
	int		iLastKeysym;

	int		iHoldingMouse;
	float	fTimeHolding;
	int		iLastCurpos;
	int		iLastMouseX;
	float	fScrollTime;
	float	fLastRepeat;
	float	fLastClick;

	float	fBlinkTime;
	int		iDrawCursor;

	bool		*bVar;
	int			*iVar;
	float		*fVar;
	std::string	*sVar;
	CGuiSkin::CallbackHandler cClick;

public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ SetGameCursor(CURSOR_TEXT); return TXT_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return TXT_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return TXT_NONE; }
	int		KeyDown(UnicodeChar c, int keysym);
	int		KeyUp(UnicodeChar c, int keysym);

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param);

	void	Backspace(void);
	void	Delete(void);
	void	SelectWord(void);
	void	Insert(UnicodeChar c);

	std::string	getText(void)						{ return sText; }
	void	setText(const std::string& buf);
	inline void setFlag(Uint32 flag) { iFlags |= flag; }
	inline void unsetFlag(Uint32 flag) { iFlags &= ~flag; }

    void    PasteText(void);
	void	CopyText(void);

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );
	void	ProcessGuiSkinEvent(int iEvent);
};




#endif  //  __CTEXTBOX_H__
