/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Checkbox
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"


///////////////////
// Draw the checkbox
void CCheckbox::Draw(SDL_Surface *bmpDest)
{
    Menu_redrawBufferRect( iX,iY, 17,17 );

    if(iValue)
		DrawImageAdv(bmpDest, bmpImage, 17,0,iX,iY,17,17);
	else
	    DrawImageAdv(bmpDest, bmpImage, 0,0,iX,iY,17,17);
}


///////////////////
// Create
void CCheckbox::Create(void)
{
    bmpImage = LoadImage("data/frontend/checkbox.png");
}

///////////////////
// Load the style
void CCheckbox::LoadStyle(void/*node_t *cssNode*/)
{
	node_t *cssNode = NULL;
	// Find the default checkbox class, if none specified
	if (!cssNode) {
		cssNode = cWidgetStyles.FindNode("checkbox");
		if (!cssNode)
			return;
	}

	// Read properties
	property_t *prop = cssNode->tProperties;
	for(;prop;prop=prop->tNext) {
		// Image
		if (!stricmp(prop->sName,"image"))  {
			bmpImage = LoadImage(prop->sValue);
		}
		// Image width
		if (!stricmp(prop->sName,"image-width"))  {
			//iImgWidth = atoi(prop->sValue);
		}
		// Unknown
		else {
			GuiSkinError("Warning: Unknown property %s in main Checkbox class",prop->sName);
		}
	}
}
