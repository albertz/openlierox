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
#include "debug.h"
#include "SkinnedGUI/CMinimap.h"
#include "XMLutils.h"


namespace SkinnedGUI {

///////////////
// Load a map to the minimap
void CMinimap::Load(const std::string& level)
{
	cMap->Shutdown();
	cMap->SetMinimapDimensions(getWidth() - cBorder.getLeftW() - cBorder.getRightW(), getHeight() - cBorder.getTopW() - cBorder.getBottomW());
	sFileName = level;
	tGameInfo.sMapRandom.bUsed = false;

	if (level == "_random_")  {
		if (cMap->New(504, 350, cMap->findRandomTheme()))  {
			cMap->ApplyRandom();

			// Free any old random map object list
			if( tGameInfo.sMapRandom.psObjects ) {
				delete[] tGameInfo.sMapRandom.psObjects;
				tGameInfo.sMapRandom.psObjects = NULL;
			}

			// Copy the layout
			maprandom_t *psRand = cMap->getRandomLayout();
			tGameInfo.sMapRandom = *psRand;
			tGameInfo.sMapRandom.bUsed = true;

			// Copy the objects, not link
			tGameInfo.sMapRandom.psObjects = new object_t[tGameInfo.sMapRandom.nNumObjects];
			if( tGameInfo.sMapRandom.psObjects ) {
				for( int i=0; i<tGameInfo.sMapRandom.nNumObjects; i++ ) {
					tGameInfo.sMapRandom.psObjects[i] = psRand->psObjects[i];
				}
			}
		}
	} else {
		cMap->Load(level);		
	}

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
	if (tWorms)
		cMap->DrawMiniMap(bmpBuffer.get(), cBorder.getLeftW(), cBorder.getTopW(), tLX->fDeltaTime, tWorms, tGameInfo.iGameMode);
	else
		DrawImage(bmpBuffer.get(), cMap->GetMiniMap(), cBorder.getLeftW(), cBorder.getTopW());

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
