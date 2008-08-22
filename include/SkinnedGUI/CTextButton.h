/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// The CTextButton (clickable label) - the current buttons are using images only, 
// this is not very handy when writing skins.
// TODO: Add large font to OLX graphics and add option to use large font for labels and CTextButton.
// TODO: Merge with CButton

#ifndef __CTEXTBUTTON_H__SKINNED_GUI__
#define __CTEXTBUTTON_H__SKINNED_GUI__

#include "SkinnedGUI/CLabel.h"


namespace SkinnedGUI {

class CTextButton : public CLabel {
public:

	CTextButton(COMMON_PARAMS, const std::string& text, Uint32 colNormal, Uint32 colGlow):
		CLabel (name, parent, text)
	{
		iColNormal = colNormal;
		iColGlow = colGlow;
		bMouseOver = false;
		iType = wid_Textbutton;
	}

private:
	// Attributes

	Uint32	iColNormal;
	Uint32	iColGlow;
	bool	bMouseOver;
	DECLARE_EVENT(OnClick, ButtonClickHandler);

	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);

public:
	// Publish some of the default events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnMouseDown, MouseHandler)

	void	Draw(SDL_Surface *bmpDest, int drawX, int drawY);
};

}; // namespace SkinnedGUI

#endif  //  __CTEXTBUTTON_H__SKINNED_GUI__
