#ifndef DEDSERV

#include "blitters.h"
#include "colors.h"
#include "macros.h"

#include <algorithm>
#include <iostream>
using std::cerr;
using std::endl;

namespace Blitters
{

void putpixel_blendHalf_32(BITMAP* where, int x, int y, Pixel color1)
{
	Pixel32* p = ((Pixel32 *)where->line[y]) + x;

	*p = blendColorsHalfCrude_32(*p, color1);
}

void putpixel_blend_32(BITMAP* where, int x, int y, Pixel color1, int fact)
{
	Pixel32* p = ((Pixel32 *)where->line[y]) + x;

	*p = blendColorsFact_32(*p, color1, fact);
}

void putpixelwu_blend_32(BITMAP* where, float x, float y, Pixel color1, int fact)
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

		urow[0] = blendColorsFact_32(urow[0], color1, ful);
		urow[1] = blendColorsFact_32(urow[1], color1, fur);
		lrow[0] = blendColorsFact_32(lrow[0], color1, fll);
		lrow[1] = blendColorsFact_32(lrow[1], color1, flr);
	}
}

void putpixelwu_blend_16(BITMAP* where, float x, float y, Pixel color1, int fact)
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
		
		Pixel16* urow = (Pixel16 *)where->line[yi] + xi;
		Pixel16* lrow = (Pixel16 *)where->line[yi + 1] + xi;
		
		int fyf = fy * fact;
		int fxf = fx * fact;
		int fxfs = fxf >> 5;
		int fyfs = fyf >> 5;
		
		int flr = (fx * fyf) >> 10;  //x * y * fact
		int fll = fyfs - flr;        //(1-x) * y * fact
		int fur = fxfs - flr;        //(1-y) * x * fact
		int ful = fact - fxfs - fll; //(1-x) * (1-y) * fact

		urow[0] = blendColorsFact_16(urow[0], color1, ful);
		urow[1] = blendColorsFact_16(urow[1], color1, fur);
		lrow[0] = blendColorsFact_16(lrow[0], color1, fll);
		lrow[1] = blendColorsFact_16(lrow[1], color1, flr);
	}
}

void putpixel_blendHalf_16(BITMAP* where, int x, int y, Pixel color1)
{
	Pixel16* p = ((Pixel16 *)where->line[y]) + x;

	*p = blendColorsHalf_16_2(*p, color1);
}

void putpixel_blend_16(BITMAP* where, int x, int y, Pixel color1, int fact)
{
	Pixel16* p = ((Pixel16 *)where->line[y]) + x;

	*p = blendColorsFact_16_2(*p, color1, fact);
}
	

void rectfill_blend_16(BITMAP* where, int x1, int y1, int x2, int y2, Pixel colour, int fact)
{
	typedef Pixel16   pixel_t_1;
	typedef Pixel16_2 pixel_t_2;

	CLIP_RECT();
	
	fact = (fact + 7) / 8;
	
	if(fact >= 127 && fact <= 128)
	{
		Pixel16_2 col = duplicateColor_16(colour);
		Pixel colA, colB;
		prepareBlendColorsHalf_16_2(col, colA, colB);
		
		RECT_Y_LOOP(
			RECT_X_LOOP_ALIGN(2, 4,
				*p = blendColorsHalf_16_2_prepared(*p, colA, colB),
				*p = blendColorsHalf_16_2_prepared(*p, colA, colB)
			)
		)
	}
	else
	{
		Pixel16_2 col = duplicateColor_16(colour);
		Pixel colA, colB;
		prepareBlendColorsFact_16_2(col, colA, colB);

		RECT_Y_LOOP(
			RECT_X_LOOP_ALIGN(2, 4,
				*p = blendColorsFact_16_2(*p, colA, colB, fact),
				*p = blendColorsFact_16_2(*p, colA, colB, fact)
			)
		)
	}
}

void rectfill_blend_32(BITMAP* where, int x1, int y1, int x2, int y2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	CLIP_RECT();
	
	if(fact >= 127 && fact <= 128)
	{
		Pixel colA;
		prepareBlendColorsHalfCrude_32(colour, colA);
		
		RECT_Y_LOOP(
			RECT_X_LOOP(
				*p = blendColorsHalfCrude_32(*p, colA)
			)
		)
	}
	else
	{
		//Pixel colA, colB;
		//prepareBlendColorsFact_32(colour, colA, colB);
		
		RECT_Y_LOOP(
			RECT_X_LOOP(
				*p = blendColorsFact_32(*p, colour, fact)
			)
		)
	}
}

void hline_blend_16(BITMAP* where, int x1, int y1, int x2, Pixel colour, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16_2 pixel_t_2;

	fact = (fact + 7) / 8;
	
	Pixel col = duplicateColor_16(colour);
	Pixel colA, colB;
	prepareBlendColorsFact_16_2(col, colA, colB);
	
	RECT_X_LOOP_ALIGN(2, 4,
		*p = blendColorsFact_16_2(*p, colA, colB, fact),
		*p = blendColorsFact_16_2(*p, colA, colB, fact)
	)
}

void hline_blend_32(BITMAP* where, int x1, int y1, int x2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	if(fact >= 255)
	{
		RECT_X_LOOP(
			*p = colour
		)
	}
	else if(fact > 0)
	{
		RECT_X_LOOP(
			*p = blendColorsFact_32(*p, colour, fact)
		)
	}
}

bool linewu_blend(BITMAP* where, float x, float y, float destx, float desty, Pixel colour, int fact)
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
	
	#define BLEND32(dest_, src_, fact_) blendColorsFact_32(dest_, src_, fact_)
	#define BLEND16(dest_, src_, fact_) blendColorsFact_16(dest_, src_, fact_)

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

void line_blend(BITMAP* where, int x, int y, int destx, int desty, Pixel colour, int fact)
{
	#define BLEND32(dest_, src_) blendColorsFact_32(dest_, src_, fact)
	#define BLEND16(dest_, src_) blendColorsFact_16(dest_, src_, fact)
		
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

void drawSprite_blend_32(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;
	
	if(bitmap_color_depth(from) != 32)
		return;

	CLIP_SPRITE_REGION();
	
	// Adjust to some suitable range where the difference isn't noticable
	if(fact >= 127 && fact <= 128) 
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				if(s != 0xFF00FF)
					*dest = blendColorsHalfCrude_32(*dest, s);
			)
		)
	}
	else
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				if(s != 0xFF00FF)
					*dest = blendColorsFact_32(*dest, s, fact);
			)
		)
	}
	
}

void drawSprite_blend_16(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel16   pixel_t_1;
	typedef Pixel16_2 pixel_t_2;
	
	if(bitmap_color_depth(from) != 16)
		return;

	CLIP_SPRITE_REGION();
	
	fact = (fact + 4) / 8;
	
	if(fact == 16)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_ALIGN(2, 4,
				Pixel s = *src;
				if(s != maskcolor_16)
					*dest = blendColorsHalf_16_2(*dest, s)
			,
				Pixel d = *dest;
				*dest = blendColorsHalf_16_2(d, blend_mask_16_2(d, *src))
			)
		)
	}
	else if(fact > 0)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_ALIGN(2, 4,
				Pixel s = *src;
				if(s != maskcolor_16)
					*dest = blendColorsFact_16_2(*dest, s, fact)
			,
				Pixel d = *dest;
				*dest = blendColorsFact_16_2(d, blend_mask_16_2(d, *src), fact)
			)
		)
	}
	
}

void drawSprite_blendalpha_32_to_32(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;

	if(fact <= 0)
		return;
		
	if(bitmap_color_depth(from) != 32)
		return;
	
	CLIP_SPRITE_REGION();
	
	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				*dest = blendColorsFact_32(*dest, s, (s >> 24))
			)
		)
	}
	else
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP(
				Pixel s = *src;
				*dest = blendColorsFact_32(*dest, s, (((s >> 24) * fact) >> 8))
			)
		)
	}
}

void drawSprite_blendalpha_32_to_16(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_src;
	typedef Pixel16 pixel_t_dest;
	
	if(fact <= 0)
		return;
		
	if(bitmap_color_depth(from) != 32)
	{
		return;
	}
	
	CLIP_SPRITE_REGION();
	
	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				Pixel s = *src;
				*dest = blendColorsFact_16(*dest, convertColor_32_to_16(s), (s >> (24+3)))
			)
		)
	}
	else
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				Pixel s = *src;
				*dest = blendColorsFact_16(*dest, convertColor_32_to_16(s), ((s >> 24) * fact) >> (8+3))
			)
		)
	}
}

void drawSprite_blendtint_8_to_32(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact, int color)
{
	typedef Pixel8 pixel_t_src;
	typedef Pixel32 pixel_t_dest;
	
	if(fact <= 0)
		return;
		
	if(bitmap_color_depth(from) != 8)
	{
		return;
	}
	
	CLIP_SPRITE_REGION();
	
	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				*dest = blendColorsFact_32(*dest, color, (int)*src)
			)
		)
	}
	else
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				Pixel s = *src;
				*dest = blendColorsFact_32(*dest, color, (s * fact) >> 8)
			)
		)
	}
}

void drawSprite_blendtint_8_to_16(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact, int color)
{
	typedef Pixel8 pixel_t_src;
	typedef Pixel16 pixel_t_dest;
	
	if(fact <= 0)
		return;
		
	if(bitmap_color_depth(from) != 8)
	{
		return;
	}
	
	CLIP_SPRITE_REGION();
		
	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				*dest = blendColorsFact_16(*dest, color, (int)*src >> 3)
			)
		)
	}
	else
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_T(
				Pixel s = *src;
				*dest = blendColorsFact_16(*dest, color, (s * fact) >> (8+3))
			)
		)
	}
}

} //namespace Blitters

#endif
