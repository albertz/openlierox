/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// CBar header file
// Created 22/6/07
// Dark Charlie

#ifndef __CPROGRESSBAR_H__SKINNED_GUI__
#define __CPROGRESSBAR_H__SKINNED_GUI__

#include <SDL.h>
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CLabel.h"


namespace SkinnedGUI {

// Bar directions
enum {
	BAR_LEFTTORIGHT,
	BAR_RIGHTTOLEFT,
	BAR_TOPTOBOTTOM,
	BAR_BOTTOMTOTOP
};

class CProgressBar : public CWidget {
public:
	CProgressBar() : CWidget("", NULL) {} // TODO: Just a compile workaround
	CProgressBar(COMMON_PARAMS, int num_fore_states = 1, int num_bg_states = 1);

private:
	// Variables
	StyleVar<int>  iDirection;
	int  iPosition;
	int  iNumForeStates;
	int	 iNumBgStates;
	int  iCurrentForeState;
	int	 iCurrentBgState;

	CLabel	*cLabel;
	StyleVar<SmartPointer<SDL_Surface> > bmpBar;

	StyleVar<Color>	bgColor;
	StyleVar<Color>	foreColor;

private:
	int CalculateWidth();
	int CalculateHeight();

	int DoCreate();
	void DoRepaint();

	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void ApplyTag(xmlNodePtr node);

public:
	// Methods
	int	getPosition()  { return iPosition; }
	void setPosition(int _p);
	int	getNumForeStates()  { return iNumForeStates; }
	void setNumForeStates(int _s) { iNumForeStates = _s; Repaint(); }  // NOTE: number of states is one state less than count of images
	int	getCurrentForeState()  { return iCurrentForeState; }  //
	void setCurrentForeState(int _s);
	int	getNumBgStates()  { return iNumBgStates; }
	void setNumBgStates(int _s) { iNumBgStates = _s; Repaint(); }  // NOTE: number of states is one state less than count of images
	int	getCurrentBgState()  { return iCurrentBgState; }  //
	void setCurrentBgState(int _s);
	Color getBgColor()  { return bgColor; }
	void setBgColor(Color _cl)  { bgColor.set(_cl, HIGHEST_PRIORITY); Repaint(); }
	Color getForeColor()  { return foreColor; }
	void setForeColor(Color _cl)  { foreColor.set(_cl, HIGHEST_PRIORITY); Repaint(); }
	bool isProperlyLoaded()  { return bmpBar.get().get() != NULL; }

	static const std::string tagName()	{ return "progress"; }
	const std::string getTagName()		{ return CProgressBar::tagName(); }
};

}; // namespace SkinnedGUI

#endif // __CPROGRESSBAR_H__SKINNED_GUI__
