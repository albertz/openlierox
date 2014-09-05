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
// Karel Petranek

#include "Options.h"

#include "SkinnedGUI/CMinimap.h"
#include "XMLutils.h"
#include "CClient.h"


namespace SkinnedGUI {

///////////////
// Load a map to the minimap
void CMinimap::Load(const std::string& level)
{
	cMap->Shutdown();
	cMap->SetMinimapDimensions(getWidth() - cBorder.getLeftW() - cBorder.getRightW(), getHeight() - cBorder.getTopW() - cBorder.getBottomW());
	sFileName = level;

	cMap->Load(level);		

	Repaint();
}

///////////////////////
// Apply the given selector
void CMinimap::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
}

//////////////////////
// Apply the given tag
void CMinimap::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	if (xmlPropExists(node, "map"))
		cMap->Load(xmlGetString(node, "map"));
}

///////////////////
// Draw the minimap
void CMinimap::DoRepaint()
{
	CHECK_BUFFER;
	
	// Draw the minimap
//	if (tWorms)
		cMap->DrawMiniMap(bmpBuffer.get(), cBorder.getLeftW(), cBorder.getTopW(), tLX->fDeltaTime);
/*	else {
		if(cMap->GetMiniMap().get())
			DrawImage(bmpBuffer.get(), cMap->GetMiniMap(), cBorder.getLeftW(), cBorder.getTopW());
	}*/
	
	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

//////////////////
// Mouse up event
int CMinimap::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// On click create a new random map
	if (sFileName == "_random_")  {
		Load("_random_");
	}

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
