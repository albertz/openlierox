/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Input box
// Created 31/7/02
// Jason Boettcher


#ifndef __CINPUTBOX_H__SKINNED_GUI__
#define __CINPUTBOX_H__SKINNED_GUI__

#include "SkinnedGUI/CDialog.h"


namespace SkinnedGUI {

class CInputboxDialog;

class CInputbox : public CWidget {
public:
	// Constructor
	CInputbox(COMMON_PARAMS);
	CInputbox(COMMON_PARAMS, int val, const std::string& _text);


private:
	// Attributes

	int			iKeyvalue;
	std::string	sText;
	CBorder		cBorder;
	CBackground	cBackground;
	CFontStyle	cFont;
	CInputboxDialog *cDialog;

	// Events
	int	DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix);
	void ApplyTag(xmlNodePtr node);

public:
	// Publish some of the default events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnMouseDown, MouseHandler)

	// Methods
	void	DoRepaint();


	int		getValue(void)					{ return iKeyvalue; }
	void	setValue(int _v)				{ iKeyvalue = _v; Repaint(); }
	std::string	getText(void)				{ return sText; }
	void	setText(const std::string& _t)	{ sText = _t; Repaint(); }

	static const std::string tagName()		{ return "inputbox"; }
	const std::string getTagName()			{ return CInputbox::tagName(); }
};

// The dialog that appears after clicking on the inputbox
class CInputboxDialog: public CDialog
{
private:
	CInputbox *cInputBox;
	CInput *cInput;

	int	DoKeyUp(UnicodeChar c, int keysym,  const ModifiersState& modstate);
public:
	CInputboxDialog(CInputbox *inpb);
	~CInputboxDialog();
	
	void Process();
	static const std::string tagName()	{ return ""; }
	const std::string getTagName()		{ return CInputboxDialog::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CINPUTBOX_H__SKINNED_GUI__
