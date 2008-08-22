// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CBOX_H__SKINNED_GUI__
#define __CBOX_H__SKINNED_GUI__

#include "InputEvents.h"


namespace SkinnedGUI {

class CBox : public CWidget {
public:
	// Constructor
	CBox(COMMON_PARAMS, int round,int border,Uint32 lightcolour,Uint32 darkcolour, Uint32 bgcolour);

private:
    // Attributes
	int		iRound;
	int		iBorder;
	Uint32	iLightColour;
	Uint32	iDarkColour;
	Uint32	iBgColour;

	SDL_Surface *bmpBuffer;

	int DoMouseEnter(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);

	bool CheckEvent(int ms_x, int ms_y);


public:
    // Methods

	void			setRound(int round)			{ iRound = round; }
	int				getRound(void)				{ return iRound; }
	void			setBorder(int border)		{ iBorder = border; }
	int				getBorder(void)				{ return iBorder; }
	void			setLightColour(Uint32 col)	{ iLightColour = col; }
	Uint32			getLightColour(void)		{ return iLightColour; }
	void			setDarkColour(Uint32 col)	{ iDarkColour = col;  }
	Uint32			getDarkColour(void)			{ return iDarkColour; }
	void			setBgColour(Uint32 col)		{ iBgColour = col; }
	Uint32			getBgColour(void)			{ return iBgColour; }

	void	LoadStyle(void);
	
	void	ProcessGuiSkinEvent(int iEvent) {};
};

}; // namespace SkinnedGUI

#endif  //  __CBOX_H__SKINNED_GUI__
