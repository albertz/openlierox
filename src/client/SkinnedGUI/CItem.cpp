/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Item class
// Created 23/5/08
// Karel Petranek

#include "SkinnedGUI/CItem.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "XMLutils.h"
#include "FindFile.h"


namespace SkinnedGUI {

//
// Combobox item
//
CItem::CItem(const std::string &name)
{
	sName = name;
	sIndex = name;
	tImage = NULL;
	bActive = false;
	bDown = false;
	bInlineStyle = false;
	iVAlign.set(algMiddle, DEFAULT_PRIORITY);
	iTag = 0;
}

CItem::CItem(const std::string &sindex, const std::string &name)
{
	sName = name;
	sIndex = sindex;
	tImage = NULL;
	bActive = false;
	bDown = false;
	bInlineStyle = false;
	iVAlign.set(algMiddle, DEFAULT_PRIORITY);
	iTag = 0;
}

CItem::CItem(const std::string &name, SmartPointer<SDL_Surface> image)
{
	sName = name;
	sIndex = name;
	tImage = image;
	bActive = false;
	bDown = false;
	bInlineStyle = false;
	iVAlign.set(algMiddle, DEFAULT_PRIORITY);
	iTag = 0;
}

CItem::CItem(const std::string &sindex, SmartPointer<SDL_Surface> image, const std::string &name)
{
	sName = name;
	sIndex = sindex;
	tImage = image;
	bActive = false;
	bDown = false;
	bInlineStyle = false;
	iVAlign.set(algMiddle, DEFAULT_PRIORITY);
	iTag = 0;
}

//////////////////
// Get the item height
int CItem::getHeight()
{
	const CItemStyle *style = getCurrentStyle();
	int h = sName.size() ? GetTextHeight(style->cFont, sName) : 0;
	if (tImage.get())
		h = MAX(h, tImage->h);

	return h;
}

///////////////////
// Get the item width
int CItem::getWidth()
{
	const CItemStyle *style = getCurrentStyle();
	int w = sName.size() ? GetTextWidth(style->cFont, sName) : 0;
	if (tImage.get())
		w += tImage->w + IMAGE_SPACING;

	return w + 2 * ITEM_SPACING;
}

/////////////////////
// Get the style that should be used for drawing
CItem::CItemStyle *CItem::getCurrentStyle()
{
	// Get the appropriate style
	CItemStyle *style = &cNormalStyle;
	if (bDown)
		style = &cClickedStyle;
	else if (bActive)
		style = &cActiveStyle;
	return style;
}

////////////////////////
// Draw the item image
void CItem::DrawItemImage(SDL_Surface *bmpDest, const SDL_Rect &itemr)
{
	CItemStyle *style = getCurrentStyle();

	int cur_x = style->cBorder.getLeftW();

	if (tImage.get())  {
		// Draw according to the valign
		switch (iVAlign)  {
			case algTop:
				DrawImageAdv(bmpDest, tImage, 0, 0, cur_x, itemr.y + style->cBorder.getTopW(),
					MIN(itemr.w - style->cBorder.getLeftW() - style->cBorder.getRightW(), tImage->w), 
					MIN(itemr.h - style->cBorder.getTopW() - style->cBorder.getBottomW(), tImage->h));
				break;

			case algMiddle:
				DrawImageAdv(bmpDest, tImage, 0, 0,
					cur_x, MAX((int)itemr.y, itemr.y + style->cBorder.getTopW() + (itemr.h - tImage->h)/2),
					MIN(itemr.w - style->cBorder.getLeftW() - style->cBorder.getRightW(), tImage->w), 
					MIN(itemr.h - style->cBorder.getTopW() - style->cBorder.getBottomW(), tImage->h));
				break;

			case algBottom:
				DrawImageAdv(bmpDest, tImage, 0, 0, cur_x,
					itemr.y + itemr.h - tImage->h - style->cBorder.getBottomW(),
					MIN(itemr.w - style->cBorder.getLeftW() - style->cBorder.getRightW(), tImage->w), 
					MIN(itemr.h - style->cBorder.getTopW() - style->cBorder.getBottomW(), tImage->h));
				break;
		}
	}
}

///////////////////////
// Draw the item text
void CItem::DrawItemText(SDL_Surface *bmpDest, const SDL_Rect& itemr)
{
	CItemStyle *style = getCurrentStyle();
	int cur_x = itemr.x + ITEM_SPACING + (tImage.get() ? tImage->w + IMAGE_SPACING : 0);

	SDL_Rect r = {cur_x, itemr.y, itemr.w - cur_x, itemr.h};
	style->cText.tFontRect = &r;
	DrawGameText(bmpDest, sName, style->cFont, style->cText);
}

////////////////////
// Set the vertical align by string
void CItem::setVAlignByString(const std::string &str, size_t priority)
{
	if (stringcaseequal(str, "top"))
		iVAlign.set(algTop, priority);
	else if (stringcaseequal(str, "middle"))
		iVAlign.set(algMiddle, priority);
	else if (stringcaseequal(str, "bottom"))
		iVAlign.set(algBottom, priority);
}

/////////////////////////
// Read the information from a xml node
void CItem::ApplyTag(xmlNodePtr node)
{
	bInlineStyle = false;
	sCSSID = sIndex = xmlGetString(node, "id", sIndex);
	sCSSClass = xmlGetString(node, "class");
	std::string image_path = xmlGetString(node, "image");
	if (image_path.size())
		tImage = LoadGameImage(JoinPaths(xmlGetBaseURL(node), image_path), true);
	sName = xmlNodeText(node, sName);

	setVAlignByString(xmlGetString(node, "valign"), TAG_ATTR_PRIORITY);

	// Parse & apply the inline styles
	std::string inline_css = xmlGetString(node, "style");
	if (inline_css.size())  {
		CSSParser::Selector inline_sel;
		if (CSSParser().parseInSelector(inline_sel, inline_css, TAG_CSS_PRIORITY))  {
			bInlineStyle = true;
			inline_sel.setBaseURL(xmlGetBaseURL(node));
			cNormalStyle.ApplySelector(inline_sel);
			cClickedStyle.ApplySelector(inline_sel);
			cActiveStyle.ApplySelector(inline_sel);
		}
	}

}

///////////////////////
// Apply CSS
void CItem::CItemStyle::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	cBackground.ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);
}

}; // namespace SkinnedGUI
