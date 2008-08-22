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


#include "LieroX.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "FindFile.h"
#include "SkinnedGUI/CCheckbox.h"
#include "XMLutils.h"


namespace SkinnedGUI {

//////////////////
// Create
CCheckbox::CCheckbox(COMMON_PARAMS, bool val) : CALL_DEFAULT_CONSTRUCTOR {
	bValue.set(val, HIGHEST_PRIORITY);
    bmpImage.set(NULL, DEFAULT_PRIORITY);
	iType = wid_Checkbox;
	CLEAR_EVENT(OnChange);
}

CCheckbox::CCheckbox(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	bValue.set(false, HIGHEST_PRIORITY);
    bmpImage.set(NULL, DEFAULT_PRIORITY);
	iType = wid_Checkbox;
	CLEAR_EVENT(OnChange);	
}

///////////////////
// Draw the checkbox
void CCheckbox::DoRepaint()
{
	if (!bmpImage.get().get())
		return;

    if(bValue)
		DrawImageAdv(bmpBuffer.get(), bmpImage, bmpImage->w / 2, 0, 0, 0, bmpImage->w / 2, bmpImage->h);
	else
	    DrawImageAdv(bmpBuffer.get(), bmpImage, 0, 0, 0, 0, bmpImage->w / 2, bmpImage->h);
}

//////////////////
// Applies the given CSS selector
void CCheckbox::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix = "")
{
	CWidget::ApplySelector(sel, prefix);


	for (std::list<CSSParser::Attribute>::const_iterator it = sel.getAttributes().begin(); it != sel.getAttributes().end(); it++)  {
		if (it->getName() == "image")
			bmpImage.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
	}
}

/////////////////////////
// Read the information from a xml node
void CCheckbox::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	// Read the attributes
	std::string image_path = xmlGetString(node, "image");
	if (image_path.size())
		bmpImage.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), image_path), true), TAG_ATTR_PRIORITY);
	bValue.set(xmlGetBool(node, "checked", bValue), TAG_ATTR_PRIORITY);
}

///////////////
// Click
int CCheckbox::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bool cancel = false;
	CALL_EVENT(OnChange, (this, !bValue, cancel));
	if (cancel)
		return WID_PROCESSED;

	bValue.set(!bValue, HIGHEST_PRIORITY);
	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);

	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
