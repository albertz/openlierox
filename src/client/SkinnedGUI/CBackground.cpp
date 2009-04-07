/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Background for widgets
// Created 22/7/08
// Karel Petranek

#include "SkinnedGUI/CBackground.h"

#include "StringUtils.h"
#include "GfxPrimitives.h"
#include "FindFile.h"


namespace SkinnedGUI {

//////////////////
// Constructor
CBackground::CBackground()
{
	const Color def(255, 255, 255, SDL_ALPHA_TRANSPARENT);
	clMain.set(def, DEFAULT_PRIORITY);
	clGradient1.set(def, DEFAULT_PRIORITY);
	clGradient2.set(def, DEFAULT_PRIORITY);
	bUseGradient.set(false, DEFAULT_PRIORITY);
	iGradientDirection.set(grdHorizontal, DEFAULT_PRIORITY);
	bmpMain.set(NULL, DEFAULT_PRIORITY);
	iRepeat.set(bgRepeat, DEFAULT_PRIORITY);

}

/////////////////////
// Apply a selector to the background
void CBackground::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "background-color" || it->getName() == prefix + "background-colour")  {
			clMain.set(it->getFirstValue().getColor(clMain), it->getPriority());
		} else if (it->getName() == prefix + "background-repeat")  {
			if (stringcaseequal(it->getFirstValue().getString(), "no-repeat"))
				iRepeat.set(bgNoRepeat, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "repeat"))
				iRepeat.set(bgRepeat, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "repeat-x"))
				iRepeat.set(bgRepeatX, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "repeat-y"))
				iRepeat.set(bgRepeatY, it->getPriority());
		} else if (it->getName() == prefix + "background-gradient")  {
			bUseGradient.set(true, it->getPriority());

			// Read the colors
			if (it->getParsedValue().size() > 0)
				clGradient1.set(it->getParsedValue()[0].getColor(clGradient1), it->getPriority());
			if (it->getParsedValue().size() > 1)
				clGradient2.set(it->getParsedValue()[1].getColor(clGradient2), it->getPriority());
			if (it->getParsedValue().size() > 2)  {
				if (stringcaseequal(it->getParsedValue()[2].getString(), "vertical"))
					iGradientDirection.set(grdVertical, it->getPriority());
				else if (stringcaseequal(it->getParsedValue()[2].getString(), "horizontal"))
					iGradientDirection.set(grdHorizontal, it->getPriority());
			}
		} else if (it->getName() == prefix + "background-image")  {
			bmpMain.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
		}
	}
}

///////////////////
// Draw the background
void CBackground::Draw(SDL_Surface *bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect)
{
	// Handle the clipping rect
	SDL_Rect r;
	if(cliprect) r = *cliprect;
	else SDL_GetClipRect(bmpDest, &r);
	ScopedSurfaceClip clip(bmpDest, r);

	// Background image
	if (bmpMain.get().get())  {
		switch (iRepeat)  {
		case bgNoRepeat:
			DrawImageAdv(bmpDest, bmpMain.get(), 0, 0, x, y, w, h);
		break;
		case bgRepeat:
			DrawImageTiled(bmpDest, bmpMain.get(), 0, 0, bmpMain->w, bmpMain->h, x, y, w, h);
		break;
		case bgRepeatX:
			DrawImageTiledX(bmpDest, bmpMain.get(), 0, 0, bmpMain->w, bmpMain->h, x, y, w, h);
		break;
		case bgRepeatY:
			DrawImageTiledY(bmpDest, bmpMain.get(), 0, 0, bmpMain->w, bmpMain->h, x, y, w, h);
		break;
		}

	// Background gradient
	} else if (bUseGradient)  {
		DrawLinearGradient(bmpDest, x, y, w, h, clGradient1, clGradient2, (GradientDirection)iGradientDirection.get());

	// Solid background
	} else {
		DrawRectFill(bmpDest, x, y, x + w, y + h, clMain);
	}
}

}; // namespace SkinnedGUI
