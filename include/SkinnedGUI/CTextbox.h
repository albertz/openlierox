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


#ifndef __CTEXTBOX_H__SKINNED_GUI__
#define __CTEXTBOX_H__SKINNED_GUI__


#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CBackground.h"
#include "FontHandling.h"
#include "Timer.h"


namespace SkinnedGUI {

class CTextbox;
typedef void(CWidget::*TextboxChangeHandler)(CTextbox *sender, const std::string& newtext, bool& cancel);
typedef void(CWidget::*TextboxEnterPressHandler)(CTextbox *sender);
#define SET_TXTCHANGE(textbox, func)		SET_EVENT(textbox, OnChange, TextboxChangeHandler, func)
#define SET_TXTENTERPRESS(textbox, func)	SET_EVENT(textbox, OnEnterPress, TextboxEnterPressHandler, func)


class CTextbox : public CWidget {
public:
	// Constructor
	CTextbox(COMMON_PARAMS);
	~CTextbox();


private:


	// Attributes

	std::string	sText;
	std::string sAltKey;  // if user is pressing Alt + Numbers, we remember the numbers and insert unicode character

	// these are related to the size of the string (sText.size()), NOT the displayed size
	size_t	iScrollPos;
	size_t	iCurpos;
	int		iSelLength; // if < 0, selection to the left, otherwise to the right
	size_t	iSelStart;
	
	StyleVar<bool>	bPassword;
	StyleVar<bool>	bNoUnicode;
	std::string	sSelectedText;

	int		iMax;

	bool	bHolding;
	AbsTime	fTimePushed;
	UnicodeChar		iLastchar;
	int		iLastKeysym;

	size_t	iLastCurpos;
	TimeDiff	fScrollTime;
	AbsTime	fLastRepeat;
	AbsTime	fLastClick;

	bool	bDrawCursor;

	Timer	*tTimer;

	CBorder cBorder;
	CBackground cBackground;
	CFontStyle cFont;
	CTextProperties cText;
	StyleVar<Color> clSelection;
	StyleVar<Color> clCaret;

private:

	void	Backspace();
	void	Delete();
	void	SelectWord();
	void	Insert(UnicodeChar c);
	void OnTimerEvent(Timer::EventData ev);

	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		DoKeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate);
	void	DoRepaint();
	int		DoCreate();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);

	void	UpdateSize();
	void	SetCursorPosByCoord(int x, int y);

	DECLARE_EVENT(OnChange, TextboxChangeHandler);
	DECLARE_EVENT(OnEnterPress, TextboxEnterPressHandler);

public:
	// Events
	EVENT_SETGET(OnChange, TextboxChangeHandler)
	EVENT_SETGET(OnEnterPress, TextboxEnterPressHandler)

	// Methods

	std::string	getText()						{ return sText; }
	void	setText(const std::string& buf);
	void	setPassword(bool _p)				{ bPassword.set(_p, HIGHEST_PRIORITY); }
	bool	isPassword()						{ return bPassword; }
	void	setNoUnicode(bool _u)				{ bNoUnicode.set(_u, HIGHEST_PRIORITY); }
	bool	getNoUnicode()						{ return bNoUnicode; }
	void	setMax(int _m)  { iMax = _m; }
	int		getMax()		{ return iMax; }

    void    PasteText();
	void	CopyText();

	static const std::string tagName()			{ return "textbox"; }
	const std::string getTagName()				{ return CTextbox::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CTEXTBOX_H__SKINNED_GUI__
