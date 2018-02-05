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
// Created 30/3/03
// Jason Boettcher


#include "LieroX.h"

#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "CInput.h"
#include "Sounds.h"
#include "DeprecatedGUI/CInputBox.h"


namespace DeprecatedGUI {

CInputbox * CInputbox::InputBoxSelected = NULL;
std::string CInputbox::InputBoxLabel;

///////////////////
// Draw the input box
void CInputbox::Draw(SDL_Surface * bmpDest)
{
	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, bmpImage.get()->w, MAX(bmpImage.get()->h, tLX->cFont.GetHeight()));

	int y = bMouseOver ? 17 : 0;
	DrawImageAdv(bmpDest,bmpImage, 0, y, iX, iY, bmpImage.get()->w, 17);
	bMouseOver = false;
    tLX->cFont.DrawCentre(bmpDest, iX+25, iY+1, tLX->clWhite, sText);
}

int CInputbox::KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if (keysym == SDLK_RETURN ||
		keysym == SDLK_KP_ENTER ||
		keysym == SDLK_LALT ||
		keysym == SDLK_LCTRL ||
		keysym == SDLK_LSHIFT ||
		keysym == SDLK_x ||
		keysym == SDLK_z) {
		PlaySoundSample(sfxGeneral.smpClick);
		return INB_MOUSEUP;
	}
	return INB_NONE;
}

}; // namespace DeprecatedGUI
