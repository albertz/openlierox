/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Widget Effects
// Created 30/6/02
// Jason Boettcher


#ifndef __CWIDGETEFFECT_H__SKINNED_GUI__
#define __CWIDGETEFFECT_H__SKINNED_GUI__

#include "SmartPointer.h"


namespace SkinnedGUI {

class CWidget;

struct EffectExpand  {
	int iAbsExpand;
	float fRelExpand;
};

class CWidgetEffect  {
public:
	CWidgetEffect(CWidget *w) : cWidget(w) {};
	virtual ~CWidgetEffect();

private:
	CWidget	*cWidget;

	// How much the effect will expand the widget
	EffectExpand eLeft;
	EffectExpand eTop;
	EffectExpand eRight;
	EffectExpand eBottom;

public:
	virtual SmartPointer<SDL_Surface> Apply(SmartPointer<SDL_Surface> surf, int& sx, int& sy) = 0;
	virtual bool isRunning() = 0;
	virtual void Run() = 0;
};

}; // namespace SkinnedGUI

#endif
