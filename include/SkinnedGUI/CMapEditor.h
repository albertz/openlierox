/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Menu
// Created 8/5/08
// Karel Petranek

#ifndef __CMAPEDITOR_H__SKINNED_GUI__
#define __CMAPEDITOR_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CScrollbar.h"
#include "game/CMap.h"
#include "Cursor.h"


namespace SkinnedGUI {


// Edit modes
enum  {
	edHoles = 0,
	edStones,
	edMisc,
	edDirt
};

class CMapEditor : public CContainerWidget {
private:
    // Attributes

	int		iEditMode;  // Stone, hole, misc, dirt
	int		iPenSize;  // Size of stone, hole, ...
	CMap	*cMap;
	int		iClientX;
	int		iClientY;
	int		iClientWidth;
	int		iClientHeight;
	CCursor *cCursor;

	CBorder cBorder;

	CScrollbar *cVScrollbar;
	CScrollbar *cHScrollbar;

	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	void	DoRepaint();
	int		DoChildNeedsRepaint(CWidget *child);
	int		DoCreate();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyCSS(CSSParser& css);

	void	RecalculateClientRect();
	void	AdjustHScrollbar();
	void	AdjustVScrollbar();
	void	AdjustScrollbars();
	void	ChangeCursor();

public:

    // Methods
	CMapEditor(COMMON_PARAMS);
	~CMapEditor();

	void	setEditMode(int _m)  { iEditMode = _m; ChangeCursor(); }
	int		getEditMode()		{ return iEditMode; ChangeCursor(); }
	void	setPenSize(int _s)	{ iPenSize = _s; ChangeCursor(); }
	int		getPenSize()		{ return iPenSize; ChangeCursor(); }

	void	New(int width, int height, const std::string& theme);
	void	Load(const std::string& path);
	void	Save(const std::string&name, const std::string& path);

	static const std::string tagName()		{ return "mapeditor"; }
	const std::string getTagName()			{ return CMapEditor::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CMENU_H__SKINNED_GUI__
