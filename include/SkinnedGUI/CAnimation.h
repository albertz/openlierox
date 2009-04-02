// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CANIMATION_H__SKINNED_GUI__
#define __CANIMATION_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "Timer.h"
#include "SkinnedGUI/CBorder.h"


namespace SkinnedGUI {

class CAnimation;
typedef void(CWidget::*ImageClickHandler)(CAnimation *sender, MouseButton button, const ModifiersState& modstate);
#define SET_IMGCLICK(image, func)	SET_EVENT(image, OnClick, ImageClickHandler, func)

class CAnimation : public CWidget {
public:
	// Constructor
	CAnimation(COMMON_PARAMS);
	CAnimation(COMMON_PARAMS, const std::string& Path);
	CAnimation(COMMON_PARAMS, SmartPointer<SDL_Surface> img);
	~CAnimation();

private:
    // Attributes
	SmartPointer<SDL_Surface>	tImage;
	std::string	sPath;
	std::vector<int> tFrameOffsets;
	std::vector<int> tFrameWidths;
	int			iNumFrames;
	int			iCurFrame;
	bool		bAnimated;
	AbsTime		fLastFrameChange;
	float		fFrameTime;
	CBorder		cBorder;
	Timer		*tTimer;

	DECLARE_EVENT(OnClick, ImageClickHandler);

	void		ParseAnimation();
	void		DoRepaint();
	int			DoCreate();
	void	OnTimer(Timer::EventData ev);

	int			DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);

	void		AutoSize();
	void		ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void		ApplyTag(xmlNodePtr node);

public:
	// Publish some of the default events
	EVENT_SETGET(OnClick, ImageClickHandler);

    // Methods

	std::string		getPath()		{ return sPath; }
	SmartPointer<SDL_Surface>	getSurface()	{ return tImage; }
	float			getFrameTime()		{ return fFrameTime; }
	void			setFrameTime(float t);
	void			Change(const std::string& Path);
	void			incFrame()			{ iCurFrame++; iCurFrame %= iNumFrames; Repaint(); }
	void			setFrame(int f)		{ iCurFrame = f; iCurFrame %= iNumFrames; Repaint(); }
	int				getFrame()			{ return iCurFrame; }
	int				getNumFrames()		{ return iNumFrames; }

	static std::string tagName()	{ return "img"; }
	const std::string	getTagName()	{ return CAnimation::tagName(); }

};

}; // namespace SkinnedGUI

#endif  //  __CANIMATION_H__SKINNED_GUI__
