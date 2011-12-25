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

#ifndef __CBAR_H__SKINNED_GUI__
#define __CBAR_H__SKINNED_GUI__

#include "SkinnedGUI/CProgressbar.h"


namespace SkinnedGUI {

typedef CProgressBar CBar;

#if 0

#include <SDL.h>
#include "MathLib.h"
#include "CodeAttributes.h"

// Bar directions
enum {
	BAR_LEFTTORIGHT,
	BAR_RIGHTTOLEFT,
	BAR_TOPTOBOTTOM,
	BAR_BOTTOMTOTOP
};

class CBar {
public:
	CBar() {}
	CBar(SmartPointer<SDL_Surface> bmp, int x, int y, int label_x, int label_y, int dir, int num_fore_states = 1, int num_bg_states = 1);

private:
	// Variables
	int  X;
	int  Y;
	int  LabelX;
	int  LabelY;
	bool LabelVisible;
	int  Direction;
	int  Position;
	int  NumForeStates;
	int	 NumBgStates;
	int  CurrentForeState;
	int	 CurrentBgState;

	SmartPointer<SDL_Surface> bmpBar;

	Uint32	bgColor;
	Uint32	foreColor;

public:
	// Methods
	void		Draw(SDL_Surface * dst);
	int			GetWidth();
	int			GetHeight();
	INLINE int	GetPosition()  { return Position; }
	INLINE void SetPosition(int _p) { Position = _p; }
	INLINE int	GetX() { return X; }
	INLINE void SetX(int _x) { X = _x; }
	INLINE int	GetY() { return Y; }
	INLINE void SetY(int _y) { Y = _y; }
	INLINE int	GetLabelX() { return LabelX; }
	INLINE void SetLabelX(int _x) { LabelX = _x; }
	INLINE int	GetLabelY() { return LabelY; }
	INLINE void SetLabelY(int _y) { LabelY = _y; }
	INLINE int	GetNumForeStates()  { return NumForeStates; }
	INLINE void SetNumForeStates(int _s) { NumForeStates = _s; }  // NOTE: number of states is one state less than count of images
	INLINE int	GetCurrentForeState()  { return CurrentForeState; }  //
	INLINE void SetCurrentForeState(int _s) { CurrentForeState = MIN(NumForeStates-1, _s); } // NOTE: the first state is 0
	INLINE int	GetNumBgStates()  { return NumBgStates; }
	INLINE void SetNumBgStates(int _s) { NumBgStates = _s; }  // NOTE: number of states is one state less than count of images
	INLINE int	GetCurrentBgState()  { return CurrentBgState; }  //
	INLINE void SetCurrentBgState(int _s) { CurrentBgState = MIN(NumBgStates-1, _s); } // NOTE: the first state is 0
	INLINE bool IsLabelVisible()  { return LabelVisible; }
	INLINE void SetLabelVisible(bool _v)  { LabelVisible = _v; }
	INLINE Uint32 GetBgColor()  { return bgColor; }
	INLINE void SetBgColor(Uint32 _cl)  { bgColor = _cl; }
	INLINE Uint32 GetForeColor()  { return foreColor; }
	INLINE void SetForeColor(Uint32 _cl)  { foreColor = _cl; }
	INLINE bool IsProperlyLoaded()  { return bmpBar.get() != NULL; }
};
#endif

}; // namespace SkinnedGUI

#endif // __CBAR_H_SKINNED_GUI__
