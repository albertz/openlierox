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


#ifndef __CTEXTBOX_H__DEPRECATED_GUI__
#define __CTEXTBOX_H__DEPRECATED_GUI__


#include "InputEvents.h"
#include "Cursor.h"
#include "Timer.h"
#include "Event.h"


namespace DeprecatedGUI {

// Event types
enum {
	TXT_NONE=-1,
	TXT_CHANGE=0,
	TXT_MOUSEOVER,
	TXT_ENTER,
	TXT_TAB
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
enum { 
	TXF_PASSWORD = 0x0001, 
	TXF_NOUNICODE = 0x0002 
};


class CTextbox : public CWidget {
public:
	// Constructor
	CTextbox() {
		iType = wid_Textbox;
		iFlags = 0;
		bDrawCursor = true;
		iScrollPos = 0;
		iSelLength = 0;
		iSelStart = 0;
		sSelectedText = "";
		bHolding = false;
		bHoldingMouse = false;
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
		tTimer = NULL;
		Create();
	}

	~CTextbox()  {
		Destroy();
	}


private:
	// Attributes

	std::string	sText;
	std::string sAltKey;  // if user is pressing Alt + Numbers, we remember the numbers and insert unicode character

	// these are related to the size of the string (sText.size()), NOT the displayed size
	size_t	iScrollPos;
	size_t	iCurpos;
	int		iSelLength; // if < 0, selection to the left, otherwise to the right
	size_t	iSelStart;
	
	Uint32	iFlags;
	std::string	sSelectedText;

	size_t	iMax;

	bool	bHolding;
	float	fTimePushed;
	UnicodeChar		iLastchar;
	int		iLastKeysym;

	bool	bHoldingMouse;
	float	fTimeHolding;
	size_t	iLastCurpos;
	int		iLastMouseX;
	float	fScrollTime;
	float	fLastRepeat;
	float	fLastClick;

	bool	bDrawCursor;

	Timer	*tTimer;

	bool		*bVar;
	int			*iVar;
	float		*fVar;
	std::string	*sVar;
	CGuiSkin::CallbackHandler cClick;


public:
	// Methods

	void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ SetGameCursor(CURSOR_TEXT); return TXT_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return TXT_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return TXT_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate);

	void	Draw(SDL_Surface * bmpDest);

	void	LoadStyle(void) {}

	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD	SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD	SendMessage(int iMsg, std::string *sStr, DWORD Param);

	void	Backspace(void);
	void	Delete(void);
	void	SelectWord(void);
	void	Insert(UnicodeChar c);

	std::string	getText(void)						{ return sText; }
	void	setText(const std::string& buf);
	void	setFlag(Uint32 flag) { iFlags |= flag; }
	void	unsetFlag(Uint32 flag) { iFlags &= ~flag; }

    void    PasteText(void);
	void	CopyText(void);

	void OnTimerEvent(Timer::EventData ev);

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );
	void	ProcessGuiSkinEvent(int iEvent);
};

}; // namespace DeprecatedGUI

#endif  //  __CTEXTBOX_H__DEPRECATED_GUI__
