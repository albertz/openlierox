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
#include "allegro.h"
#include "FindFile.h"
#include "Debug.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "InputEvents.h"
#include "EventQueue.h"
#include "LieroX.h"


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

BITMAP *load_bitmap(const char *filename, RGB *pal) {
	SDL_Surface* img = IMG_Load(filename);
	if(!img) return NULL;

	SDL_Surface* converted = SDL_DisplayFormatAlpha(img);
	SDL_FreeSurface(img);

	if(!converted) return NULL;
	
	return create_bitmap_from_sdl(converted);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	//color_depth = 32;
	int flags = SDL_SWSURFACE;
	if(color_depth == 32) flags |= SDL_SRCALPHA;
	SDL_Surface* surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, color_depth, 0,0,0,0);
	if(!surf) return NULL;
	return create_bitmap_from_sdl(surf);
}

BITMAP *create_bitmap(int width, int height) {
	return create_bitmap_ex(32, width, height);
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
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





SDL_PixelFormat defaultFallbackFormat =
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

SDL_PixelFormat* mainPixelFormat = &defaultFallbackFormat;


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
	//DumpPixelFormat(mainPixelFormat);
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
	SDL_EnableKeyRepeat(200,20);

	return true;
}

void allegro_init() {
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



bool exists(const char* filename) { return IsFileAvailable(filename, true); }



int makecol(int r, int g, int b) { return SDL_MapRGB(mainPixelFormat,r,g,b); }
int makecol_depth(int color_depth, int r, int g, int b) { return makecol(r,g,b); }


int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
    _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
    _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
    _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

int _rgb_scale_5[32], _rgb_scale_6[64];

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {
	// TODO...
}





int getpixel(BITMAP *bmp, int x, int y) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

void putpixel(BITMAP *bmp, int x, int y, int color) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: bmp_write8(addr, color); break;
		case 2: bmp_write16(addr, color); break;
		case 3: bmp_write24(addr, color); break;
		case 4: bmp_write32(addr, color); break;
	}
}



void vline(BITMAP *bmp, int x, int y1, int y2, int color) {
	for(int y = y1; y < y2; ++y)
		putpixel(bmp, x, y, color);
}

void hline(BITMAP *bmp, int x1, int y, int x2, int color) {
	for(int x = x1; x < x2; ++x)
		putpixel(bmp, x, y, color);
}

void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {}

void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	SDL_Rect rect = { x1, y1, x2 - x1, y2 - y1 };
	SDL_FillRect(bmp->surf.get(), &rect, color);
}

void circle(BITMAP *bmp, int x, int y, int radius, int color) {}


void clear_to_color(BITMAP *bmp, int color) {
	rectfill(bmp, 0,0, bmp->w, bmp->h, color);
}


void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	SDL_Rect srcrect = { source_x, source_y, width, height };
	SDL_Rect dstrect = { dest_x, dest_y, width, height };
	SDL_BlitSurface(source->surf.get(), &srcrect, dest->surf.get(), &dstrect);
}

void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) {
	// TODO...
	blit(s,d,s_x,s_y,d_x,d_y,s_w,s_h);
}

void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	// TODO...
	blit(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}

void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) {
	blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}

void draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y) {
	// TODO...
	draw_sprite(bmp, sprite, x, y);
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

int get_color_conversion() { return 0; }
void set_color_conversion(int mode) {}

int get_color_depth() { return 0; }
void set_color_depth(int depth) {}


void set_clip_rect(BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}








void install_keyboard() {}
void remove_keyboard() {}
bool keypressed() { return false; }
int readkey() { return 0; }
int key[KEY_MAX];

void clear_keybuf() {}







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

	//GetKeyboard();

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
void VideoPostProcessor::transformCoordinates_ScreenToVideo( int& x, int& y ) {}
std::string GetConfigFile() { return ""; }

#include "Debug.h"
void SetError(const std::string& t) { errors << "OLX SetError: " << t << endl; }

void handle_system_event(const SDL_Event& ) {}

template <> void SmartPointer_ObjectDeinit<SDL_Surface> ( SDL_Surface * obj )
{
	SDL_FreeSurface(obj);
}

