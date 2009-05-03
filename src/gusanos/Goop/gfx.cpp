#include "gfx.h"
#include "gconsole.h"

#ifndef DEDSERV
#include "2xsai.h"
#include "blitters/blitters.h"
#include "blitters/colors.h"
#include "blitters/macros.h"
#include "mouse.h"
#include "sprite_set.h"
#include "sprite.h"
#endif
#include <boost/bind.hpp>
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <allegro.h>

#include <loadpng.h>

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

#ifndef DEDSERV
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
	int m_bitdepth = 32;

	BITMAP* m_doubleResBuffer = 0;
	SpriteSet* mouseCursor = 0;
	
	string screenShot(const list<string> &args)
	{
		int nameIndex = 0;
		
	#ifdef GLIPTIC_SCREENSHOT_HAX
		string filename;
		do
		{
			filename = "/usr/local/htdocs/stuff/gusanos-screens/ss" + cast<string>(nameIndex) + ".png";
			++nameIndex;
		} while( exists( filename.c_str() ) );
	#else
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
		} while( exists( filename.c_str() ) );
	#endif
		
		BITMAP * tmpbitmap = create_bitmap_ex(24,screen->w,screen->h);
		blit(screen,tmpbitmap,0,0,0,0,screen->w,screen->h);
		bool success = gfx.saveBitmap( filename.c_str(),tmpbitmap,0);
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
			clear_keybuf();
		}
	}
	
	void doubleRes( int oldValue )
	{
		if(m_doubleRes == oldValue)
			return;
			
		if(gfx)
		{
			gfx.doubleResChange();
			clear_keybuf();
		}
	}
#endif
}

#ifndef DEDSERV

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
#ifndef DEDSERV
: buffer(NULL)
#endif
{
}

Gfx::~Gfx()
{
}


void Gfx::init()
{
	register_png_file_type();
	
#ifndef DEDSERV
	set_color_depth(m_bitdepth); //Ugh

	doubleResChange(); // This calls fullscreenChange() that sets the gfx mode

	loadpng_init();
	
	Init_2xSaI(m_bitdepth); // needed for SUPER2XSAI and SUPEREAGLE filters
	
	buffer = create_bitmap(320,240);
#else
	set_color_depth(32);
#endif

	m_initialized = true; // Tell console commands it's safe to manipulate gfx
}

void Gfx::shutDown()
{
#ifndef DEDSERV
	destroy_bitmap(buffer); buffer = 0;
#endif
}

void Gfx::registerInConsole()
{
#ifndef DEDSERV
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
		("VID_BITDEPTH", &m_bitdepth, 32)
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
#else //def LINUX   ..or?
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
#ifndef DEDSERV
	mouseCursor = spriteList.load("cursor");
#endif
}
#ifndef DEDSERV

void Gfx::updateScreen()
{
	if(mouseCursor)
	{
		int x = mouseHandler.getX();
		int y = mouseHandler.getY();
		mouseCursor->getSprite()->draw(buffer, x, y);
	}
	//show_mouse(0);
	
	if ( m_vsync ) vsync();
	if ( !m_doubleRes )
	{
		blit(buffer,screen,0,0,0,0,m_vwidth,m_vheight);
	}
	else
	{
		bool blitFromBuffer = true;
		switch ( (Filters)m_filter )
		{
			case NO_FILTER:
				//blit(buffer, videobuffer, 0, 0, 0, 0, 320, 240);
				//stretch_blit(videobuffer, screen, 0, 0, videobuffer->w, videobuffer->h, 0, 0, screen->w, screen->h);
				
				switch(bitmap_color_depth(screen))
				{
					case 32:
					{
						acquire_screen();

						bmp_select(screen);
						
						for(int y = 0; y < 240; ++y)
						{
							Pixel32* src = (Pixel32 *)buffer->line[y];

							unsigned long dest1 = bmp_write_line(screen, y*2);
							
										
							for(int x = 0; x < 320; ++x)
							{
								Pixel p = *src++;
								
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
								
							}
							
							src = (Pixel32 *)buffer->line[y];
							dest1 = bmp_write_line(screen, y*2 + 1);
							
							for(int x = 0; x < 320; ++x)
							{
								Pixel p = *src++;
								
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
							}
							
							
						}
						
						bmp_unwrite_line(screen);
						
						release_screen();
					}
					break;
					
					case 16:
						acquire_screen();

						bmp_select(screen);

						for(int y = 0; y < 240; ++y)
						{
							// This is done in two loops to avoid shearing
							Pixel16* src = (Pixel16 *)buffer->line[y];
							
							unsigned long dest1 = bmp_write_line(screen, y*2);
														
							for(int x = 0; x < 320; ++x)
							{
								Pixel32 p = (Pixel32)*src++;
								
								p = p | (p << 16);

								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
							}
							
							src = (Pixel16 *)buffer->line[y];
							
							dest1 = bmp_write_line(screen, y*2 + 1);
							
							for(int x = 0; x < 320; ++x)
							{
								Pixel32 p = (Pixel32)*src++;
								
								p = p | (p << 16);

								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
							}
						}
						
						bmp_unwrite_line(screen);
						
						release_screen();
					break;
					
					default:
						stretch_blit(buffer, screen, 0, 0, buffer->w, buffer->h, 0, 0, screen->w, screen->h);
					break;
				}

				blitFromBuffer = false;
			break;
			
			case NO_FILTER2:
				
				switch(bitmap_color_depth(screen))
				{
					case 32:
					{
						acquire_screen();

						bmp_select(screen);
						
						for(int y = 0; y < 240; ++y)
						{
							Pixel32* src = (Pixel32 *)buffer->line[y];

							unsigned long dest1 = bmp_write_line(screen, y*2);
							unsigned long dest2 = bmp_write_line(screen, y*2 + 1);
							
										
							for(int x = 0; x < 320; ++x)
							{
								Pixel p = *src++;
								
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
								bmp_write32(dest2, p); dest2 += sizeof(Pixel32);
								bmp_write32(dest2, p); dest2 += sizeof(Pixel32);
							}
						}
						
						bmp_unwrite_line(screen);
						
						release_screen();
					}
					break;
					
					case 16:
						acquire_screen();

						bmp_select(screen);

						for(int y = 0; y < 240; ++y)
						{
							// This is done in two loops to avoid shearing
							Pixel16* src = (Pixel16 *)buffer->line[y];
							
							unsigned long dest1 = bmp_write_line(screen, y*2);
														
							for(int x = 0; x < 320; ++x)
							{
								Pixel32 p = (Pixel32)*src++;
								
								p = p | (p << 16);

								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
							}
							
							src = (Pixel16 *)buffer->line[y];
							
							dest1 = bmp_write_line(screen, y*2 + 1);
							
							for(int x = 0; x < 320; ++x)
							{
								Pixel32 p = (Pixel32)*src++;
								
								p = p | (p << 16);

								bmp_write32(dest1, p); dest1 += sizeof(Pixel32);
							}
						}
						
						bmp_unwrite_line(screen);
						
						release_screen();
					break;
					
					default:
						stretch_blit(buffer, screen, 0, 0, buffer->w, buffer->h, 0, 0, screen->w, screen->h);
					break;
				}

				blitFromBuffer = false;
			break;
			
			case SCANLINES: 
			/*
				acquire_screen();
				for ( int i = 0; i < buffer->h; ++i )
				{
					stretch_blit(buffer, screen, 0, i, buffer->w, 1, 0, i*2, screen->w, 1);
					hline(screen, 0, i*2+1, screen->w, 0);
				}
				release_screen();*/
				switch(bitmap_color_depth(screen))
				{
					case 32:
						acquire_screen();

						bmp_select(screen);

						for(int y = 0; y < 240; ++y)
						{
							unsigned long* src = (unsigned long *)buffer->line[y];

							unsigned long dest1 = bmp_write_line(screen, y*2);
							unsigned long dest2 = bmp_write_line(screen, y*2 + 1);
														
							for(int x = 0; x < 320; ++x)
							{
								unsigned long p = *src++;

								bmp_write32(dest1, p); dest1 += sizeof(unsigned long);
								bmp_write32(dest1, p); dest1 += sizeof(unsigned long);
								bmp_write32(dest2, 0); dest2 += sizeof(unsigned long);
								bmp_write32(dest2, 0); dest2 += sizeof(unsigned long);
							}
						}
						
						bmp_unwrite_line(screen);
						
						release_screen();
					break;
					
					default:
						acquire_screen();
						for ( int i = 0; i < buffer->h; ++i )
						{
							stretch_blit(buffer, screen, 0, i, buffer->w, 1, 0, i*2, screen->w, 1);
							hline(screen, 0, i*2+1, screen->w, 0);
						}
						release_screen();
					break;
				}
				blitFromBuffer = false;
			break;
			
			case SCANLINES2: 
				//blit(buffer, m_doubleResBuffer, 0, 0, 0, 0, buffer->w, buffer->h);
				acquire_screen();
				for ( int i = 0; i < buffer->h; ++i )
				{
					stretch_blit(buffer, screen, 0, i, buffer->w, 1, 0, i*2, screen->w, 1);
					//stretch_blit(m_doubleResBuffer, screen, 0, i+1, buffer->w, 1, 0, i*2+1, screen->w, 1);
				}
				drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
				set_trans_blender(0, 0, 0, 128);
				rectfill( buffer, 0, 0, buffer->w, buffer->h, 0 );
				solid_mode();
				//acquire_screen();
				for ( int i = 0; i < buffer->h; ++i )
				{
					//stretch_blit(buffer, screen, 0, i, buffer->w, 1, 0, i*2, screen->w, 1);
					stretch_blit(buffer, screen, 0, i, buffer->w, 1, 0, i*2+1, screen->w, 1);
				}
				release_screen();
				blitFromBuffer = false;
			break;
			
			case BILINEAR:
			{
				switch(bitmap_color_depth(screen))
				{
					case 32:
					{
						typedef Pixel32 pixel_t_1;
						
						FILTER_2X_TO_VIDEO(
							bmp_write32(dest, ul)
						,
							bmp_write32(dest, Blitters::blendColorsHalfCrude_32(ul, ur))
						,
							bmp_write32(dest, Blitters::blendColorsHalfCrude_32(ul, ll))
						,
							bmp_write32(dest, Blitters::blendColorsHalfCrude_32(
								Blitters::blendColorsHalfCrude_32(ul, ur),
								Blitters::blendColorsHalfCrude_32(ll, lr)))
						)
					}
					break;
					
					case 16:
					{
						typedef Pixel16 pixel_t_1;
						
						FILTER_2X_TO_VIDEO(
							bmp_write16(dest, ul)
						,
							bmp_write16(dest, Blitters::blendColorsHalf_16_2(ul, ur))
						,
							bmp_write16(dest, Blitters::blendColorsHalf_16_2(ul, ll))
						,
							bmp_write16(dest, Blitters::blendColorsHalf_16_2(
								Blitters::blendColorsHalf_16_2(ul, ur),
								Blitters::blendColorsHalf_16_2(ll, lr)))
						)
					}
					break;
				}
				
				blitFromBuffer = false;
			}
			break;
			
			case SUPER2XSAI:
				Super2xSaI(buffer, m_doubleResBuffer, 0, 0, 0, 0, 320, 240);
				blitFromBuffer = true;
			break;

			case SUPEREAGLE:
				SuperEagle(buffer, m_doubleResBuffer, 0, 0, 0, 0, 320, 240);
				blitFromBuffer = true;
			break;

			case PIXELATE:
				switch(bitmap_color_depth(screen))
				{
					case 32:
					{
						typedef Pixel32 pixel_t_1;
						
						FILTER_2X_TO_VIDEO(
							bmp_write32(dest, ul)
						,
							bmp_write32(dest, 0)
						,
							bmp_write32(dest, 0)
						,
							bmp_write32(dest, 0)
						)
					}
					break;
					
					case 16:
					{
						typedef Pixel16 pixel_t_1;
						
						FILTER_2X_TO_VIDEO(
							bmp_write16(dest, ul)
						,
							bmp_write16(dest, 0)
						,
							bmp_write16(dest, 0)
						,
							bmp_write16(dest, 0)
						)
					}
					break;
				}
				blitFromBuffer = false;
			break;
			
		}
		if(blitFromBuffer)
			blit(m_doubleResBuffer, screen, 0, 0, 0, 0, m_vwidth, m_vheight);
	}
	if ( m_clearBuffer ) clear_bitmap(buffer);
	//show_mouse(screen);
}

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
	set_color_depth(m_bitdepth);
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

	int flags = COLORCONV_DITHER | COLORCONV_KEEP_TRANS;
	
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

#ifndef DEDSERV
	if(returnValue && !keepAlpha && bitmap_color_depth(returnValue) == 32 && get_color_depth() == 32)
	{
		typedef Pixel32 pixel_t_1;
		APPLY_ON_BITMAP(returnValue,
			RECT_Y_LOOP(
				RECT_X_LOOP(
					*p &= 0xFFFFFF;
				)
			)
		);
	}
#endif
	return returnValue;
}

bool Gfx::saveBitmap( const string &filename,BITMAP* image, RGB* palette )
{
	bool returnValue = false;
	
	if ( !save_bitmap(filename.c_str(), image, palette) ) returnValue = true;
	
	return returnValue;
}

Gfx::operator bool()
{
	return m_initialized;
}


