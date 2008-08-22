
#include "LieroX.h"
#include "SkinnedGUI/CDialog.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "XMLutils.h"
#include "FindFile.h"


namespace SkinnedGUI {

///////////////
// Create
CTitleBar::CTitleBar(CDialog *parent, const std::string& text) : CWidget("_TitleBar", parent)
{
	sText = text;
	bGrabbed = false;
	bmpLeft.set(NULL, DEFAULT_PRIORITY);
	bmpRight = bmpMain = bmpLeft;
}

////////////////////
// Repaint the titlebar
void CTitleBar::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	SDL_Rect r = { 0, 0, getWidth() - cBorder.getLeftW() - cBorder.getRightW(), getHeight() - cBorder.getTopW() - cBorder.getBottomW() };

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Images
	if (bmpLeft.get().get()) DrawImageAdv(bmpBuffer.get(), bmpLeft, r, bmpLeft->clip_rect);
	if (bmpMain.get().get())  {
		int right_bound = getWidth() - (bmpRight.get().get() ? bmpRight->w : 0);
		for (int i = bmpLeft->w; i < right_bound; i += bmpMain->w)
			DrawImageAdv(bmpBuffer.get(), bmpMain, 0, 0, i, 0, bmpMain->w, bmpMain->h);
	}
	if (bmpRight.get().get()) DrawImageAdv(bmpBuffer.get(), bmpRight, 0, 0, getWidth() - bmpRight->w, 0, bmpRight->w, bmpRight->h);
	
	// Text
	cText.tFontRect = &r;
	DrawGameText(bmpBuffer.get(), sText, cFont, cText);
}

////////////////////
// Apply the CSS
void CTitleBar::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);

	// Go through the attributes
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "left-image")
			bmpLeft.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
		else if (it->getName() == prefix + "middle-image")
			bmpMain.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
		else if (it->getName() == prefix + "right-image")
			bmpRight.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
	}

}

/////////////////
// Mouse down event
int CTitleBar::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if (!((CDialog *)cParent)->isMoveable())  {
		CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	}

	// First click, grab the dialog
	bGrabbed = true;

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

////////////////
// Mouse move event
int	CTitleBar::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	// Holding and moving the mouse - move the owner dialog
	if (bGrabbed && down)
		((CDialog *)cParent)->MoveBy(dx, dy);

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}

//////////////////
// Mouse up event
int CTitleBar::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bGrabbed = false;
	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

///////////////////
// Parent move/resize event
int	CTitleBar::DoParentResize(int& new_parent_w, int& new_parent_h)
{
	int min_w = (bmpLeft.get().get() ? bmpLeft->w : 0) + (bmpRight.get().get() ? bmpRight->w : 0) + cBorder.getLeftW() + cBorder.getTopW();
	new_parent_w = MAX(new_parent_w, min_w);
	new_parent_h = MAX(new_parent_w, getHeight());

	return WID_PROCESSED;
}





//////////////////
// Create
CDialog::CDialog(COMMON_PARAMS, const std::string& title, bool modal, bool moveable, bool resizable) :
CGuiSkinnedLayout(name, parent)
{
	cTitle = new CTitleBar(this, title);
	bModal = modal;
	bMoveable.set(moveable, DEFAULT_PRIORITY);
	bResizable.set(resizable, DEFAULT_PRIORITY);
	bOverlap = true;
}

CDialog::CDialog(const std::string &name, CContainerWidget *parent) : CGuiSkinnedLayout(name, parent)
{
	cTitle = new CTitleBar(this, "");
	bModal = true;
	bMoveable.set(true, DEFAULT_PRIORITY);
	bResizable.set(true, DEFAULT_PRIORITY);
	bOverlap = true;
}

//////////////////
// Destroy
CDialog::~CDialog()
{

}

/////////////////
// Enable/disable
void CDialog::setEnabled(bool _e)
{
	if (bModal && _e != bEnabled)  {
		if (_e)
			((CGuiSkinnedLayout *)cParent)->incModalsRunning();
		else
			((CGuiSkinnedLayout *)cParent)->decModalsRunning();
	}

	bEnabled = _e;
}

/////////////////
// Show/hide
void CDialog::setVisible(bool _v)
{
	if (bModal && _v != bVisible)  {
		if (_v)
			((CGuiSkinnedLayout *)cParent)->incModalsRunning();
		else
			((CGuiSkinnedLayout *)cParent)->decModalsRunning();
	}

	bVisible.set(_v, HIGHEST_PRIORITY);

	// HINT: we don't call Repaint() here, because dialogs are overlapping widgets and
	// therefore are not drawn on the buffer
}

///////////////////
// Move the dialog by a specified number of pixels
void CDialog::MoveBy(int dx, int dy)
{
	if (!bMoveable)
		return;

	// Make sure we are inside the screen
	CGuiSkinnedLayout::MoveBy(dx, dy);
	CGuiSkinnedLayout *p = (CGuiSkinnedLayout *)cParent;
	int x = MIN(getX(), p->getX() + p->getWidth() - getWidth());
	int y = MIN(getY(), p->getY() + p->getHeight() - getHeight());
	x = MAX(p->getX(), getX());
	y = MAX(p->getX(), getY());

	Resize(x, y, getWidth(), getHeight());
}

////////////////////
// Move the dialog to a specified position
void CDialog::MoveTo(int x, int y)
{
	MoveBy(x - getX(), y - getY());
}

////////////////////
// Resize the dialog
void CDialog::Resize(int x, int y, int w, int h)
{
	int min_h = cBorder.getBottomW() + cBorder.getTopW() + cTitle->getHeight();
	h = MAX(h, min_h);

  	SDLRect rect(cParent->getRect());
	if (ClipRefRectWith(x, y, w, h, rect)) {
		CGuiSkinnedLayout::Resize(x, y, w, h);

		RecalculateClientRect();
	}
}

////////////////////
// Close the dialog
void CDialog::Close()
{
	Destroy();
}

/////////////////////
// Recalculate the client rectangle
void CDialog::RecalculateClientRect()
{
	iClientX = getX() + cBorder.getLeftW();
	iClientY = getY() + cBorder.getRightW();
	iClientWidth = getWidth() - cBorder.getLeftW() - cBorder.getRightW();
	iClientHeight = getHeight() - cBorder.getTopW() - cBorder.getBottomW() - cTitle->getHeight();
}

////////////////////
// Repaint the dialog
void CDialog::DoRepaint()
{
	// Adjust the background & border not to affect the titlebar
	tBgRect = MakeRect(0, cTitle->getHeight(), getWidth(), getHeight() - cTitle->getHeight());
	tBorderRect = tBgRect;

	// Draw the widgets & stuff
	CGuiSkinnedLayout::DoRepaint();

	// Make sure there is nothing under the titlebar
	DrawRectFill(bmpBuffer.get(), cTitle->getX(), cTitle->getY(), cTitle->getX() + cTitle->getWidth(), 
		cTitle->getY() + cTitle->getHeight(), Color(0, 0, 0, SDL_ALPHA_TRANSPARENT));

	// Draw the titlebar
	cTitle->Draw(bmpBuffer.get(), cTitle->getX(), cTitle->getY());
}

///////////////////
// Apply the given selector
void CDialog::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CGuiSkinnedLayout::ApplySelector(sel, prefix);
}

/////////////////////
// Apply the given tag
void CDialog::ApplyTag(xmlNodePtr node)
{
	CGuiSkinnedLayout::ApplyTag(node);

	cTitle->setText(xmlGetString(node, "title", cTitle->getText()));
}

}; // namespace SkinnedGUI
