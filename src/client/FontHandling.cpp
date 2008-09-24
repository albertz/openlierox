/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Font Handling
// Created 22/7/08
// Karel Petranek

#include "FontHandling.h"
#include "StringUtils.h"
#include "GfxPrimitives.h"
#include "CFont.h"
#include "MathLib.h"

///////////////////
// Apply CSS to the font style
void CFontStyle::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "font-family")  {
			sFontName.set(it->getFirstValue().getString("default"), it->getPriority());
		} else if (it->getName() == prefix + "color" || it->getName() == prefix + "colour")  {
			iColor.set(it->getFirstValue().getColor(iColor), it->getPriority());
		} else if (it->getName() == prefix + "font-background-color" || it->getName() == prefix + "font-background-colour")  {
			iBackgroundColor.set(it->getFirstValue().getColor(iBackgroundColor), it->getPriority());
		} else if (it->getName() == prefix + "font-weight")  {
			if (stringcaseequal(it->getFirstValue().getString(), "bold"))
				bBold.set(true, it->getPriority());
			else
				bBold.set(false, it->getPriority());
		} else if (it->getName() == prefix + "text-decoration")  {
			// Go through the values
			for (std::vector<CSSParser::Attribute::Value>::const_iterator val = it->getParsedValue().begin(); val != it->getParsedValue().end(); val++)  {
				if (stringcaseequal(val->getString(), "underline"))
					bUnderline.set(true, it->getPriority());
				else if (stringcaseequal(val->getString(), "overline"))
					bOverline.set(true, it->getPriority());
				else if (stringcaseequal(val->getString(), "line-through"))
					bStrikeThrough.set(true, it->getPriority());
			}
		} else if (it->getName() == prefix + "font-style")  {
			if (stringcaseequal(it->getFirstValue().getString(), "italics"))  {
				bItalics.set(true, it->getPriority());
			} else {
				bItalics.set(false, it->getPriority());
			}
		} else if (it->getName() == prefix + "font-size")  {
			iSize.set(it->getFirstValue().getInteger(12), it->getPriority());
		} else if (it->getName() == prefix + "vertical-spacing")  {
			iVSpacing.set(it->getFirstValue().getInteger(2), it->getPriority());
		} else if (it->getName() == prefix + "horizontal-spacing")  {
			iHSpacing.set(it->getFirstValue().getInteger(0), it->getPriority());
		} else if (it->getName() == prefix + "font-texture")  {
			bmpTexture.set(LoadGameImage(it->getFirstValue().getURL()), it->getPriority());
		}
	}
}

////////////////////
// Apply a selector to the given text
void CTextProperties::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "text-align")  {
			if (stringcaseequal(it->getFirstValue().getString(), "left"))
				iHAlignment.set(algLeft, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "center"))
				iHAlignment.set(algCenter, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "right"))
				iHAlignment.set(algRight, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "justify"))
				iHAlignment.set(algJustify, it->getPriority());
		} else if (it->getName() == prefix + "text-valign")  {
			if (stringcaseequal(it->getFirstValue().getString(), "top"))
				iVAlignment.set(algTop, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "middle"))
				iVAlignment.set(algMiddle, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "bottom"))
				iVAlignment.set(algBottom, it->getPriority());
		} else if (it->getName() == prefix + "overflow")  {
			if (stringcaseequal(it->getFirstValue().getString(), "clip"))  {
				// This is default, just remove any other clipping possibilities
				bThreeDotsEnd.set(false, it->getPriority());
				bWrap.set(false, it->getPriority());
			} else if (stringcaseequal(it->getFirstValue().getString(), "strip"))  {
				bThreeDotsEnd.set(true, it->getPriority());
			} else if (stringcaseequal(it->getFirstValue().getString(), "wrap"))  {
				bWrap.set(true, it->getPriority());
			}
		}
	}
}

/**************************
*
* Font handling
*
***************************/

#include <map>

// A very simple cache, should be improved and moved to CCache later
std::map<std::string, CFont *> fontCache;

CFont *GetFont(const std::string& name)
{
	std::map<std::string, CFont *>::iterator it = fontCache.find(name);
	if (it == fontCache.end())  {
		CFont *tmp = new CFont();
		if (!tmp->Load(name, true))  {
			delete tmp;
			tmp = NULL;
		}
		fontCache[name] = tmp;
		return tmp;
	}

	return it->second;
}

void ShutdownFontCache()
{
	for (std::map<std::string, CFont *>::iterator it = fontCache.begin(); it != fontCache.end(); it++)  {
		it->second->Shutdown();
		delete it->second;
	}
	fontCache.clear();
}


static void StripTextWithDots(std::string& buf, CFont *font, int width)
{
	if (buf.size() == 0)
		return;

	int dotwidth = font->GetWidth("...");
	bool stripped = false;
	for(size_t j=buf.length()-1; font->GetWidth(buf) > width && j != 0; j--)  {
		buf.erase(buf.length()-1);
		stripped = true;
	}

	// Make space for the dots
	if (stripped)  {
		for(size_t j=buf.length()-1; font->GetWidth(buf) > width-dotwidth && j != 0; j--)
			buf.erase(buf.length()-1);
	}

	if(stripped)
		buf += "...";
}

/////////////////////
// Draws the text to the surface according to the style & positions given
void DrawGameText(SDL_Surface *bmpDest, const std::string& text, const CFontStyle& style, const CTextProperties& prop)
{
	CFont *fnt = GetFont(style.sFontName);
	if (!fnt)  {
		return;
	}

	fnt->SetVSpacing(style.iVSpacing);
	fnt->SetSpacing(style.iHSpacing);

	// Save the clipping rect
	SDL_Rect oldrect, fontrect;
	SDL_GetClipRect(bmpDest, &oldrect);
	fontrect = oldrect;
	if (prop.tFontRect)  {
		SDL_SetClipRect(bmpDest, prop.tFontRect);
		fontrect = *prop.tFontRect;
	}

	int x = fontrect.x;
	switch (prop.iHAlignment) {
		case algLeft:
			x = fontrect.x;
			break;
		case algCenter:
			x = MAX(prop.iCentreMinX, fontrect.x + (fontrect.w - fnt->GetWidth(text)) / 2);
			break;
		case algRight:
			x = fontrect.x + fontrect.w - fnt->GetWidth(text);
			break;
		case algJustify:
		// TODO: justify
			break;
	}

	int y = fontrect.y;
	switch (prop.iVAlignment)  {
		case algTop:
			y = fontrect.y;
			break;
		case algMiddle:
			y = MAX(prop.iCentreMinY, fontrect.y + (fontrect.h - fnt->GetHeight(text)) / 2);
			break;
		case algBottom:
			y = fontrect.y + fontrect.h - fnt->GetHeight(text);
			break;
	}

	std::string txt = text;
	if (prop.bThreeDotsEnd)  {
		StripTextWithDots(txt, fnt, fontrect.w);
	}

	// Draw it
	fnt->Draw(bmpDest, x, y, style.iColor.get().get(), txt);

	// Restore the clipping
	SDL_SetClipRect(bmpDest, &oldrect);
}

///////////////////
// Draw a text onto a surface
void DrawGameText(SDL_Surface *bmpDest, int x, int y, const std::string& text)
{
	SDL_Rect r = { x, y, 9999, 9999 };
	DrawGameText(bmpDest, text, CFontStyle(), CTextProperties(&r));
}


/////////////////////
// Get a width of the given text (handles newlines correctly)
int GetTextWidth(const CFontStyle& style, const std::string& text)
{
	CFont *fnt = GetFont(style.sFontName);
	if (!fnt)  {
		return 0;
	}

	fnt->SetVSpacing(style.iVSpacing);
	fnt->SetSpacing(style.iHSpacing);

	int w = fnt->GetWidth(text);
	return w;
}

///////////////////
// Get the height of the text
int GetTextHeight(const CFontStyle& style, const std::string& text)
{
	CFont *fnt = GetFont(style.sFontName);
	if (!fnt)  {
		return 0;
	}

	fnt->SetVSpacing(style.iVSpacing);
	fnt->SetSpacing(style.iHSpacing);

	int h = fnt->GetHeight(text);
	return h;
}
