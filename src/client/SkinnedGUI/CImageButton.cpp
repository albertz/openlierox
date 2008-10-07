/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Image Button
// Created 09/05/08
// Karel Petranek


#include "LieroX.h"
#include "debug.h"
#include "GfxPrimitives.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "SkinnedGUI/CImageButton.h"
#include "Cursor.h"
#include "XMLutils.h"


namespace SkinnedGUI {

//////////////////
// Constructor
CImageButton::CImageButton(COMMON_PARAMS) : CALL_DEFAULT_CONSTRUCTOR {
	CLEAR_EVENT(OnClick);
	bActive = false;
	iType = wid_Imagebutton;
	sPath = "";
}

///////////////////
// Draw the button
void CImageButton::DoRepaint()
{
	CHECK_BUFFER;

	// Don't draw an empty image
	if (!bmpImage.get())
		return;

	// Get the right frame
	int sy = 0;
	if (bMouseOver || bActive)
		sy = getHeight();
	if (bMouseDown)
		sy = 2 * getHeight();

	DrawImageAdv(bmpBuffer.get(), bmpImage, 0, sy, 0, 0, getWidth(), getHeight());
}

////////////////
// Apply the given selector
void CImageButton::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);

	// Go through the attributes
	for (std::list<CSSParser::Attribute>::const_iterator it = sel.getAttributes().begin(); it != sel.getAttributes().end(); it++)  {
		if (it->getName() == "image")  {
			sPath = JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL());
			bmpImage = LoadGameImage(sPath, true);
		}
	}
}

///////////////////
// Apply the given node
void CImageButton::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	sPath = xmlGetString(node, "src", sPath);
	if (sPath.size())
		bmpImage = LoadGameImage(sPath, true);
}

/////////////////
// Mouse move event
int	CImageButton::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	SetGameCursor(CURSOR_HAND);

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}

/////////////////
// Mouse up event
int	CImageButton::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bMouseDown = false;

	// Click?
	if (InBox(x, y))
		CALL_EVENT(OnClick, (this, button, modstate));

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate); // Calls the user defined function
	return WID_PROCESSED;
}


////////////////////
// Change the image
void CImageButton::setImage(SmartPointer<SDL_Surface> img)
{
	if (!img.get())
		return;

	Resize(getX(), getY(), img->w, img->h / 3);
	bmpImage = img;
}

}; // namespace SkinnedGUI
