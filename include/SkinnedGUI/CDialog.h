/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Dialog class
// Created 30/5/08
// Karel Petranek


#ifndef __CDIALOG_H__SKINNED_GUI__
#define __CDIALOG_H__SKINNED_GUI__

#include "SkinnedGUI/CGuiSkinnedLayout.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CBackground.h"
#include "FontHandling.h"
#include "SmartPointer.h"


namespace SkinnedGUI {

class CDialog;

class CTitleBar : public CWidget
{
public:
	// Constructor
	CTitleBar(CDialog *parent, const std::string& text);


private:
	// Attributes
	std::string sText;
	bool bGrabbed;
	CBackground cBackground;
	CBorder cBorder;
	CFontStyle cFont;
	CTextProperties cText;
	StyleVar<SmartPointer<SDL_Surface> > bmpLeft;
	StyleVar<SmartPointer<SDL_Surface> > bmpRight;
	StyleVar<SmartPointer<SDL_Surface> > bmpMain;

	// Events
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoParentResize(int& new_w, int& new_h);
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");

public:

	// Draw the title bar
	void	DoRepaint();

	void	setText(const std::string& text)	{ sText = text; Repaint(); }
	const std::string& getText()				{ return sText; }

	static const std::string tagName()	{ return ""; }
	const std::string getTagName()	{ return CTitleBar::tagName(); }
};

// Abstract dialog class
class CDialog : public CGuiSkinnedLayout  {
public:
	CDialog(COMMON_PARAMS);
	CDialog(COMMON_PARAMS, const std::string& title, bool modal = true, bool moveable = true, bool resizable = false);
	virtual ~CDialog();

private:
	StyleVar<bool> bMoveable;
	StyleVar<bool> bResizable;
	CTitleBar *cTitle;

protected:
	virtual void RecalculateClientRect();

public:
	// Methods
	virtual void	DoRepaint();

	virtual void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	virtual void ApplyTag(xmlNodePtr node);

	void MoveBy(int dx, int dy);
	void MoveTo(int x, int y);
	void Resize(int x, int y, int w, int h);

	virtual void Close();

	bool	isMoveable()	{ return bMoveable; }
	bool	isResizable()	{ return bResizable; }

	void	setEnabled(bool _e);
	void	setVisible(bool _v);

	static const std::string tagName()	{ return "dialog"; }
	const std::string getTagName()	{ return CDialog::tagName(); }
};

}; // namespace SkinnedGUI

#endif // __CDIALOG_H__SKINNED_GUI__
