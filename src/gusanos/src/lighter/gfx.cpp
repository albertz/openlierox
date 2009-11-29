#include "gfx.h"

#include <boost/bind.hpp>
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <allegro.h>

#include "../loadpng/loadpng.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>

using namespace std;

Gfx gfx;

Gfx::Gfx()
{
}

Gfx::~Gfx()
{
}

void Gfx::init()
{
	register_png_file_type();

	loadpng_init();

	set_color_depth(32);
}

BITMAP* Gfx::loadBitmap( const string& filename, RGB* palette, bool keepAlpha )
{
	BITMAP* returnValue = NULL;
	
#ifndef COLORCONV_KEEP_ALPHA
	const int COLORCONV_KEEP_ALPHA = COLORCONV_EXPAND_256 |
			COLORCONV_15_TO_8 |
			COLORCONV_16_TO_8 |
			COLORCONV_24_TO_8 |
			COLORCONV_32_TO_8 |
			COLORCONV_EXPAND_15_TO_16 |
			COLORCONV_REDUCE_16_TO_15 |
			COLORCONV_EXPAND_HI_TO_TRUE |
			COLORCONV_REDUCE_TRUE_TO_HI |
			COLORCONV_24_EQUALS_32;
#endif

	int flags = COLORCONV_DITHER;
	
	if(keepAlpha)
		flags |= COLORCONV_KEEP_ALPHA;
	else
		flags |= COLORCONV_TOTAL;

	LocalSetColorConversion cc(flags);
	
	if ( exists( filename.c_str() ) )
	{
		returnValue = load_bitmap(filename.c_str(), palette);
	}
	else
	{
		string tmp = filename;
		tmp += ".png";
		if ( exists( tmp.c_str() ) )
		{
			returnValue = load_png( tmp.c_str() , palette );
		}
		else
		{
			tmp = filename;
			tmp += ".bmp";
			if ( exists( tmp.c_str() ))
			{
				returnValue = load_bmp( tmp.c_str() , palette );
			}
		}
	}
	
	return returnValue;
}

bool Gfx::saveBitmap( const string &filename,BITMAP* image, RGB* palette )
{
	bool returnValue = false;
	
	if ( !save_bitmap(filename.c_str(), image, palette) ) returnValue = true;
	
	return returnValue;
}
