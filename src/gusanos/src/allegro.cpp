/*
 *  allegro.cpp
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

#include <SDL.h>
#include <SDL_image.h>
#include <boost/static_assert.hpp>

#include "allegro.h"
#include "FindFile.h"
#include "Debug.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "InputEvents.h"
#include "EventQueue.h"
#include "LieroX.h"




SDL_PixelFormat pixelformat32alpha =
{
NULL, //SDL_Palette *palette;
32, //Uint8  BitsPerPixel;
4, //Uint8  BytesPerPixel;
0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
0xff000000, 0xff0000, 0xff00, 0xff, //Uint32 Rmask, Gmask, Bmask, Amask;
0, //Uint32 colorkey;
255 //Uint8  alpha;
};

SDL_PixelFormat* mainPixelFormat = &pixelformat32alpha;




SDL_PixelFormat pixelformat[5];


/*
 * Calculate an 8-bit (3 red, 3 green, 2 blue) dithered palette of colors
 */
static void DitherColors(SDL_Color *colors, int bpp)
{
	int i;
	if(bpp != 8)
		return;         /* only 8bpp supported right now */
	
	for(i = 0; i < 256; i++) {
		int r, g, b;
		/* map each bit field to the full [0, 255] interval,
		 so 0 is mapped to (0, 0, 0) and 255 to (255, 255, 255) */
		r = i & 0xe0;
		r |= r >> 3 | r >> 6;
		colors[i].r = r;
		g = (i << 3) & 0xe0;
		g |= g >> 3 | g >> 6;
		colors[i].g = g;
		b = i & 0x3;
		b |= b << 2;
		b |= b << 4;
		colors[i].b = b;
	}
}

static void SetPixelFormat(SDL_PixelFormat& fmt, int bpp) {
	SDL_PixelFormat *format = &fmt;
	
	SDL_memset(format, 0, sizeof(*format));
	format->alpha = SDL_ALPHA_OPAQUE;
	
	/* Set up the format */
	format->BitsPerPixel = bpp;
	format->BytesPerPixel = (bpp+7)/8;
	if ( bpp > 8 ) {         /* Packed pixels with standard mask */
		/* R-G-B */
		if ( bpp > 24 )
			bpp = 24;
		format->Rloss = 8-(bpp/3);
		format->Gloss = 8-(bpp/3)-(bpp%3);
		format->Bloss = 8-(bpp/3);
		format->Rshift = ((bpp/3)+(bpp%3))+(bpp/3);
		format->Gshift = (bpp/3);
		format->Bshift = 0;
		format->Rmask = ((0xFF>>format->Rloss)<<format->Rshift);
		format->Gmask = ((0xFF>>format->Gloss)<<format->Gshift);
		format->Bmask = ((0xFF>>format->Bloss)<<format->Bshift);
	} else {
		/* Palettized formats have no mask info */
		format->Rloss = 8;
		format->Gloss = 8;
		format->Bloss = 8;
		format->Aloss = 8;
		format->Rshift = 0;
		format->Gshift = 0;
		format->Bshift = 0;
		format->Ashift = 0;
		format->Rmask = 0;
		format->Gmask = 0;
		format->Bmask = 0;
		format->Amask = 0;
	}
	if ( bpp <= 8 ) {                       /* Palettized mode */
		int ncolors = 1<<bpp;
#ifdef DEBUG_PALETTE
		fprintf(stderr,"bpp=%d ncolors=%d\n",bpp,ncolors);
#endif
		format->palette = (SDL_Palette *)SDL_malloc(sizeof(SDL_Palette));
		if ( format->palette == NULL ) {
			SDL_OutOfMemory();
			return;
		}
		(format->palette)->ncolors = ncolors;
		(format->palette)->colors = (SDL_Color *)SDL_malloc(
															(format->palette)->ncolors*sizeof(SDL_Color));
		if ( (format->palette)->colors == NULL ) {
			SDL_OutOfMemory();
			return;
		}
		if ( ncolors == 2 ) {
			/* Create a black and white bitmap palette */
			format->palette->colors[0].r = 0xFF;
			format->palette->colors[0].g = 0xFF;
			format->palette->colors[0].b = 0xFF;
			format->palette->colors[1].r = 0x00;
			format->palette->colors[1].g = 0x00;
			format->palette->colors[1].b = 0x00;
		} else if(ncolors == 256) {
			DitherColors(format->palette->colors, 8);
		} else {
			/* Create an empty palette */
			SDL_memset((format->palette)->colors, 0,
					   (format->palette)->ncolors*sizeof(SDL_Color));
		}
	}
}

static void init_pixelformats() {
	for(int bpp = 1; bpp <= 3; ++bpp) {
		SetPixelFormat(pixelformat[bpp], bpp * 8);
	}
	pixelformat[4] = pixelformat32alpha;
	
	mainPixelFormat = &pixelformat[4];
}

static int color_conversion = 0;
int get_color_conversion() { return color_conversion; }
void set_color_conversion(int mode) { color_conversion = mode; }

static int color_depth = 32;
int get_color_depth() { return color_depth; }
void set_color_depth(int depth) { color_depth = depth; }


static void sub_to_abs_coords_x(BITMAP* bmp, int& x) { x += bmp->sub_x; }
static void sub_to_abs_coords_y(BITMAP* bmp, int& y) { y += bmp->sub_y; }
static void sub_to_abs_coords(BITMAP* bmp, int& x, int& y) {
	sub_to_abs_coords_x(bmp, x);
	sub_to_abs_coords_y(bmp, y);
}

static BITMAP* create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf, int subx, int suby, int subw, int subh) {
	BITMAP* bmp = new BITMAP();
	
	bmp->surf = surf;
	bmp->sub_x = subx;
	bmp->sub_y = suby;
	bmp->w = subw;
	bmp->h = subh;
	bmp->cl = 0;
	bmp->cr = subw;
	bmp->ct = 0;
	bmp->cb = subh;
	
	bmp->line = new unsigned char* [subh];
	for(int y = 0; y < subh; ++y)
		bmp->line[y] = (Uint8 *)surf->pixels + ((suby + y) * surf->pitch) + (subx * surf->format->BytesPerPixel);

	return bmp;
}

static BITMAP *create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf) {	
	return create_bitmap_from_sdl(surf, 0, 0, surf->w, surf->h);
}

static void
graphics_dump_palette(SDL_Surface* p_bitmap)
{
    for(int l_i = 0; l_i < p_bitmap->format->palette->ncolors; l_i++) {
        printf("%d: %x %x %x\n", l_i,
			   p_bitmap->format->palette->colors[l_i].r,
			   p_bitmap->format->palette->colors[l_i].g,
			   p_bitmap->format->palette->colors[l_i].b);
    }
}

static void dumpUsedColors(SDL_Surface* surf);

BITMAP *load_bitmap(const char *filename, RGB *pal) {
	notes << "load " << filename << endl;
	std::string fullfilename = GetFullFileName(filename);	
	SDL_Surface* img = IMG_Load(fullfilename.c_str());
	if(!img) return NULL;
	
	if(/*color_depth == 8*/ img->format->BitsPerPixel == 8 ) {
	//	notes << "Used colors of " << filename << endl;
	//	dumpUsedColors(img);
//		DitherColors(img->format->palette->colors, 8);
		return create_bitmap_from_sdl(img);
	}
	
	
	int bpp = color_depth; //32; //color_depth;
	int flags = SDL_SWSURFACE;
//	if(bpp == 32) flags |= SDL_SRCALPHA;
	SDL_Surface* converted = NULL; //SDL_ConvertSurface(img, &pixelformat[bpp / 8], flags);
	//if(bpp != 32)
		converted = SDL_DisplayFormat(img);
	//else
	//	converted = SDL_DisplayFormatAlpha(img);
	SDL_FreeSurface(img);

	if(!converted) {
		errors << "Failed: Converting of bitmap " << filename << /*" to " << bpp <<*/ " bit" << endl;
		return NULL;
	}
	
	return create_bitmap_from_sdl(converted);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	//color_depth = 32;
	int flags = SDL_SWSURFACE;
	//if(color_depth == 32) flags |= SDL_SRCALPHA;
	//SDL_PixelFormat& fmt = pixelformat[color_depth/8];
	SDL_Surface* surf = SDL_CreateRGBSurface(flags, width, height, color_depth, 0,0,0,0); //fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
	if(!surf) return NULL;
	
	if(surf->format->BitsPerPixel != color_depth)
		warnings << "couldn't create surface with " << color_depth << " bpp" << endl;
	
//	if(surf->format->BitsPerPixel == 8)
//		DitherColors(surf->format->palette->colors, 8);
	
	return create_bitmap_from_sdl(surf);
}

BITMAP *create_bitmap(int width, int height) {
	return create_bitmap_ex(color_depth, width, height);
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
	sub_to_abs_coords(parent, x, y);
	return create_bitmap_from_sdl(parent->surf, x, y, width, height);
}

void destroy_bitmap(BITMAP *bmp) {
	if(bmp == NULL) return;
	bmp->surf = NULL;
	delete[] bmp->line;
	delete bmp;
}



int set_gfx_mode(int card, int w, int h, int v_w, int v_h) { return 0; }
int SCREEN_W = 640, SCREEN_H = 480;

BITMAP* screen = NULL;

int allegro_error = 0;





///////////////////
// Set the video mode
static bool SetVideoMode(int bpp = 32) {
	bool resetting = false;

	int DoubleBuf = false;
	int vidflags = 0;

	// Check that the bpp is valid
	switch (bpp) {
	case 0:
	case 16:
	case 24:
	case 32:
		break;
	default: bpp = 16;
	}
	notes << "ColorDepth: " << bpp << endl;

	// BlueBeret's addition (2007): OpenGL support
	bool opengl = false; //tLXOptions->bOpenGL;

	// Initialize the video
	if(/*tLXOptions->bFullscreen*/ false)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		vidflags |= SDL_OPENGL;
		vidflags |= SDL_OPENGLBLIT; // SDL will behave like normally
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1); // always use double buffering in OGL mode
	}	

	vidflags |= SDL_SWSURFACE;

	if(DoubleBuf && !opengl)
		vidflags |= SDL_DOUBLEBUF;

#ifdef WIN32
	UnSubclassWindow();  // Unsubclass before doing anything with the window
#endif

#ifdef WIN32
	static bool firsttime = true;
	// Reset the video subsystem under WIN32, else we get a "Could not reset OpenGL context" error when switching mode
	if (opengl && !firsttime)  {  // Don't reset when we're setting up the mode for first time
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}
	firsttime = false;
#endif

	int scrW = SCREEN_W;
	int scrH = SCREEN_H;
setvideomode:
	if( SDL_SetVideoMode(scrW, scrH, bpp, vidflags) == NULL) {
		if (resetting)  {
			errors << "Failed to reset video mode"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " let's wait a bit and retry" << endl;
			SDL_Delay(500);
			resetting = false;
			goto setvideomode;
		}

		if(bpp != 0) {
			errors << "Failed to use " << bpp << " bpp"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying automatic bpp detection ..." << endl;
			bpp = 0;
			goto setvideomode;
		}

		if(vidflags & SDL_OPENGL) {
			errors << "Failed to use OpenGL"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying without ..." << endl;
			vidflags &= ~(SDL_OPENGL | SDL_OPENGLBLIT | SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL);
			goto setvideomode;
		}
		
		if(vidflags & SDL_FULLSCREEN) {
			errors << "Failed to set full screen video mode "
					<< scrW << "x" << scrH << "x" << bpp
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying window mode ..." << endl;
			vidflags &= ~SDL_FULLSCREEN;
			goto setvideomode;
		}

		errors << "Failed to set the video mode " << scrW << "x" << scrH << "x" << bpp << endl;
		errors << "nErrorMsg: " << std::string(SDL_GetError()) << endl;
		return false;
	}

	SDL_WM_SetCaption("Gusanos", NULL);
	SDL_ShowCursor(SDL_DISABLE);

#ifdef WIN32
	if (false/*!tLXOptions->bFullscreen*/)  {
		SubclassWindow();
	}
#endif

	mainPixelFormat = SDL_GetVideoSurface()->format;
	DumpPixelFormat(mainPixelFormat);
	if(SDL_GetVideoSurface()->flags & SDL_DOUBLEBUF)
		notes << "using doublebuffering" << endl;

	if(SDL_GetVideoSurface()->flags & SDL_OPENGL)
		hints << "using OpenGL" << endl;
	
	FillSurface(SDL_GetVideoSurface(), Color(0, 0, 0));
	
	notes << "video mode was set successfully" << endl;
	return true;
}

static bool sdl_video_init() {
	if(getenv("SDL_VIDEODRIVER"))
		notes << "SDL_VIDEODRIVER=" << getenv("SDL_VIDEODRIVER") << endl;

	// Solves problem with FPS in fullscreen
#ifdef WIN32
	if(!getenv("SDL_VIDEODRIVER")) {
		notes << "SDL_VIDEODRIVER not set, setting to directx" << endl;
		putenv((char*)"SDL_VIDEODRIVER=directx");
	}
#endif

	// Initialize SDL
	int SDLflags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
	if(SDL_Init(SDLflags) == -1) {
		errors << "Failed to initialize the SDL system!\nErrorMsg: " << std::string(SDL_GetError()) << endl;
#ifdef WIN32
		// retry it with any available video driver
		unsetenv("SDL_VIDEODRIVER");
		if(SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the any driver" << endl;
		// retry with windib
		else if(putenv((char*)"SDL_VIDEODRIVER=windib") == 0 && SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the windib driver" << endl;
		else
#endif
			return false;
	}

	if(!SetVideoMode())
		return false;

    // Enable the system events
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);

	// Enable unicode and key repeat
	SDL_EnableUNICODE(1);
	//SDL_EnableKeyRepeat(200,20);

	return true;
}

void allegro_init() {
	InitBaseSearchPaths();
	
	init_pixelformats();
	bJoystickSupport = false;
	
	InitEventQueue();
	InitEventSystem();
		
	if(!sdl_video_init()) {
		errors << "Allegro init: video init failed" << endl;
		return;
	}

	screen = create_bitmap_from_sdl(SDL_GetVideoSurface());
}

void allegro_exit() {
	screen->surf = NULL; // to not free the video surface (SDL does that)
	destroy_bitmap(screen);
	screen = NULL;

	SDL_Quit();

	ShutdownEventSystem();
	ShutdownEventQueue();
}

void rest(int t) { SDL_Delay(t); }
void vsync() {  }


static void handle_sdlevents();

void acquire_screen() {}
void release_screen() {
	SDL_Flip(SDL_GetVideoSurface());
	handle_sdlevents();
}





int set_display_switch_mode(int mode) { return 0; }



bool exists(const char* filename) { return IsFileAvailable(filename); }



int makecol(int r, int g, int b) { return SDL_MapRGB(mainPixelFormat,r,g,b); }
int makecol_depth(int color_depth, int r, int g, int b) { return SDL_MapRGB(&pixelformat[color_depth/8],r,g,b); }


int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
    _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
    _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
    _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

int _rgb_scale_5[32], _rgb_scale_6[64];

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {	
    float maxc = MAX(MAX(r, g), b);
    float minc = MIN(MIN(r, g), b);
    *v = maxc;
    if(minc == maxc) {
		*h = 0;
		*s = 0;
		return;
	}

	*s = (maxc-minc) / maxc;
	float rc = (maxc-r) / (maxc-minc);
	float gc = (maxc-g) / (maxc-minc);
	float bc = (maxc-b) / (maxc-minc);
	if(r == maxc) *h = bc-gc;
	else if(g == maxc) *h = 2.0+rc-bc;
	else *h = 4.0+gc-rc;

	*h = *h/6.0;
	FMOD(*h, 1.0f);
}


static bool abscoord_in_bmp(BITMAP* bmp, int x, int y) {
	return x >= 0 && x < bmp->sub_x + bmp->w && y >= 0 && y < bmp->sub_y + bmp->h;
}

static int getpixel__nocheck(BITMAP *bmp, int x, int y) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

static void putpixel__nocheck(BITMAP *bmp, int x, int y, int color) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: bmp_write8(addr, color); break;
		case 2: bmp_write16(addr, color); break;
		case 3: bmp_write24(addr, color); break;
		case 4: bmp_write32(addr, color); break;
	}
}

int getpixel(BITMAP *bmp, int x, int y) {
	sub_to_abs_coords(bmp, x, y);
	if(!abscoord_in_bmp(bmp, x, y)) return 0;
	return getpixel__nocheck(bmp, x, y);
}

void putpixel(BITMAP *bmp, int x, int y, int color) {
	sub_to_abs_coords(bmp, x, y);
	if(!abscoord_in_bmp(bmp, x, y)) return;
	putpixel__nocheck(bmp, x, y, color);
}



void vline(BITMAP *bmp, int x, int y1, int y2, int color) {
	sub_to_abs_coords(bmp, x, y1);
	sub_to_abs_coords_y(bmp, y2);
	DrawVLine(bmp->surf.get(), y1, y2, x, Color(color));
/*	for(int y = y1; y < y2; ++y)
		if(abscoord_in_bmp(bmp, x, y))
			putpixel(bmp, x, y, color);*/
}

void hline(BITMAP *bmp, int x1, int y, int x2, int color) {
	sub_to_abs_coords(bmp, x1, y);
	sub_to_abs_coords_x(bmp, x2);
	DrawHLine(bmp->surf.get(), x1, x2, y, Color(color));
/*	for(int x = x1; x < x2; ++x)
		if(abscoord_in_bmp(bmp, x, y))
			putpixel(bmp, x, y, color);*/
}

void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	DrawLine(bmp->surf.get(), x1, y1, x2, y2, Color(color));
}

void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	SDL_Rect rect = { x1, y1, x2 - x1, y2 - y1 };
	SDL_FillRect(bmp->surf.get(), &rect, color);
}

void circle(BITMAP *bmp, int x, int y, int radius, int color) {
	sub_to_abs_coords(bmp, x, y);
	DrawCircleFilled(bmp->surf.get(), x, y, radius, radius, Color(color));
}


void clear_to_color(BITMAP *bmp, int color) {
	rectfill(bmp, 0,0, bmp->w, bmp->h, color);
}


static void blit_8to8__abscoord(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	int& sy = source_y;
	int& dy = dest_y;
	for(int Cy = height; Cy >= 0; --Cy, ++sy, ++dy) {
		int sx = source_x;
		int dx = dest_x;
		for(int Cx = width; Cx >= 0; --Cx, ++sx, ++dx) {
			if(abscoord_in_bmp(dest, dx, dy) && abscoord_in_bmp(source, sx, sy))
				putpixel__nocheck(dest, dx, dy, getpixel__nocheck(source, sx, sy));
		}
	}
}

static int getpixel__nocheck(SDL_Surface *surf, int x, int y) {
	unsigned long addr = (unsigned long) surf->pixels + y * surf->pitch + x * surf->format->BytesPerPixel;
	switch(surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

static void dumpUsedColors(SDL_Surface* surf) {
	std::set<Uint32> cols;
	for(int y = surf->h - 1; y >= 0; --y) {
		for(int x = surf->w - 1; x >= 0; --x) {
			cols.insert(getpixel__nocheck(surf, x, y));
		}
	}
	for(std::set<Uint32>::iterator i = cols.begin(); i != cols.end(); ++i)
		notes << "  : " << *i << endl;
}

void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	sub_to_abs_coords(source, source_x, source_y);
	sub_to_abs_coords(dest, dest_x, dest_y);
/*	if(source->surf->format->BitsPerPixel == 8 && dest->surf->format->BitsPerPixel == 8) {
		blit_8to8__abscoord(source, dest, source_x, source_y, dest_x, dest_y, width, height);
	}
	else {*/
		SDL_Rect srcrect = { source_x, source_y, width, height };
		SDL_Rect dstrect = { dest_x, dest_y, width, height };
		SDL_BlitSurface(source->surf.get(), &srcrect, dest->surf.get(), &dstrect);
//	}
}

void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) {
	sub_to_abs_coords(s, s_x, s_y);
	sub_to_abs_coords(d, d_x, d_y);
	DrawImageResizedAdv(d->surf.get(), s->surf.get(), s_x, s_y, d_x, d_y, s_w, s_h, d_w, d_h);
}

void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	SetColorKey(source->surf.get());
	blit(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}

void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) {
	masked_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}

void draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y) {
	DrawImageAdv_Mirror(bmp->surf.get(), sprite->surf.get(), 0, 0, x, y, sprite->w, sprite->h);
}


void clear_bitmap(BITMAP* bmp) { clear_to_color(bmp, 0); }



unsigned long bmp_write_line(BITMAP *bmp, int line) {
	return (unsigned long) bmp->line[line];
}

void bmp_unwrite_line(BITMAP* bmp) {}




void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor) {}
void set_trans_blender(int r, int g, int b, int a) {}
void set_add_blender (int r, int g, int b, int a) {}
void solid_mode() {}

int getr(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return r; }
int getg(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return g; }
int getb(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return b; }


void set_clip_rect(BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}






// index is allegro key, value is sdl keysym
static const int sdlkeymap[KEY_MAX+1] =
{
SDLK_UNKNOWN,
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'0','1','2','3','4','5','6','7','8','9',
'0','1','2','3','4','5','6','7','8','9', // numpads (no difference in SDL)
SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,SDLK_F11,SDLK_F12,
SDLK_ESCAPE,
'~',
'-',
'=',
SDLK_BACKSPACE,
SDLK_TAB,
'{','}',
SDLK_RETURN,
SDLK_COLON,
SDLK_QUOTE,
SDLK_BACKSLASH,
SDLK_BACKSLASH, // second backslash?
',',
SDLK_UNKNOWN, // STOP ?
SDLK_SLASH,
SDLK_SPACE,
SDLK_INSERT,SDLK_DELETE,
SDLK_HOME,SDLK_END,
SDLK_PAGEUP,SDLK_PAGEDOWN,
SDLK_LEFT,SDLK_RIGHT,SDLK_UP,SDLK_DOWN,
SDLK_SLASH, // numpad
SDLK_ASTERISK,
SDLK_MINUS,SDLK_PLUS,SDLK_DELETE,SDLK_RETURN, // numpad
SDLK_PRINT,
SDLK_PAUSE,
SDLK_UNKNOWN, // ABNT_C1
SDLK_UNKNOWN, // YEN
SDLK_UNKNOWN, // __allegro_KEY_KANA         = 96,
SDLK_UNKNOWN, // __allegro_KEY_CONVERT      = 97,
SDLK_UNKNOWN, // __allegro_KEY_NOCONVERT    = 98,
SDLK_AT,
SDLK_UNKNOWN, // __allegro_KEY_CIRCUMFLEX   = 100,
SDLK_UNKNOWN, // __allegro_KEY_COLON2       = 101,
SDLK_UNKNOWN, // __allegro_KEY_KANJI        = 102,
SDLK_EQUALS, // numpad
SDLK_BACKQUOTE,
SDLK_SEMICOLON,
SDLK_LSUPER, // command (MacOSX)
0,0,0,0,0,0,0,0, // allegro unknown1-8
// modifiers are starting here
SDLK_LSHIFT,SDLK_RSHIFT,
SDLK_LCTRL,SDLK_RCTRL,
SDLK_LALT, // alt
SDLK_RALT, // alt gr
SDLK_LSUPER, // lwin
SDLK_RSUPER, // rwin
SDLK_LMETA, // menu
SDLK_SCROLLOCK,
SDLK_NUMLOCK,
SDLK_CAPSLOCK,
SDLK_UNKNOWN // key max
};


// index is sdl keysym, value is allegro key
static int allegrokeymap[SDLK_LAST];

static int findAllegroKey(Uint32 sdlkeysym) {
	for(int i = 0; i < KEY_MAX; ++i) {
		if(sdlkeymap[i] == sdlkeysym) return i;
	}
	return KEY_UNKNOWN1;
}

void install_keyboard() {
	for(Uint32 i = 0; i < SDLK_LAST; ++i)
		allegrokeymap[i] = findAllegroKey(i);
	
	// just some checks to be sure i didn't made a mistake
	assert(sdlkeymap[KEY_ESC] == SDLK_ESCAPE);
	assert(sdlkeymap[__allegro_KEY_DEL] == SDLK_DELETE);
	assert(sdlkeymap[__allegro_KEY_DOWN] == SDLK_DOWN);
	assert(sdlkeymap[__allegro_KEY_AT] == SDLK_AT);
	assert(sdlkeymap[KEY_SEMICOLON] == SDLK_SEMICOLON);
	assert(sdlkeymap[KEY_LSHIFT] == SDLK_LSHIFT);
	assert(sdlkeymap[KEY_SCRLOCK] == SDLK_SCROLLOCK);
	assert(sdlkeymap[KEY_MAX] == SDLK_UNKNOWN);
	assert(allegrokeymap[SDLK_ESCAPE] == KEY_ESC);
}

void remove_keyboard() {}


static std::list<UnicodeChar> keyqueue;

bool keypressed() {
	return keyqueue.size() > 0;
}

int readkey() {
	int key = keyqueue.front();
	keyqueue.pop_front();
	return key;
}

int key[KEY_MAX];

void clear_keybuf() {
	keyqueue.clear();
	// don't clear key[], this is only about readkey()
	// for(int i = 0; i < KEY_MAX; ++i) key[i] = 0;
}

static void handle_sdlevents_keyb() {
	for(int i = 0; i < KEY_MAX; ++i)
		key[i] = GetKeyboard()->KeyDown[sdlkeymap[i]] ? 1 : 0;
	
	for(int i = 0; i < GetKeyboard()->queueLength; ++i) {
		KeyboardEvent& ev = GetKeyboard()->keyQueue[i];
		if(ev.down) keyqueue.push_back( ev.sym );
	}
}







void install_mouse() {}
void remove_mouse() {}

volatile int mouse_x = 0;
volatile int mouse_y = 0;
volatile int mouse_z = 0;
volatile int mouse_b = 0;
void (*mouse_callback)(int flags) = NULL;


int poll_mouse() { return 0; }


static void handle_sdlevents_mouse() {
	mouse_t* m = GetMouse();
	mouse_x = m->X;
	mouse_y = m->Y;
	mouse_b = m->Button;
}

static void handle_sdlevents_pushall() {
	SDL_Event ev;
	while( SDL_PollEvent(&ev) ) {
		if( ev.type == SDL_SYSWMEVENT ) {
			EvHndl_SysWmEvent_MainThread( &ev );
			continue;
		}

		if( ev.type == SDL_QUIT ) {
			notes << "SDL quit event" << endl;
			// not the best way but does the trick for now
			exit(0);
		}
		
		mainQueue->push(ev);
	}
}

static void handle_sdlevents() {
	handle_sdlevents_pushall();
	ProcessEvents(); // calls event handler from OLX

	handle_sdlevents_keyb();
	handle_sdlevents_mouse();	
}




// OLX wrappers

#include "CGameMode.h"
#include "Options.h"
				 
void InitGameModes() {}
CGameMode* GameMode(GameModeIndex i) { return NULL; }
GameModeIndex GetGameModeIndex(CGameMode* gameMode) { return GameModeIndex(0); }
void SystemError(const std::string& txt) {}
GameOptions* tLXOptions = NULL;
bool GameOptions::Init() { return false; }
bool Con_IsInited() { return false; }
void Con_AddText(int color, const std::string&, bool) {}

#ifndef WIN32
#include <setjmp.h>
sigjmp_buf longJumpBuffer;
#endif

#include "DeprecatedGUI/Menu.h"

namespace DeprecatedGUI { menu_t	*tMenu = NULL; }

void ClearUserNotify() {}
void updateFileListCaches() {}
void SetQuitEngineFlag(const std::string&) {}

#include "AuxLib.h"
SDL_Surface* VideoPostProcessor::m_videoSurface = NULL;
void VideoPostProcessor::transformCoordinates_ScreenToVideo( int& x, int& y ) {}
std::string GetConfigFile() { return ""; }

#include "Debug.h"
void SetError(const std::string& t) { errors << "OLX SetError: " << t << endl; }

void handle_system_event(const SDL_Event& ) {}

#include "Cache.h"
CCache cCache;
SmartPointer<SDL_Surface> CCache::GetImage__unsafe(const std::string& file) { return NULL; }
void CCache::SaveImage__unsafe(const std::string& file, const SmartPointer<SDL_Surface> & img) {}

struct SoundSample;
class CMap; class CGameScript;
template <> void SmartPointer_ObjectDeinit<SoundSample> ( SoundSample * obj ) {}
template <> void SmartPointer_ObjectDeinit<CMap> ( CMap * obj ) {}
template <> void SmartPointer_ObjectDeinit<CGameScript> ( CGameScript * obj ) {}
