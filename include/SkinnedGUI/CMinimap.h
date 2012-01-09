/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Label
// Created 30/6/02
// Jason Boettcher


#ifndef __CMINIMAP_H__SKINNED_GUI__
#define __CMINIMAP_H__SKINNED_GUI__


#include "CMap.h"
#include "CWorm.h"
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CBorder.h"


namespace SkinnedGUI {

class CMinimap : public CWidget {
public:
	CMinimap(COMMON_PARAMS) : CALL_DEFAULT_CONSTRUCTOR {
		cMap = new CMap;
		bFreeMap = true;
	}

	CMinimap(COMMON_PARAMS, CMap *map) : CALL_DEFAULT_CONSTRUCTOR {
		cMap = map;
		bFreeMap = false;
	}

	~CMinimap()  {
		if (bFreeMap)
			delete cMap;
	}


private:
	// Attributes
	CMap *cMap;
	std::string sFileName;
	bool bFreeMap;

	// Appearance
	CBorder		cBorder;

	// Events
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);

	void	DoRepaint();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);

public:
	// Methods
	void Load(const std::string& level);

	bool	needsRepaint()	{ return CWidget::needsRepaint(); }

	static const std::string tagName()		{ return "minimap"; }
	const std::string getTagName()			{ return CMinimap::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CMINIMAP_H__SKINNED_GUI__
