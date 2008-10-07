/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Border for widgets
// Created 22/7/08
// Karel Petranek

#include "SkinnedGUI/CBorder.h"
#include "StringUtils.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "MathLib.h"
#include "Utils.h"



namespace SkinnedGUI {

///////////////////
// Helper function, converts a string to border style
BorderStyle StringToBorderStyle(const std::string& str, bool& fail)
{
	fail = false;

	if (stringcaseequal(str, "solid"))  {
		return brdSolid;
	} else if (stringcaseequal(str, "inset"))  {
		return brdInset;
	} else if (stringcaseequal(str, "outset"))  {
		return brdOutset;
	}

	fail = true;
	return brdSolid;
}

///////////////////
// Apply a selector to one border side
void CBorder::ApplySelectorToBorder(const CSSParser::Selector &sel, const std::string &prefix, const std::string &side, CBorder::BorderLineSettings &sett)
{
	bool fail = false;
	std::string att = prefix + "border-" + side;
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == att + "color" || it->getName() == att + "light" || it->getName() == "colour")  {
			sett.clLight.set(it->getFirstValue().getColor(sett.clLight), it->getPriority());
			sett.clDark.set(sett.clLight, it->getPriority());
		} else if (it->getName() == att + "dark")  {
			sett.clDark.set(it->getFirstValue().getColor(sett.clDark), it->getPriority());
		} else if (it->getName() == att + "size")  {
			sett.iThickness.set(it->getFirstValue().getInteger(0), it->getPriority());
		} else if (it->getName() == att + "style")  {
			sett.iStyle.set(StringToBorderStyle(it->getFirstValue().getString(), fail), it->getPriority());
		} else if (it->getName() == att + "image")  {
			sett.bmpLine.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
		} else if (it->getName() == prefix + "border")  {
			bool thickness_set = false;
			for (std::vector<CSSParser::Attribute::Value>::const_iterator val = it->getParsedValue().begin(); val != it->getParsedValue().end(); val++)  {
				// Style
				BorderStyle style = StringToBorderStyle(val->getString(), fail);
				if (!fail)  {
					sett.iStyle.set(style, it->getPriority());
					if (!thickness_set)
						sett.iThickness.set(MAX(1, (int)sett.iThickness), it->getPriority()); // for example "border: solid black" means "1px solid black"
					continue;
				}

				// Thickness
				int thick = from_string<int>(val->getString(), fail);
				if (!fail && thick >= 0)  {
					thickness_set = true;
					sett.iThickness.set(thick, it->getPriority());
				}

				// Color
				Color c = StrToCol(val->getString(), fail);
				if (!fail)  {
					sett.clDark.set(c, it->getPriority());
					sett.clLight.set(c, it->getPriority());
				}
			}
		}
	}
}

/////////////////////////
// Apply a selector to a corner
void CBorder::ApplySelectorToCorner(const CSSParser::Selector &sel, const std::string &prefix, const std::string &corner, CBorder::BorderCornerSettings &sett)
{
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "border-radius-" + corner)
			sett.iRoundRadius.set(it->getFirstValue().getInteger(0), it->getPriority());
		else if (it->getName() == prefix + "border-image-" + corner)
			sett.bmpCorner.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL())), it->getPriority());
	}
}

/////////////////////
// Apply a selector to a border
void CBorder::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	ApplySelectorToBorder(sel, prefix, "", BorderLeft);
	ApplySelectorToBorder(sel, prefix, "", BorderRight);
	ApplySelectorToBorder(sel, prefix, "", BorderTop);
	ApplySelectorToBorder(sel, prefix, "", BorderBottom);

	ApplySelectorToBorder(sel, prefix, "left-", BorderLeft);
	ApplySelectorToBorder(sel, prefix, "right-", BorderRight);
	ApplySelectorToBorder(sel, prefix, "top-", BorderTop);
	ApplySelectorToBorder(sel, prefix, "bottom-", BorderBottom);

	ApplySelectorToCorner(sel, prefix, "topleft", CornerTopLeft);
	ApplySelectorToCorner(sel, prefix, "topright", CornerTopRight);
	ApplySelectorToCorner(sel, prefix, "bottomleft", CornerBottomLeft);
	ApplySelectorToCorner(sel, prefix, "bomttomright", CornerBottomRight);
}

//////////////////////
// Get dimensions of a corner
void CBorder::getCornerDimensions(const BorderCornerSettings& sett, const CBorder::BorderLineSettings& hrz, const CBorder::BorderLineSettings& vrt, int& w, int& h)
{
	if (sett.bmpCorner.get().get())  {
		w = sett.bmpCorner->w;
		h = sett.bmpCorner->h;
	} else {
		w = MAX(sett.iRoundRadius, vrt.iThickness);
		h = MAX(sett.iRoundRadius, hrz.iThickness);
	}
}

///////////////////
// Returns top-left corner rect
SDL_Rect CBorder::getTopLeftR(const SDL_Rect& border_r)
{
	int w, h;
	getCornerDimensions(CornerTopLeft, BorderTop, BorderLeft, w, h);
	return MakeRect(border_r.x, border_r.y, w, h);
}

///////////////////
// Returns top-right corner rect
SDL_Rect CBorder::getTopRightR(const SDL_Rect& border_r)
{
	int w, h;
	getCornerDimensions(CornerTopRight, BorderTop, BorderRight, w, h);
	return MakeRect(border_r.x  + border_r.w - w, border_r.y, w, h);
}

///////////////////
// Returns bottom-left corner rect
SDL_Rect CBorder::getBottomLeftR(const SDL_Rect& border_r)
{
	int w, h;
	getCornerDimensions(CornerBottomLeft, BorderBottom, BorderLeft, w, h);
	return MakeRect(border_r.x, border_r.y + border_r.h - h, w, h);
}

///////////////////
// Returns bottom-right corner rect
SDL_Rect CBorder::getBottomRightR(const SDL_Rect& border_r)
{
	int w, h;
	getCornerDimensions(CornerBottomRight, BorderBottom, BorderRight, w, h);
	return MakeRect(border_r.x + border_r.w - w, border_r.y + border_r.h - h, w, h);
}

///////////////////////
// Draws a 1/4 circle (for rounded borders)
void DrawFourthCircle(SDL_Surface *dest, float from, float to, int x, int y, int radius, int thickness, Color cl_in, Color cl_out)
{
	// Check
	assert(radius != 0 && thickness != 0);

	// Gradient

	float rstep = (float)(cl_out.r - cl_in.r) / (float)thickness;
	float gstep = (float)(cl_out.g - cl_in.g) / (float)thickness;
	float bstep = (float)(cl_out.b - cl_in.b) / (float)thickness;
	float astep = (float)(cl_out.a - cl_in.a) / (float)thickness;

	float step = 1.0f / ((float)PI * radius * thickness);

	// Draw the circle
	for (float i = from; i < to; i += step)  {
		Uint32 cur_col = cl_in.get(dest->format);
		for (int j = 0; j < thickness; j++)  {
			int px = (int)((radius - j) * sin(PI * i)) - 1;
			int py = (int)((radius - j) * cos(PI * i)) - 1;
			PutPixel(dest, x + px + radius, y + py + radius, cur_col);
			++j;
			cur_col = SDL_MapRGBA(dest->format, cl_out.r + Round(rstep * j),
				cl_out.g + Round(gstep * j),
				cl_out.b + Round(bstep * j),
				cl_out.a + Round(astep * j));
		}
	}
}

void DrawBorderCorner(SDL_Surface *dest, const SDL_Rect& r, int round, int v_thick, int h_thick, Color v_in, Color v_out, Color h_in, Color h_out, int corn)
{
	if (v_thick <= 0 || h_thick <= 0 || r.w == 0 || r.h == 0)
		return;

	float v_rstep = (float)(v_out.r - v_in.r) / (float)v_thick;
	float v_gstep = (float)(v_out.g - v_in.g) / (float)v_thick;
	float v_bstep = (float)(v_out.b - v_in.b) / (float)v_thick;
	float v_astep = (float)(v_out.a - v_in.a) / (float)v_thick;

	float h_rstep = (float)(h_out.r - h_in.r) / (float)h_thick;
	float h_gstep = (float)(h_out.g - h_in.g) / (float)h_thick;
	float h_bstep = (float)(h_out.b - h_in.b) / (float)h_thick;
	float h_astep = (float)(h_out.a - h_in.a) / (float)h_thick;

	float h_ln_step = (float)r.w / (float)r.h; // By how many pixels we go every line when drawing the horizontal part
	float v_ln_step = (float)r.h / (float)r.w;

	int i = 0;
	Uint32 cl;

	// Draw the corner
	switch (corn)  {
		case 1:  { // Top left
			// Horizontal part
			i = 0;
			for (int ly = r.y; ly < r.y + h_thick; ly++, i++)  {
				cl = Color(h_in.r + Round(i * h_rstep), h_in.g + Round(i * h_gstep), h_in.b + Round(i * h_bstep), h_in.a + Round(i * h_astep)).get(dest->format);
				for (int lx = r.x + Round(i * h_ln_step); lx < r.x + v_thick; lx++)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}

			// Vertical part
			i = 0;
			for (int lx = r.x; lx < r.x + v_thick; lx++, i++)  {
				cl = Color(v_in.r + Round(i * v_rstep), v_in.g + Round(i * v_gstep), v_in.b + Round(i * v_bstep), v_in.a + Round(i * v_astep)).get(dest->format);
				for (int ly = r.y + Round(i * v_ln_step); ly < r.y + h_thick; ly++)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}
		} break;

		case 2:  { // Top right
			// Horizontal part
			i = 0;
			for (int ly = r.y; ly < r.y + h_thick; ly++, i++)  {
				cl = Color(h_in.r + Round(i * h_rstep), h_in.g + Round(i * h_gstep), h_in.b + Round(i * h_bstep), h_in.a + Round(i * h_astep)).get(dest->format);
					for (int lx = r.x + v_thick - Round(i * h_ln_step) - 1; lx >= r.x; lx--)  {
						if (PointInRect(lx, ly, dest->clip_rect))
							PutPixel(dest, lx, ly, cl);
					}
			}

			// Vertical part
			i = 0;
			for (int lx = r.x; lx < r.x + v_thick; lx++, i++)  {
				cl = Color(v_out.r - Round(i * v_rstep), v_out.g - Round(i * v_gstep), v_out.b - Round(i * v_bstep), v_out.a - Round(i * v_astep)).get(dest->format);
				for (int ly = r.y + h_thick - Round(i * v_ln_step); ly < r.y + h_thick; ly++)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}
		} break;

		case 3:  { // Bottom left
			// Horizontal part
			i = 0;
			for (int ly = r.y; ly < r.y + h_thick; ly++, i++)  {
				cl = Color(h_out.r - Round(i * h_rstep), h_out.g - Round(i * h_gstep), h_out.b - Round(i * h_bstep), h_out.a - Round(i * h_astep)).get(dest->format);
					for (int lx = r.x + v_thick - Round(i * h_ln_step) - 1; lx < r.x + r.w; lx++)  {
						if (PointInRect(lx, ly, dest->clip_rect))
							PutPixel(dest, lx, ly, cl);
					}
			}

			// Vertical part
			i = 0;
			for (int lx = r.x; lx < r.x + v_thick; lx++, i++)  {
				cl = Color(v_in.r + Round(i * v_rstep), v_in.g + Round(i * v_gstep), v_in.b + Round(i * v_bstep), v_in.a + Round(i * v_astep)).get(dest->format);
				for (int ly = r.y + Round((v_thick - i) * v_ln_step) - 1; ly >= r.y; ly--)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}
		} break;

		case 4:  { // Bottom right
			// Horizontal part
			i = 0;
			for (int ly = r.y; ly < r.y + h_thick; ly++, i++)  {
				cl = Color(h_out.r - Round(i * h_rstep), h_out.g - Round(i * h_gstep), h_out.b - Round(i * h_bstep), h_out.a - Round(i * h_astep)).get(dest->format);
				for (int lx = r.x + Round(i * h_ln_step); lx >= r.x; lx--)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}

			// Vertical part
			i = 0;
			for (int lx = r.x; lx < r.x + v_thick; lx++, i++)  {
				cl = Color(v_out.r - Round(i * v_rstep), v_out.g - Round(i * v_gstep), v_out.b - Round(i * v_bstep), v_out.a - Round(i * v_astep)).get(dest->format);
				for (int ly = r.y +  Round(i * v_ln_step); ly >= r.y; ly--)  {
					if (PointInRect(lx, ly, dest->clip_rect))
						PutPixel(dest, lx, ly, cl);
				}
			}
		} break;
	}
}

////////////////////////
// Draws the top-left corner
void CBorder::DrawTopLeftCorner(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	// Bitmap
	if (CornerTopLeft.bmpCorner.get().get())
		DrawImage(bmpDest, CornerTopLeft.bmpCorner.get(), r.x, r.y);

	else {
		DrawBorderCorner(bmpDest, getTopLeftR(r), CornerTopLeft.iRoundRadius, BorderLeft.iThickness, BorderTop.iThickness, BorderLeft.clDark, BorderLeft.clLight, 
			BorderTop.clDark, BorderTop.clLight, 1);
	}
}

////////////////////////
// Draws the top-right corner
void CBorder::DrawTopRightCorner(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	int x = r.x + r.w;

	// Bitmap
	if (CornerTopRight.bmpCorner.get().get())
		DrawImage(bmpDest, CornerTopRight.bmpCorner.get(), x - CornerTopRight.bmpCorner->w, r.y);

	// Line
	else {
		DrawBorderCorner(bmpDest, getTopRightR(r), CornerTopRight.iRoundRadius, BorderRight.iThickness, BorderTop.iThickness, BorderRight.clDark, BorderRight.clLight, 
			BorderTop.clDark, BorderTop.clLight, 2);
	}
}

////////////////////////
// Draws the bottom-left corner
void CBorder::DrawBottomLeftCorner(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	int y = r.y + r.h;

	// Bitmap
	if (CornerBottomLeft.bmpCorner.get().get())
		DrawImage(bmpDest, CornerBottomLeft.bmpCorner.get(), r.x, y - CornerBottomLeft.bmpCorner->h);

	// Line
	else {
		DrawBorderCorner(bmpDest, getBottomLeftR(r), CornerBottomLeft.iRoundRadius, BorderLeft.iThickness, BorderBottom.iThickness, BorderLeft.clDark, BorderLeft.clLight, 
			BorderBottom.clDark, BorderBottom.clLight, 3);
	}
}


/////////////////////////
// Draws the bottom-right corner
void CBorder::DrawBottomRightCorner(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	int y = r.y + r.h;
	int x = r.x + r.w;

	// Bitmap
	if (CornerBottomRight.bmpCorner.get().get())
		DrawImage(bmpDest, CornerBottomRight.bmpCorner.get(), x - CornerBottomRight.bmpCorner->w, y - CornerBottomRight.bmpCorner->h);

	// Line
	else {
		DrawBorderCorner(bmpDest, getBottomRightR(r), CornerBottomRight.iRoundRadius, BorderRight.iThickness, BorderBottom.iThickness, BorderRight.clDark, BorderRight.clLight, 
			BorderBottom.clDark, BorderBottom.clLight, 4);
	}
}

/////////////////////////
// Draws a border line
void DrawBorderLine(SDL_Surface *bmpDest, int x1, int x2, int y1, int y2, int thick, SDL_Surface *line, BorderStyle style, Color cl_in, Color cl_out, bool vertical)
{
	// Adjust the colors according to the style
	switch (style)  {
	case brdSolid:
		cl_out = cl_in;
	break;
	case brdOutset:
	break;
	case brdInset:  {
		Color tmp = cl_in;
		cl_in = cl_out;
		cl_out = tmp;
	} break;
	}

	// Bitmap
	if (line)  {
		if (vertical)
			DrawImageTiledY(bmpDest, line, 0, 0, line->w, line->h, x1, y1, x2 - x1, y2 - y1);
		else
			DrawImageTiledX(bmpDest, line, 0, 0, line->w, line->h, x1, y1, x2 - x1, y2 - y1);

	// Color(s)
	} else
		DrawLinearGradient(bmpDest, x1, y1, x2 - x1, y2 - y1, cl_in, cl_out, vertical ? grdHorizontal : grdVertical);
}

void CBorder::DrawLeftLine(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	const SDL_Rect& topleft_r = getTopLeftR(r);
	const SDL_Rect& botleft_r = getBottomLeftR(r);
	DrawBorderLine(bmpDest, r.x, r.x + BorderLeft.iThickness, topleft_r.y + topleft_r.h, botleft_r.y, BorderLeft.iThickness,
		BorderLeft.bmpLine.get().get(), BorderLeft.iStyle, BorderLeft.clDark, BorderLeft.clLight, true);

}

void CBorder::DrawRightLine(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	const SDL_Rect& topright_r = getTopRightR(r);
	const SDL_Rect& botright_r = getBottomRightR(r);
	DrawBorderLine(bmpDest, r.x + r.w - BorderRight.iThickness, r.x + r.w, topright_r.y + topright_r.h, botright_r.y, BorderRight.iThickness,
		BorderRight.bmpLine.get().get(), BorderRight.iStyle, BorderRight.clLight, BorderRight.clDark, true);
}

void CBorder::DrawTopLine(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	const SDL_Rect& topleft_r = getTopLeftR(r);
	const SDL_Rect& topright_r = getTopRightR(r);
	DrawBorderLine(bmpDest, topleft_r.x + topleft_r.w, r.x + r.w - topright_r.w, r.y, r.y + getTopW(), BorderTop.iThickness,
		BorderTop.bmpLine.get().get(), BorderTop.iStyle, BorderTop.clDark, BorderTop.clLight, false);
}

void CBorder::DrawBottomLine(SDL_Surface *bmpDest, const SDL_Rect& r)
{
	const SDL_Rect& botleft_r = getBottomLeftR(r);
	const SDL_Rect& botright_r = getBottomRightR(r);
	DrawBorderLine(bmpDest, botleft_r.x + botleft_r.w, r.x + r.w - botright_r.w, r.y + r.h - getBottomW(), r.y + r.h, BorderBottom.iThickness,
		BorderBottom.bmpLine.get().get(), BorderBottom.iStyle, BorderBottom.clLight, BorderBottom.clDark, false);
}

////////////////////////
// Draws the border
void CBorder::Draw(SDL_Surface *bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect)
{
	// Set the clipping rectangle
	SDL_Rect oldrect;
	if (cliprect)  {
		SDL_GetClipRect(bmpDest, &oldrect);
		SDL_SetClipRect(bmpDest, cliprect);
	}

	SDL_Rect r = { x, y, w, h };
	
	// Corners
	DrawTopLeftCorner(bmpDest, r);
	DrawTopRightCorner(bmpDest, r);
	DrawBottomLeftCorner(bmpDest, r);
	DrawBottomRightCorner(bmpDest, r);

	// Lines
	DrawLeftLine(bmpDest, r);
	DrawRightLine(bmpDest, r);
	DrawTopLine(bmpDest, r);
	DrawBottomLine(bmpDest, r);


	// Restore the clipping rectangle
	if (cliprect)
		SDL_SetClipRect(bmpDest, &oldrect);
}

}; // namespace SkinnedGUI
