/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Marquee
// Created 20/4/08
// Karel Petranek

#ifndef __CMARQUEE_H__SKINNED_GUI__
#define __CMARQUEE_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "Timer.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CBackground.h"
#include "FontHandling.h"


namespace SkinnedGUI {

static const int MARQUEE_STEP = 2; // By how many pixels it moves
static const float MARQUEE_TIME = 0.10f; // How often it moves
static const float MARQUEE_ENDWAIT = 0.2f;  // How long it waits when it reaches end

class CMarquee : public CWidget  {
public:
	CMarquee(COMMON_PARAMS);
	CMarquee(COMMON_PARAMS, const std::string& text);
	~CMarquee();

private:
	std::string sText;
	int		iFrame;
	int		iDirection;  // 1 = right, -1 = left
	int		iTextWidth;
	bool	bAtEnd;
	Timer	*tTimer;

	CFontStyle cFont;
	CBorder cBorder;
	CBackground cBackground;
	CTextProperties cText;

	void DoRepaint();
	int DoCreate();
	void AutoSize();
	void OnTimer(Timer::EventData ev);
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);

public:
	void	Resize(int x, int y, int w, int h);

	const std::string& getText(void)	{ return sText; }
	void setText(const std::string& text);

	static const std::string tagName()	{ return "marquee"; }
	const std::string getTagName()		{ return CMarquee::tagName(); }
};

}; // namespace SkinnedGUI

#endif // __CMARQUEE_H__SKINNED_GUI__

