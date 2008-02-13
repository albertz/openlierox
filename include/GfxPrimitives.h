/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Graphics primitives
// Created 12/11/01
// By Jason Boettcher


#ifndef __GFXPRIMITIVES_H__
#define __GFXPRIMITIVES_H__

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <assert.h>

#include "Color.h"


//
// Misc routines, defines and variables
//

// Flags used for screen and new surfaces
extern	int		iSurfaceFormat;

// Like in SDL_video.c in SDL_DisplayFormatAlpha
#define ALPHASURFACE_RMASK 0x00ff0000
#define ALPHASURFACE_GMASK 0x0000ff00
#define ALPHASURFACE_BMASK 0x000000ff
#define ALPHASURFACE_AMASK 0xff000000

/////////////////////
// Locking and unlocking routines, must be called before doing anything with pixels
inline bool LockSurface(SDL_Surface *bmp)  {
	if (SDL_MUSTLOCK(bmp))
		return SDL_LockSurface(bmp) != -1;
	return true;
}

inline void UnlockSurface(SDL_Surface *bmp)  {
	if (SDL_MUSTLOCK(bmp))
		SDL_UnlockSurface(bmp);
}

#define LOCK_OR_QUIT(bmp)	{ if(!LockSurface(bmp)) return; }
#define LOCK_OR_FAIL(bmp)	{ if(!LockSurface(bmp)) return false; }


//
// Clipping routines
//

/////////////////////
// Clip the line to the surface
bool ClipLine(SDL_Surface * dst, int * x1, int * y1, int * x2, int * y2);


class SDLRectBasic : public SDL_Rect {
public:
	typedef Sint16 Type;
	typedef Uint16 TypeS;
	
	Type& x() { return this->SDL_Rect::x; }
	Type& y() { return this->SDL_Rect::y; }
	TypeS& width() { return this->SDL_Rect::w; }
	TypeS& height() { return this->SDL_Rect::h; }
	
	Type x() const { return this->SDL_Rect::x; }
	Type y() const { return this->SDL_Rect::y; }
	TypeS width() const { return this->SDL_Rect::w; }
	TypeS height() const { return this->SDL_Rect::h; }
};

template<typename _Type, typename _TypeS>
class RefRectBasic {
public:
	typedef _Type Type;
	typedef _TypeS TypeS;
private:
	Type *m_x, *m_y;
	TypeS *m_w, *m_h;
public:
	RefRectBasic() : m_x(NULL), m_y(NULL), m_w(NULL), m_h(NULL) {
		// HINT: never use this constructor directly; it's only there to avoid some possible compiler-warnings
		assert(false);
	}
	RefRectBasic(Type& x_, Type& y_, TypeS& w_, TypeS& h_)
	: m_x(&x_), m_y(&y_), m_w(&w_), m_h(&h_) {}
	
	Type& x() { return *m_x; }
	Type& y() { return *m_y; }
	TypeS& width() { return *m_w; }
	TypeS& height() { return *m_h; }
	
	Type x() const { return *m_x; }
	Type y() const { return *m_y; }
	TypeS width() const { return *m_w; }
	TypeS height() const { return *m_h; }
};


// _RectBasic has to provide the following public members:
//		typedef ... Type; // type for x,y
//		typedef ... TypeS; // type for w,h
//		Type x();
//		Type y();
//		TypeS width();
//		TypeS height();
//		and the above as const
template<typename _RectBasic>
class Rect : public _RectBasic {
public:
	
	class AssignX2 : private _RectBasic {
	public:
		AssignX2& operator=(const typename _RectBasic::Type& v)
		{ this->_RectBasic::width() = v - this->_RectBasic::x(); return *this; }
		operator typename _RectBasic::Type () const
		{ return this->_RectBasic::x() + this->_RectBasic::width(); }
	};
	AssignX2& x2() { return (AssignX2&)*this; }
	const AssignX2& x2() const { return (const AssignX2&)*this; }
	
	class AssignY2 : private _RectBasic {
	public:
		AssignY2& operator=(const typename _RectBasic::Type& v)
		{ this->_RectBasic::height() = v - this->_RectBasic::y(); return *this; }
		operator typename _RectBasic::Type () const
		{ return this->_RectBasic::y() + this->_RectBasic::height(); }
	};
	AssignY2& y2() { return (AssignY2&)*this; }
	const AssignY2& y2() const { return (AssignY2&)*this; }
	
	template<typename _ClipRect>
	bool clipWith(const _ClipRect& clip) {
		// Horizontal
		{
			typename Rect::Type orig_x2 = this->Rect::x2();
			this->Rect::x() = MAX( (typename Rect::Type)this->Rect::x(), (typename Rect::Type)clip.x() );
			this->Rect::x2() = MIN( orig_x2, (typename Rect::Type)clip.x2() );
			this->Rect::x2() = MAX( this->Rect::x(), (typename Rect::Type)this->Rect::x2() );
		}

		// Vertical
		{		
			typename Rect::Type orig_y2 = this->Rect::y2();
			this->Rect::y() = MAX( (typename Rect::Type)this->Rect::y(), (typename Rect::Type)clip.y() );
			this->Rect::y2() = MIN( orig_y2, (typename Rect::Type)clip.y2() );
			this->Rect::y2() = MAX( this->Rect::y(), (typename Rect::Type)this->Rect::y2() );
		}
				
		return (this->Rect::width() && this->Rect::height());
	}
};


typedef Rect<SDLRectBasic> SDLRect;  // Use this for creating clipping rects from SDL

template<typename _Type, typename _TypeS, typename _ClipRect>
bool ClipRefRectWith(_Type& x, _Type& y, _TypeS& w, _TypeS& h, const _ClipRect& clip) {
	RefRectBasic<_Type, _TypeS> refrect = RefRectBasic<_Type, _TypeS>(x, y, w, h);
	return ((Rect<RefRectBasic<_Type, _TypeS> >&) refrect).clipWith(clip);
}

template<typename _ClipRect>
bool ClipRefRectWith(SDL_Rect& rect, const _ClipRect& clip) {
	return ((SDLRect&)rect).clipWith(clip);
}


bool OneSideClip(int& c, int& d, const int clip_c, const int clip_d);

//
// Image loading and saving
//

//////////////////
// Load an image
SDL_Surface*	LoadImage(const std::string& _filename, bool withalpha = false);

/////////////////
// Loads an image and quits with error if could not load
#define		LOAD_IMAGE(bmp,name) if (!Load_Image(bmp,name)) {return false;}
#define		LOAD_IMAGE_WITHALPHA(bmp,name) if (!Load_Image_WithAlpha(bmp,name)) {return false;}

/////////////////
// Gets the colorkey from the surface
#define		COLORKEY(bmp) ((bmp)->format->colorkey)


/////////////////////
// Load an image, without alpha channel
inline bool Load_Image(SDL_Surface*& bmp, const std::string& name)  {
	bmp = LoadImage(name); 
	if (bmp == NULL)  { 
		printf("WARNING: could not load image %s\n", name.c_str()); 
		return false;
	}
	return true;
}

////////////////////
// Load an image with alpha channel
inline bool Load_Image_WithAlpha(SDL_Surface*& bmp, const std::string& name)  {
	bmp = LoadImage(name, true);
	if (bmp == NULL)  { 
		printf("WARNING: could not load image %s\n", name.c_str()); 
		return false;
	}
	return true;
}

///////////////////
// Save surface in the specified format
bool SaveSurface(SDL_Surface *image, const std::string& FileName, int Format, const std::string& Data);


//
// Surface stuff
//

//////////////////
// Creates a buffer with the same details as the screen
inline SDL_Surface* gfxCreateSurface(int width, int height, bool forceSoftware = false) {
	if (width <= 0 || height <= 0) // Nonsense, can cause trouble
		return NULL;

	SDL_PixelFormat* fmt = getMainPixelFormat();

	SDL_Surface* result = SDL_CreateRGBSurface(
			forceSoftware ? SDL_SWSURFACE : iSurfaceFormat,
			width, height, 
			fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if (result)
		// OpenGL strictly requires the surface to be cleared
		SDL_FillRect(result, NULL, SDL_MapRGBA(result->format, 0, 0, 0, 255));
	
	return result;
}


///////////////////
// Creates an ARGB 32bit surface if screen supports no alpha or a surface like screen
inline SDL_Surface* gfxCreateSurfaceAlpha(int width, int height, bool forceSoftware = false) {
	if (width <= 0 || height <= 0) // Nonsense, can cause trouble
		return NULL;

	SDL_Surface* result;
	SDL_PixelFormat* fmt = getMainPixelFormat();
	
	// HINT: in 32bit mode with software surfaces, we have to use the predefined masks because they are hardcoded in SDL
	// (else the blitting is wrong)
	// it seems that for other BPP this is not the case
	if(!forceSoftware && (iSurfaceFormat == SDL_HWSURFACE || fmt->BitsPerPixel != 32) && fmt->Amask != 0) // the main pixel format supports alpha blending
		result = SDL_CreateRGBSurface(
				iSurfaceFormat | SDL_SRCALPHA,
				width, height,
				fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
	
	else // no native alpha blending or forced software, so create a software alpha blended surface
		result = SDL_CreateRGBSurface(
				SDL_SWSURFACE | SDL_SRCALPHA,
				width, height, 32,
				ALPHASURFACE_RMASK, ALPHASURFACE_GMASK, ALPHASURFACE_BMASK, ALPHASURFACE_AMASK);

	if (result)
		// OpenGL strictly requires the surface to be cleared
		SDL_FillRect( result, NULL, SDL_MapRGB(result->format, 0, 0, 0));
	
	return result;
}

////////////////////
// Destroys a surface
inline void gfxFreeSurface(SDL_Surface *& surf)  {
	if (surf == NULL)
		return;

	SDL_FreeSurface(surf);
	surf = NULL;
}


//
// Image drawing
//

///////////////
// Copies one surface to another (not blitting, so the alpha values are kept!)
void CopySurface(SDL_Surface* dst, SDL_Surface* src, int sx, int sy, int dx, int dy, int w, int h);


//////////////
// Draw the image with a huge amount of options
inline void DrawImageAdv(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, SDL_Rect& rDest, SDL_Rect& rSrc) {
	SDL_BlitSurface(bmpSrc, &rSrc, bmpDest, &rDest);
}

//////////////
// Draw the image with a huge amount of options
inline void DrawImageAdv(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	SDL_Rect r1 = { dx, dy, 0, 0 };
	SDL_Rect r2 = { sx, sy, w, h };
	DrawImageAdv( bmpDest, bmpSrc, r1, r2); 
}


///////////////
// Draw the image, with more options
inline void DrawImageEx(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y, int w, int h) {
	DrawImageAdv(bmpDest, bmpSrc, 0, 0, x, y, w, h);
}

///////////////
// Simply draw the image
inline void DrawImage(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, SDL_Rect& rDest) {
	SDL_BlitSurface(bmpSrc, NULL, bmpDest, &rDest);
}

///////////////
// Simply draw the image
inline void DrawImage(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y) {
	SDL_Rect r = { x, y, 0, 0 };
	DrawImage( bmpDest, bmpSrc, r);
}


///////////////
// Draws image mirror flipped
// WARNING: passing invalid source x/y/w/h causes a segfault
void DrawImageAdv_Mirror(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);

////////////////
// Draws the image doubly stretched (fast)
void DrawImageStretch2(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);

/////////////////
// Draws the image doubly stretched while checking for colorkey
void DrawImageStretch2Key(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);

/////////////////
// Draws image doubly stretched, mirrored and checking for colorkey
// WARNING: passing invalid source x/y/w/h causes a segfault
void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);

/////////////////
// Creates a new surface of the same size and draws the image mirror flipped onto it
inline SDL_Surface *GetMirroredImage(SDL_Surface *bmpSrc)  {
	SDL_Surface* result = SDL_CreateRGBSurface(
			bmpSrc->flags,
			bmpSrc->w, bmpSrc->h,
			bmpSrc->format->BitsPerPixel,
			bmpSrc->format->Rmask,
			bmpSrc->format->Gmask,
			bmpSrc->format->Bmask,
			bmpSrc->format->Amask);
	if (!result)
		return NULL;
	DrawImageAdv_Mirror(result, bmpSrc, 0, 0, 0, 0, bmpSrc->w, bmpSrc->h);
	return result;
}

/////////////////
// Draws a sprite doubly stretched but not so advanced
inline void	DrawImageStretch(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy) {
	DrawImageStretch2(bmpDest,bmpSrc,0,0,dx,dy,bmpSrc->w,bmpSrc->h);
}

/////////////////
// Draws a sprite doubly stretched, with a colour key and not so advanced
inline void	DrawImageStretchKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy) {
	DrawImageStretch2Key(bmpDest, bmpSrc, 0, 0, dx, dy, bmpSrc->w, bmpSrc->h);
}

/////////////////
// Draws the image resized according to ratios
void DrawImageResizedAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, float sx, float sy, int dx, int dy, int sw, int sh, float xratio, float yratio);

/////////////////
// Draws the image nicely resampled, blur says how much the result should be blurred
void DrawImageResampledAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, float sx, float sy, int dx, int dy, int sw, int sh, float xratio, float yratio, float blur = 1.0f);


//
// Pixel and color routines
//

/////////////////
// Put pixel to a specified address
// WARNING: passing an invalid adress will cause a segfault
// NOTE: destination surface must be locked before calling this
inline void PutPixelToAddr(Uint8* p, Uint32 color, short bpp) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	memcpy(p, (Uint8*)&color + 4 - bpp, bpp);
#else	
	memcpy(p, &color, bpp);
#endif
}

//////////////
// Pixel drawing
// WARNING: passing invalid coordinates will cause a segfault
// NOTE: bmpDest must be locked before calling this
inline void PutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 color) {
	PutPixelToAddr(
			(Uint8*)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel,
			color,
			bmpDest->format->BytesPerPixel);
}

////////////////
// Get a pixel from an 8bit address
// WARNING: passing invalid adress will cause a segfault
// NOTE: the surface must be locked before calling this
inline Uint32 GetPixelFromAddr(Uint8* p, short bpp) {
	Uint32 result;
	result = 0;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	memcpy((Uint8*)&result + 4 - bpp, p, bpp);
#else
	memcpy(&result, p, bpp);
#endif	
	return result;
}

////////////////
// Get a pixel from the surface
// WARNING: passing invalid coordinates will cause a segfault
// NOTE: bmpSrc must be locked before calling this
inline Uint32 GetPixel(SDL_Surface* bmpSrc, int x, int y) {
	return GetPixelFromAddr(
			(Uint8*)bmpSrc->pixels + y * bmpSrc->pitch + x * bmpSrc->format->BytesPerPixel,
			bmpSrc->format->BytesPerPixel);
}

////////////////
// Copy pixel from one surface to another, both surfaces must have same format
// WARNING: doesn't do clipping
// NOTE: dst must be locked before calling this
inline void CopyPixel_SameFormat(
	SDL_Surface* dst, SDL_Surface* src,
	int dx, int dy, int sx, int sy) {
	memcpy(
		(Uint8*)dst->pixels + dy * dst->pitch + dx * dst->format->BytesPerPixel,
		(Uint8*)src->pixels + sy * src->pitch + sx * dst->format->BytesPerPixel,
		dst->format->BytesPerPixel);
}

////////////////
// Copy pixel from one surface to another, the coordinate on both surfaces is the same
// WARNING: doesn't do clipping
// WARNING: surfaces must have same format
// NOTE: dst must be locked before calling his
inline void CopyPixel_SameFormat(
	SDL_Surface* dst, SDL_Surface* src, int x, int y) {
	CopyPixel_SameFormat(dst, src, x, y, x, y);
}


////////////////
// Put pixel alpha blended with the background
// WARNING: passing invalid coordinates will cause a segfault
// NOTE: dst must be locked before calling this
void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, float a);

inline void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, Uint8 a) {
	PutPixelA(bmpDest, x, y, colour, (float)a / 255.0f);
}


////////////////
// Extract 4 colour components from a packed int
inline void GetColour4(Uint32 pixel, SDL_PixelFormat* format, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
	SDL_GetRGBA(pixel, format, r, g, b, a);
}

///////////////
// Extract 3 colour components from a packed int
inline void GetColour3(Uint32 pixel, SDL_PixelFormat* format, Uint8 *r, Uint8 *g, Uint8 *b) {
	SDL_GetRGB(pixel, format, r, g, b);
}

////////////////
// Returns true if the color is considered as (partly) transparent on the surface
inline bool IsTransparent(SDL_Surface* surf, Uint32 color)  {
	if((surf->flags & SDL_SRCALPHA) && ((color & surf->format->Amask) != surf->format->Amask))
		return true;
	
	// TODO: should this check be done, if SDL_SRCALPHA was set? SDL/OpenGL possibly will ignore it
	if((surf->flags & SDL_SRCCOLORKEY) && (EqualRGB(color, COLORKEY(surf), surf->format)))
		return true;
		
	return false;
}




//
// Solid drawing
//


///////////////////
// Draw horizontal line
void	DrawHLine(SDL_Surface *bmpDest, int x, int x2, int y, Uint32 colour);

///////////////////
// Draw vertical line
void	DrawVLine(SDL_Surface *bmpDest, int y, int y2, int x, Uint32 colour);

///////////////////
// Draw a line
void	DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);

//////////////////
// Draw the line nicely antialiased
void	AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, Uint32 color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8));

/////////////////////
// Draws a filled rectangle
inline void	DrawRectFill(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 color) {
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = x2-x;
	r.h = y2-y;
	SDL_FillRect(bmpDest,&r,color);
}

////////////////////
// Fills the surface with specified colour
inline void FillSurface(SDL_Surface* dst, Uint32 colour) {
	SDL_FillRect(dst, NULL, colour);
}

////////////////////
// Fills the whole surface with a transparent color
inline void FillSurfaceTransparent(SDL_Surface *dst)  {
	// check alpha first as it has priority (if set, colorkey is ignored)
	if (dst->flags & SDL_SRCALPHA)
		FillSurface(dst, SDL_MapRGBA(dst->format, 255, 0, 255, SDL_ALPHA_TRANSPARENT));
	else if (dst->flags & SDL_SRCCOLORKEY)
		FillSurface(dst, COLORKEY(dst));
	else
		printf("Warning: There's no possibility to make this surface transparent!\n");
}


////////////////////
// Draws a rectangle
inline void	DrawRect(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 colour) {
	DrawHLine(bmpDest, x, x2, y, colour);
	DrawHLine(bmpDest, x, x2, y2, colour);
	DrawVLine(bmpDest, y, y2, x, colour);
	DrawVLine(bmpDest, y, y2, x2, colour);
}

///////////////////
// Draws a rectangle with transparency
inline void DrawRectFillA(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 color, Uint8 alpha)  {
	SDL_Surface *tmp = gfxCreateSurfaceAlpha(x2-x,y2-y);
	Uint8 r,g,b;
	GetColour3(color,bmpDest->format,&r,&g,&b);
	Uint32 friendly_col = SDL_MapRGBA(tmp->format,r,g,b,alpha);
	if (tmp)  {
		// TODO: optimise
		SDL_FillRect(tmp,NULL,friendly_col);
		DrawImage(bmpDest,tmp,x,y);
		SDL_FreeSurface(tmp);
	}
}

//////////////////
// Draw a triangle
inline void DrawTriangle(SDL_Surface *bmpDest, int x1, int y1, int x2, int y2, int x3, int y3, Uint32 colour) {
	DrawLine(bmpDest, x1, y1, x2, y2, colour);
	DrawLine(bmpDest, x2, y2, x3, y3, colour);
	DrawLine(bmpDest, x3, y3, x1, y1, colour);
}



//
// Special lines (rope, laser sight, beam)
//

void	DrawRope(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawBeam(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawLaserSight(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);


//
// Colorkey handling
//

// sets alpha in a safe way for both non-alpha-surfaces and alpha-surfaces
// for non-alpha surfaces, it uses SDL_SetAlpha
// for real alphablended surfaces, that means this multiplies a/255 to each a-value
void SetPerSurfaceAlpha(SDL_Surface *dst, Uint8 a);

// set colorkey for both alpha-blended and non-alpha surfaces
// for non-alpha surfaces, SDL_SetAlpha is used
// for alpha surfaces, it applies to every pixel
void SetColorKey(SDL_Surface* dst, Uint8 r, Uint8 g, Uint8 b);

//////////////////
// Set's the game's default color key (pink) to the surface
// Works for both alpha and nonalpha surfaces
void SetColorKey(SDL_Surface* dst);

//////////////////
// Resets the alpha-channel and the colorkey
inline void ResetAlpha(SDL_Surface* dst) {
	SDL_SetColorKey(dst, 0, 0); // Remove the colorkey
	SDL_SetAlpha(dst, 0, 0); // Remove the persurface-alpha

	LOCK_OR_QUIT(dst);
	
	int x, y;
	for(y = 0; y < dst->h; y++)
		for(x = 0; x < dst->w; x++)
			PutPixel(dst, x, y, GetPixel(dst, x, y) | dst->format->Amask);

	UnlockSurface(dst);
}

#endif  //  __GFXPRIMITIVES_H__
