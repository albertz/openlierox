#ifndef DEDSERV

#include "blitters.h"
#include "colors.h"
#include "macros.h"

#include <algorithm>

namespace Blitters
{

void putpixel_add_16(BITMAP* where, int x, int y, Pixel color1)
{
	Pixel16* p = ((Pixel16 *)where->line[y]) + x;

	*p = addColors_16_2(*p, color1);
}

void putpixel_addFact_16(BITMAP* where, int x, int y, Pixel color1, int fact)
{
	Pixel16* p = ((Pixel16 *)where->line[y]) + x;
	
	fact = (fact + 7) / 8;

	*p = addColors_16_2(*p, scaleColor_16(color1, fact));
}

void putpixel_addFact_32(BITMAP* where, int x, int y, Pixel color1, int fact)
{
	Pixel32* p = ((Pixel32 *)where->line[y]) + x;

	*p = addColorsCrude_32(*p, scaleColor_32(color1, fact));
}

void putpixel_add_32(BITMAP* where, int x, int y, Pixel color1)
{
	Pixel32* p = ((Pixel32 *)where->line[y]) + x;

	*p = addColorsCrude_32(*p, color1);
}


void putpixelwu_add_32(BITMAP* where, float x, float y, Pixel color1, int fact)
{
	int xf = int(x * 256.f);
	int yf = int(y * 256.f);
	
	int xi = xf >> 8;
	int yi = yf >> 8;
	
	if((unsigned int)xi < (unsigned int)where->w - 1
	&& (unsigned int)yi < (unsigned int)where->h - 1)
	{
		int fx = xf & 0xFF;
		int fy = yf & 0xFF;
		
		Pixel32* urow = ((Pixel32 *)where->line[yi]) + xi;
		Pixel32* lrow = ((Pixel32 *)where->line[yi + 1]) + xi;
		
		int fyf = fy * fact;
		int fxf = fx * fact;
		int fxfs = fxf >> 8;
		int fyfs = fyf >> 8;
		
		int flr = (fx * fyf) >> 16;  //x * y * fact
		int fll = fyfs - flr;        //(1-x) * y * fact
		int fur = fxfs - flr;        //(1-y) * x * fact
		int ful = fact - fxfs - fll; //(1-x) * (1-y) * fact

		urow[0] = addColorsCrude_32(urow[0], scaleColor_32(color1, ful));
		urow[1] = addColorsCrude_32(urow[1], scaleColor_32(color1, fur));
		lrow[0] = addColorsCrude_32(lrow[0], scaleColor_32(color1, fll));
		lrow[1] = addColorsCrude_32(lrow[1], scaleColor_32(color1, flr));
	}
}

void putpixelwu_add_16(BITMAP* where, float x, float y, Pixel color1, int fact)
{
	int xf = int(x * 32.f);
	int yf = int(y * 32.f);
	
	int xi = xf >> 5;
	int yi = yf >> 5;
	
	if((unsigned int)xi < (unsigned int)(where->w - 1)
	&& (unsigned int)yi < (unsigned int)(where->h - 1))
	{
		int fx = xf & 31;
		int fy = yf & 31;
		
		Pixel16_2* urow = (Pixel16_2 *)((Pixel16 *)where->line[yi] + xi);
		Pixel16_2* lrow = (Pixel16_2 *)((Pixel16 *)where->line[yi + 1] + xi);
		
		int fyf = fy * fact;
		int fxf = fx * fact;
		int fxfs = fxf >> 5;
		int fyfs = fyf >> 5;
		
		int flr = (fx * fyf) >> 10;  //x * y * fact
		int fll = fyfs - flr;        //(1-x) * y * fact
		int fur = fxfs - flr;        //(1-y) * x * fact
		int ful = fact - fxfs - fll; //(1-x) * (1-y) * fact
		
		Pixel usrc = packColors_16(scaleColor_16(color1, ful), scaleColor_16(color1, fur));
		Pixel lsrc = packColors_16(scaleColor_16(color1, fll), scaleColor_16(color1, flr));

		urow[0] = addColors_16_2(urow[0], usrc);
		lrow[0] = addColors_16_2(lrow[0], lsrc);
	}
}

void rectfill_add_16(BITMAP* where, int x1, int y1, int x2, int y2, Pixel colour, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16_2 pixel_t_2;
	
	CLIP_RECT();
	
	fact = (fact + 7) / 8;
	
	Pixel col = duplicateColor_16(scaleColor_16(colour, fact));
	Pixel colA, colB;
	prepareAddColors_16_2(col, colA, colB);
	
	RECT_Y_LOOP(
		RECT_X_LOOP_ALIGN(2, 4,
			*p = addColors_16_2(*p, colA, colB),
			*p = addColors_16_2(*p, colA, colB)
		)
	)
}

void rectfill_add_32(BITMAP* where, int x1, int y1, int x2, int y2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	CLIP_RECT();
	
	Pixel col = scaleColor_32(colour, fact);

	RECT_Y_LOOP(
		RECT_X_LOOP(
			*p = addColorsCrude_32(*p, col)
		)
	)
}

void hline_add_16(BITMAP* where, int x1, int y1, int x2, Pixel colour, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16_2 pixel_t_2;

	fact = (fact + 7) / 8;
	
	Pixel col = duplicateColor_16(scaleColor_16(colour, fact));
	Pixel colA, colB;
	prepareAddColors_16_2(col, colA, colB);
	
	RECT_X_LOOP_ALIGN(2, 4,
		*p = addColors_16_2(*p, colA, colB),
		*p = addColors_16_2(*p, colA, colB)
	)
}

void hline_add_32(BITMAP* where, int x1, int y1, int x2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	Pixel col = scaleColor_32(colour, fact);

	RECT_X_LOOP(
		*p = addColorsCrude_32(*p, col)
	)
}

bool linewu_add(BITMAP* where, float x, float y, float destx, float desty, Pixel colour, int fact)
{
	const long prec = 8;
	const long one = (1 << prec);
	const long half = one / 2;
	const long fracmask = one - 1;
	long x1 = long(x * one);
	long y1 = long(y * one);
	long x2 = long(destx * one);
	long y2 = long(desty * one);
	
	long xdiff = x2 - x1;
	long ydiff = y2 - y1;
	
	#define BLEND32(dest_, src_, fact_) addColorsCrude_32(dest_, scaleColor_32(src_, fact_))
	#define BLEND16(dest_, src_, fact_) addColors_16_2(dest_, scaleColor_16(src_, fact_))
			
	switch(bitmap_color_depth(where))
	{
		case 32:
			if(labs(xdiff) > labs(ydiff))
				WULINE(x, y, 0, 1, 32, 0, 256, BLEND32)
			else
				WULINE(y, x, 1, 0, 32, 0, 256, BLEND32)
		break;
		
		case 16:
			if(labs(xdiff) > labs(ydiff))
				WULINE(x, y, 0, 1, 16, 3, 32, BLEND16)
			else
				WULINE(y, x, 1, 0, 16, 3, 32, BLEND16)
		break;
	}

	#undef BLEND16
	#undef BLEND32

	return false;
}


void line_add(BITMAP* where, int x, int y, int destx, int desty, Pixel colour, int fact)
{
	#define BLEND32(dest_, src_) addColorsCrude_32(dest_, scaleColor_32(src_, fact))
	#define BLEND16(dest_, src_) addColors_16_2(dest_, scaleColor_16(src_, fact))
		
	switch(bitmap_color_depth(where))
	{
		case 32:
			LINE(BLEND32, 32);
		break;
		
		case 16:
			LINE(BLEND16, 16);
		break;
	}

	#undef BLEND16
	#undef BLEND32
}

void drawSprite_add_32(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	if(bitmap_color_depth(from) != 32)
		return;

	CLIP_SPRITE_REGION();
	
	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = addColorsCrude_32(*dest, s);
			)
		)
	}
	/*
	else if(fact >= 127 && fact <= 128)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = addColorsCrudeHalf_32(*dest, s);
			)
		)
	}
	*/
	else if(fact > 0)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = addColorsCrude_32(*dest, scaleColor_32(s, fact));
			)
		)
	}
	
}

void drawSprite_add_16(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16_2 pixel_t_2;

	if(bitmap_color_depth(from) != 16)
		return;

	CLIP_SPRITE_REGION();
	
	fact = (fact + 4) / 8;

	if(fact >= 31)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_ALIGN(2, 4,
				Pixel s = *src;
				if(s != maskcolor_16)
					*dest = addColors_16_2(*dest, *src)
			,
				*dest = addColors_16_2(*dest, add_mask_16_2(*src))
			)
		)
	}
	else if(fact > 0)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_ALIGN(2, 4,
				Pixel s = *src;
				if(s != maskcolor_16)
					*dest = addColors_16_2(*dest, scaleColor_16(s, fact))
			,
				*dest = addColors_16_2(*dest, scaleColor_16_2(add_mask_16_2(*src), fact));
			)
		)
	}
	
}

void drawSpriteLine_add_32(BITMAP* where, BITMAP* from, int x, int y, int x1, int y1, int x2, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	if(bitmap_color_depth(from) != 32)
		return;
		
	CLIP_HLINE();

	
	if(fact >= 255)
	{
		SPRITE_X_LOOP(
			Pixel s = *src;
			if(s != maskcolor_32)
				*dest = addColorsCrude_32(*dest, s);
		)
	}
	else if(fact > 0)
	{
		SPRITE_X_LOOP(
			Pixel s = *src;
			if(s != maskcolor_32)
				*dest = addColorsCrude_32(*dest, scaleColor_32(s, fact));
		)
	}
}

void drawSpriteLine_add_16(BITMAP* where, BITMAP* from, int x, int y, int x1, int y1, int x2, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16_2 pixel_t_2;

	if(bitmap_color_depth(from) != 16)
		return;
		
	CLIP_HLINE();

	fact = (fact + 4) / 8;
	
	if(fact >= 31)
	{
		SPRITE_X_LOOP_ALIGN(2, 4,
			Pixel s = *src;
			if(s != maskcolor_16)
				*dest = addColors_16_2(*dest, *src)
		,
			*dest = addColors_16_2(*dest, add_mask_16_2(*src))
		)
	}
	else if(fact > 0)
	{
		SPRITE_X_LOOP_ALIGN(2, 4,
			Pixel s = *src;
			if(s != maskcolor_16)
				*dest = addColors_16_2(*dest, scaleColor_16(s, fact))
		,
			*dest = addColors_16_2(*dest, scaleColor_16_2(add_mask_16_2(*src), fact));
		)
	}
}

void drawSpriteLine_add_8(BITMAP* where, BITMAP* from, int x, int y, int x1, int y1, int x2, int fact)
{
	typedef Pixel8 pixel_t_1;
	typedef Pixel8_4 pixel_t_2;

	if(bitmap_color_depth(from) != 8)
		return;
		
	CLIP_HLINE();
	
	if(fact >= 255)
	{
		SPRITE_X_LOOP_ALIGN(4, 4,
			*dest = addColorsCrude_8_4(*dest, *src)
		,
			*dest = addColorsCrude_8_4(*dest, *src)
		)
	}
	else if(fact > 0)
	{
		SPRITE_X_LOOP_ALIGN(4, 4,
			*dest = addColorsCrude_8_4(*dest, scaleColor_8_4(*src, fact))
		,
			*dest = addColorsCrude_8_4(*dest, scaleColor_8_4(*src, fact))
		)
	}
}

} //namespace Blitters

#endif
