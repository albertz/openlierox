#include "gfx.h"
#include "gconsole.h"

#ifndef DEDICATED_ONLY
#include "2xsai.h"
#include "blitters/blitters.h"
#include "blitters/colors.h"
#include "blitters/macros.h"
#include "sprite_set.h"
#include "sprite.h"
#endif
#include <boost/bind.hpp>
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include "gusanos/allegro.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>

using namespace std;

Gfx gfx;

namespace
{
	bool m_initialized = false;

#ifndef DEDICATED_ONLY
	ALLEGRO_BITMAP* m_doubleResBuffer = 0;
	SpriteSet* mouseCursor = 0;
	
	string screenShot(const list<string> &args)
	{
		int nameIndex = 0;
		
		string filename;
		do
		{
			string ssIndex = cast<string>(nameIndex);
			while ( ssIndex.size() < 3 )
			{
				ssIndex = "0" + ssIndex;
			}
			filename = "screenshots/ss" + ssIndex + ".png";
			++nameIndex;
		} while( gusExists( filename.c_str() ) );
		
		ALLEGRO_BITMAP * tmpbitmap = create_bitmap_ex(24,screen->w,screen->h);
		blit(screen,tmpbitmap,0,0,0,0,screen->w,screen->h);
		bool success = gfx.saveBitmap( filename.c_str(),tmpbitmap);
		destroy_bitmap(tmpbitmap);
		
		if ( success )
			return "SCREENSHOT SAVED AS: " + filename;
		else 
			return "UNABLE TO SAVE SCREENSHOT";
	}	
#endif
}

Gfx::Gfx()
#ifndef DEDICATED_ONLY
: buffer(NULL)
#endif
{
}

Gfx::~Gfx()
{
	if (m_doubleResBuffer)  {
		destroy_bitmap(m_doubleResBuffer);
		m_doubleResBuffer = NULL;
	}
}


void Gfx::init()
{	
#ifndef DEDICATED_ONLY	
	Init_2xSaI(32); // needed for SUPER2XSAI and SUPEREAGLE filters
	
	// we don't use this buffer anymore in game. however, some code
	// uses it and too lazy to cleanup...
	buffer = create_bitmap(0,0);
#endif

	m_initialized = true; // Tell console commands it's safe to manipulate gfx
}

void Gfx::shutDown()
{
#ifndef DEDICATED_ONLY
	destroy_bitmap(buffer); buffer = 0;
#endif
}

void Gfx::registerInConsole()
{
#ifndef DEDICATED_ONLY
	console.registerCommands()
		("SCREENSHOT", screenShot)
	;
	
	console.registerVariables()
		("VID_DISTORTION_AA", &m_distortionAA, 1)
	;
#endif
}

void Gfx::loadResources()
{
#ifndef DEDICATED_ONLY
	mouseCursor = spriteList.load("cursor");
#endif
}

ALLEGRO_BITMAP* Gfx::loadBitmap( const string& filename, bool keepAlpha , bool stretch2)
{
	ALLEGRO_BITMAP* returnValue = NULL;
	/*
	int flags = COLORCONV_DITHER | COLORCONV_KEEP_TRANS;
	
	if(keepAlpha)
		flags |= COLORCONV_KEEP_ALPHA;
	else
		flags |= COLORCONV_TOTAL;
*/
	int flags = 0;
	
	LocalSetColorConversion cc(flags);
	
	if ( gusExists( filename.c_str() ) )
	{
		returnValue = load_bitmap(filename, stretch2);
	}
	else
	{
		string tmp = filename;
		tmp += ".png";
		if ( gusExists( tmp ) )
		{
			returnValue = load_bitmap(tmp, stretch2);
		}
		else
		{
			tmp = filename;
			tmp += ".bmp";
			if ( gusExists( tmp ))
			{
				returnValue = load_bitmap(tmp, stretch2);
			}
		}
	}

#ifndef DEDICATED_ONLY
	if(returnValue && !keepAlpha && bitmap_color_depth(returnValue) == 32 && get_color_depth() == 32)
	{
		typedef Pixel32 pixel_t_1;
		APPLY_ON_BITMAP(returnValue,
			RECT_Y_LOOP(
				RECT_X_LOOP(
					//*p &= 0xFFFFFF;
					*p |= makecol(0,0,0);
				)
			)
		);
	}
#endif
	return returnValue;
}

bool Gfx::saveBitmap( const string &filename,ALLEGRO_BITMAP* image)
{
	bool returnValue = false;
	
	//if ( !save_bitmap(filename.c_str(), image, palette) ) returnValue = true;
	
	return returnValue;
}

Gfx::operator bool()
{
	return m_initialized;
}


