/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Map Editor
// Created 8/5/08
// Karel Petranek


#include <assert.h>
#include "LieroX.h"

#include "GfxPrimitives.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Cursor.h"
#include "game/CMap.h"
#include "SkinnedGUI/CMapEditor.h"


namespace SkinnedGUI {

#define SCROLL_STEP 5

///////////////////
// Constructor
CMapEditor::CMapEditor(COMMON_PARAMS) : CContainerWidget(name, parent)
{
	cMap = new CMap();
	cVScrollbar = new CScrollbar("_VertScroll", this, scrVertical);
	cHScrollbar = new CScrollbar("_HorzScroll", this, scrHorizontal);
	iEditMode = 0;
	iPenSize = 0;
	iClientX = 0;
	iClientY = 0;
	iClientWidth = 0;
	iClientHeight = 0;
	cCursor = NULL;
}

//////////////////
// Destructor
CMapEditor::~CMapEditor()
{
	cMap->Shutdown();
	delete cMap;
	delete cVScrollbar;
	delete cHScrollbar;
	if (cCursor)
		delete cCursor;
}

///////////////////
// Apply the selector
void CMapEditor::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CContainerWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
}

/////////////////////
// Apply the given CSS
void CMapEditor::ApplyCSS(CSSParser& css)
{
	CContainerWidget::ApplyCSS(css);
	cVScrollbar->ApplyCSS(css);
	cHScrollbar->ApplyCSS(css);
}

///////////////////
// Recalculate the client area of the editor
void CMapEditor::RecalculateClientRect()
{
	// Height
	iClientHeight = getHeight() - cBorder.getTopW() - cBorder.getBottomW();
	if (cHScrollbar->getVisible())
		iClientHeight -= cHScrollbar->getHeight();

	// Width
	iClientWidth = getWidth() - cBorder.getLeftW() - cBorder.getRightW();
	if (cVScrollbar->getVisible())
		iClientWidth -= cVScrollbar->getWidth();
}

//////////////////
// Adjust the horizontal scrollbar
void CMapEditor::AdjustHScrollbar()
{
	if ((int)cMap->GetWidth() > iClientWidth)  {
		cHScrollbar->setMax(cMap->GetWidth() / SCROLL_STEP);
		cHScrollbar->setVisible(true);
	} else {
		cHScrollbar->setMax(cHScrollbar->getItemsperbox());
		cHScrollbar->setValue(0);
		cHScrollbar->setVisible(false);	
	}

	cHScrollbar->Resize(getHeight() - cBorder.getBottomW() - cHScrollbar->getHeight(), cBorder.getLeftW(), 
		getWidth() - cBorder.getLeftW() - cBorder.getRightW() - (cVScrollbar->getVisible() ? cVScrollbar->getWidth() : 0),
		cHScrollbar->getHeight());

	RecalculateClientRect();
}

///////////////////
// Adjust the vertical scrollbar
void CMapEditor::AdjustVScrollbar()
{
	if ((int)cMap->GetHeight() > iClientHeight)  {
		cVScrollbar->setMax(cMap->GetHeight() / SCROLL_STEP);
		cVScrollbar->setVisible(true);
	} else {
		cVScrollbar->setMax(cVScrollbar->getItemsperbox());
		cVScrollbar->setValue(0);
		cVScrollbar->setVisible(false);
	}

	cVScrollbar->Resize(getWidth() - cBorder.getRightW() - cVScrollbar->getWidth(), cBorder.getTopW(), cVScrollbar->getWidth(), 
		getHeight() - cBorder.getTopW() - cBorder.getBottomW());

	RecalculateClientRect();
}

/////////////////////
// Adjust both scrollbars
void CMapEditor::AdjustScrollbars()
{
	AdjustHScrollbar();
	AdjustVScrollbar();
	AdjustHScrollbar(); // The client area size could change when adding vert. scrollbar
	AdjustVScrollbar();
}

/////////////////////
// Reload the internal cursor according to iEditMode & iPenSize
void CMapEditor::ChangeCursor()
{
	if (cCursor)
		delete cCursor;
	cCursor = NULL;

	SmartPointer<SDL_Surface> image = NULL;
	theme_t *t = cMap->GetTheme();

	// Change depending on the edit mode
	switch (iEditMode)  {
	case edHoles:
	case edDirt:
		image = t->bmpHoles[CLAMP(iPenSize, 0, 4)];
	break;
	case edStones:
		image = t->bmpStones[CLAMP(iPenSize, 0, t->NumStones)];
	break;
	case edMisc:
		image = t->bmpMisc[CLAMP(iPenSize, 0, t->NumMisc)];
	break;
	default:
		warnings << "Warning: unknown edit mode in CMapEditor::ChangeCursor" << endl;
	}

	cCursor = new CCursor(image, CUR_AIM);
}

//////////////////
// Create a new map
void CMapEditor::New(int width, int height, const std::string &theme)
{
	cMap->New(width, height, theme);

	// Adjust scrollbars
	AdjustScrollbars();

	Repaint();
}

//////////////////
// Load the map from a file
void CMapEditor::Load(const std::string &path)
{
	if (!cMap->Load(path))
		New(getWidth(), getHeight(), "dirt"); // Just create a blank map on failure
	Repaint();
}

//////////////////
// Save the map to a file
void CMapEditor::Save(const std::string& name, const std::string &path)
{
	cMap->Save(name, path);
}

///////////////////
// Draw the map editor
void CMapEditor::DoRepaint()
{
	if (cVScrollbar->needsRepaint())
		cVScrollbar->DoRepaint();
	if (cHScrollbar->needsRepaint())
		cHScrollbar->DoRepaint();

	SDL_Rect r = { iClientX, iClientY, iClientWidth, iClientHeight};
	cMap->Draw(bmpBuffer.get(), r, cHScrollbar->getValue() * SCROLL_STEP, cVScrollbar->getValue() * SCROLL_STEP);
	cVScrollbar->Draw(bmpBuffer.get(), cVScrollbar->getX(), cVScrollbar->getY());
	cHScrollbar->Draw(bmpBuffer.get(), cHScrollbar->getX(), cHScrollbar->getY());
}

///////////////////
// Child repaint event
int CMapEditor::DoChildNeedsRepaint(CWidget *child)
{
	// HINT: child widgets are only scrollbars and if they need repaint, it means
	// they have scrolled -> we need repaint, too
	CContainerWidget::DoChildNeedsRepaint(child);
	Repaint();

	return WID_PROCESSED;
}

////////////////////
// Create event
int CMapEditor::DoCreate()
{
	CWidget::DoCreate();

	// Setup the scrollbars
	cVScrollbar->setItemsperbox(getHeight()/SCROLL_STEP);
	cVScrollbar->setMax(getHeight()/SCROLL_STEP);
	cHScrollbar->setItemsperbox(getWidth()/SCROLL_STEP);
	cHScrollbar->setMax(getWidth()/SCROLL_STEP);
	cVScrollbar->setVisible(false);
	cHScrollbar->setVisible(false);

	// Client rectangle
	RecalculateClientRect();

	// Create default map
	New(getWidth(), getHeight(), "dirt");
	ChangeCursor();

	return WID_PROCESSED;
}



///////////////////
// Mouse move event
int CMapEditor::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	// Scrollbars
	if (cVScrollbar->getVisible() && cVScrollbar->getGrabbed())
		return cVScrollbar->DoMouseMove(x, y, dx, dy, down, button, modstate);
	if (cHScrollbar->getVisible() && cHScrollbar->getGrabbed())
		return cHScrollbar->DoMouseMove(x, y, dx, dy, down, button, modstate);

	if (InBox(x, y))  {
		// Set the cursor to one of the edited
		SetGameCursor(cCursor);

		// Process the modes that need continuous update when the mouse is down
		if (down)  {

			// Carve/place dirt in the whole line from old point to the new
			CVec dir = CVec((float)dx, (float)dy);
			CVec cur_pos = CVec((float)(x - dx + cHScrollbar->getValue() * SCROLL_STEP), (float)(y - dy + cVScrollbar->getValue() * SCROLL_STEP));
			int len = (int)dir.GetLength();
			dir = dir.Normalize() * 3; // Go by 3 px

			switch (iEditMode) {
			case edHoles:
				for (int i=0; i < len; i++, cur_pos += dir)
					cMap->CarveHole(iPenSize, cur_pos, false);
			break;
			case edDirt:
				for (int i=0; i < len; i++, cur_pos += dir)
					cMap->PlaceDirt(iPenSize, cur_pos);
			break;
			}

			// Repaint
			Repaint();
		}
	}

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
    return WID_PROCESSED;
}


///////////////////
// Mouse Up event
int CMapEditor::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Scrollbars
	if (cVScrollbar->getVisible() && cVScrollbar->InBox(x, y))
		return cVScrollbar->DoMouseUp(x, y, dx, dy, button, modstate);
	if (cHScrollbar->getVisible() && cHScrollbar->InBox(x, y))
		return cHScrollbar->DoMouseUp(x, y, dx, dy, button, modstate);

	// Only put the objects if user clicked in the editing area
	if(InBox(x, y))  {
		CVec map_pos = CVec((float)(x + cHScrollbar->getValue() * SCROLL_STEP), (float)(y + cVScrollbar->getValue() * SCROLL_STEP));
		switch (iEditMode)  {
			case edStones: // Stone
				cMap->PlaceStone(iPenSize, map_pos);
			break;

			case edMisc: // Misc
				cMap->PlaceMisc(iPenSize, map_pos);
			break;
		}

		// Repaint
		Repaint();
	}

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
    return WID_PROCESSED;
}


///////////////////
// Mouse down event
int CMapEditor::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Scrollbars
	if (cVScrollbar->getVisible() && cVScrollbar->InBox(x, y))
		return cVScrollbar->DoMouseDown(x, y, dx, dy, button, modstate);
	if (cHScrollbar->getVisible() && cHScrollbar->InBox(x, y))
		return cHScrollbar->DoMouseDown(x, y, dx, dy, button, modstate);

    // Only put the objects if user clicked in the editing area
	if(InBox(x, y))  {
		CVec map_pos = CVec((float)(x + cHScrollbar->getValue() * SCROLL_STEP), (float)(y + cVScrollbar->getValue() * SCROLL_STEP));

		switch (iEditMode) {
		case edHoles:
			cMap->CarveHole(iPenSize, map_pos, false);
		break;
		case edDirt:
			cMap->PlaceDirt(iPenSize, map_pos);
		break;
		}

		// Repaint
		Repaint();
	}

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
    return WID_PROCESSED;
}

///////////////////
// Mouse wheel down event
int CMapEditor::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState &modstate)
{
	if (cVScrollbar->getVisible())
		cVScrollbar->DoMouseWheelDown(x, y, dx, dy, modstate);

	if (cHScrollbar->getVisible())
		cHScrollbar->DoMouseWheelDown(x, y, dx, dy, modstate);

	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

////////////////////
// Mouse wheel up event
int CMapEditor::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState &modstate)
{
	if (cVScrollbar->getVisible())
		cVScrollbar->DoMouseWheelUp(x, y, dx, dy, modstate);

	if (cHScrollbar->getVisible())
		cHScrollbar->DoMouseWheelUp(x, y, dx, dy, modstate);

	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
