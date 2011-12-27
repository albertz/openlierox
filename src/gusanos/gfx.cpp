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
	enum Filters
	{
		NO_FILTER,
		NO_FILTER2,
		SCANLINES,
		SCANLINES2,
		BILINEAR,
		SUPER2XSAI,
		SUPEREAGLE,
		PIXELATE
	};
	
	int m_fullscreen = 1;
	int m_doubleRes = 1;
	int m_vwidth = 320;
	int m_vheight = 240;
	int m_vsync = 0;
	int m_clearBuffer = 0;
	int m_filter = NO_FILTER;
	int m_driver = GFX_AUTODETECT;

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
	
	void fullscreen( int oldValue )
	{
		if(m_fullscreen == oldValue)
			return;
			
		if(gfx)
		{
			gfx.fullscreenChange();
		}
	}
	
	void doubleRes( int oldValue )
	{
		if(m_doubleRes == oldValue)
			return;
			
		if(gfx)
		{
			gfx.doubleResChange();
		}
	}
#endif
}

#ifndef DEDICATED_ONLY

/*
void Gfx::fullscreen( int oldValue )
{
	if(m_fullscreen == oldValue)
		return;
		
	if(*this)
	{
		fullscreenChange();
		clear_keybuf();
	}
}

void Gfx::doubleRes( int oldValue )
{
	if(m_doubleRes == oldValue)
		return;
		
	if(*this)
	{
		doubleResChange();
		clear_keybuf();
	}
}
*/
#endif
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
	doubleResChange(); // This calls fullscreenChange() that sets the gfx mode
	
	Init_2xSaI(32); // needed for SUPER2XSAI and SUPEREAGLE filters
	
	buffer = create_bitmap(640,480); // this is because we may need up to this size for the sizefactor
	//buffer = create_bitmap(320,240);
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
		//("VID_FULLSCREEN", &m_fullscreen, 0, boost::bind(&Gfx::fullscreen, this, _1))
		("VID_FULLSCREEN", &m_fullscreen, 0, fullscreen)
		//("VID_DOUBLERES", &m_doubleRes, 0, boost::bind(&Gfx::doubleRes, this, _1))
		("VID_DOUBLERES", &m_doubleRes, 0, doubleRes)
		("VID_VSYNC", &m_vsync, 0)
		("VID_CLEAR_BUFFER", &m_clearBuffer, 0)
		//("VID_BITDEPTH", &m_bitdepth, 32)
		("VID_DISTORTION_AA", &m_distortionAA, 1)
		("VID_HAX_WORMLIGHT", &m_haxWormLight, 1)
		//("VID_DARK_MODE", &darkMode, 0)
	;
	
	// NOTE: When/if adding a callback to gfx variables, make it do nothing if
	// gfx.operator bool() returns false.
	
	{
		EnumVariable::MapType videoFilters;

		insert(videoFilters) // These neat boost::assign functions actually make it smaller than!
			("NOFILTER", NO_FILTER)
			("NOFILTER2", NO_FILTER2)
			("SCANLINES", SCANLINES)
			("SCANLINES2", SCANLINES2)
			("BILINEAR", BILINEAR)
			("SUPER2XSAI",SUPER2XSAI)
			("SUPEREAGLE", SUPEREAGLE)
			("PIXELATE", PIXELATE)
		;

		console.registerVariable(new EnumVariable("VID_FILTER", &m_filter, NO_FILTER, videoFilters));
	}
	
	{
		EnumVariable::MapType videoDrivers;
		
		insert(videoDrivers)
			("AUTO", GFX_AUTODETECT)
#ifdef WINDOWS
			("DIRECTX", GFX_DIRECTX)
#else //def LINUX
			("XDGA", GFX_XDGA)
			("XDGA2", GFX_XDGA2)
			("XWINDOWS", GFX_XWINDOWS)
#endif
		;

		console.registerVariable(new EnumVariable("VID_DRIVER", &m_driver, GFX_AUTODETECT, videoDrivers));
	}
#endif
}

void Gfx::loadResources()
{
#ifndef DEDICATED_ONLY
	mouseCursor = spriteList.load("cursor");
#endif
}
#ifndef DEDICATED_ONLY

int Gfx::getGraphicsDriver()
{
	int driverSelected = GFX_AUTODETECT;
	if ( m_fullscreen )
	{
		driverSelected = GFX_AUTODETECT_FULLSCREEN;
		switch ( m_driver )
		{
			case GFX_AUTODETECT: driverSelected = GFX_AUTODETECT_FULLSCREEN; break;
#ifdef WINDOWS
			case GFX_DIRECTX: driverSelected = GFX_DIRECTX; break;
#else //def LINUX   ..or?
			case GFX_XDGA: driverSelected = GFX_XDGA_FULLSCREEN; break;
			case GFX_XDGA2: driverSelected = GFX_XDGA2; break;
			case GFX_XWINDOWS: driverSelected = GFX_XWINDOWS_FULLSCREEN; break;
#endif

		}
	}
	else
	{
		driverSelected = GFX_AUTODETECT_WINDOWED;
		switch ( m_driver )
		{
			case GFX_AUTODETECT: driverSelected = GFX_AUTODETECT_WINDOWED; break;
#ifdef WINDOWS
			case GFX_DIRECTX: driverSelected = GFX_DIRECTX_WIN; break;
#else //ifdef LINUX   ..or?
			case GFX_XDGA: driverSelected = GFX_XDGA; break;
			case GFX_XDGA2: driverSelected = GFX_AUTODETECT_WINDOWED; break; //XDGA2 only works in fullscreen
			case GFX_XWINDOWS: driverSelected = GFX_XWINDOWS; break;
#endif
			// TODO: DirectX overlay support (GFX_DIRECTX_OVL)?
		}
	}
	
	return driverSelected;
}

void Gfx::fullscreenChange()
{
	//destroy_bitmap( videobuffer );
	
	// TODO: I suppose that changing graphics driver will clear out bitmaps and such
	
	int driver = getGraphicsDriver();

	int result = set_gfx_mode(driver, m_vwidth, m_vheight, 0, 0);
	if(result < 0)
	{
		cerr << "set_gfx_mode("
		<< driver << ", " << m_vwidth << ", " << m_vheight << ", 0, 0): " << allegro_error << endl;
		// We hit some error
		if(m_driver != GFX_AUTODETECT)
		{
			// If the driver wasn't at autodetect, revert to it and try again
			m_driver = GFX_AUTODETECT;
			result = set_gfx_mode(getGraphicsDriver(), m_vwidth, m_vheight, 0, 0);
		}
	}
	
	// Check if we still haven't got a graphics mode working
	if(result < 0)
		throw std::runtime_error("Couldn't set graphics mode");
	
	if(set_display_switch_mode(SWITCH_BACKAMNESIA) == -1)
		set_display_switch_mode(SWITCH_BACKGROUND);
			
	
	//videobuffer = create_video_bitmap(320, 240);
	//if(!videobuffer)
	//	cerr << ">:O" << endl;
}

void Gfx::doubleResChange()
{
	if( m_doubleResBuffer ) destroy_bitmap( m_doubleResBuffer );
	
	if ( m_doubleRes )
	{
		m_vwidth = 640;
		m_vheight = 480;
		m_doubleResBuffer = create_bitmap(m_vwidth, m_vheight );
		
	}else
	{
		m_vwidth = 320;
		m_vheight = 240;
		m_doubleResBuffer = NULL;
	}
	fullscreenChange();
}

int Gfx::getScalingFactor()
{
	return m_doubleRes ? 2 : 1;
}

#endif

ALLEGRO_BITMAP* Gfx::loadBitmap( const string& filename, bool keepAlpha )
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
		returnValue = load_bitmap(filename);
	}
	else
	{
		string tmp = filename;
		tmp += ".png";
		if ( gusExists( tmp ) )
		{
			returnValue = load_bitmap( tmp );
		}
		else
		{
			tmp = filename;
			tmp += ".bmp";
			if ( gusExists( tmp ))
			{
				returnValue = load_bitmap( tmp );
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


