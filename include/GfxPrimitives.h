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
#include "SmartPointer.h"
#include "Debug.h"
#include "CVec.h"


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

// Gradient direction
enum GradientDirection  {
	grdHorizontal,
	grdVertical
};



void DumpPixelFormat(const SDL_PixelFormat* format);
bool PixelFormatEqual(const SDL_PixelFormat* fm1, const SDL_PixelFormat* fm2);
bool IsCorrectSurfaceFormat(const SDL_PixelFormat* format);


/////////////////////
// Locking and unlocking routines, must be called before doing anything with pixels
inline bool LockSurface(SDL_Surface * bmp)  {
	if (SDL_MUSTLOCK(bmp))
		return SDL_LockSurface(bmp) != -1;
	return true;
}
inline bool LockSurface(const SmartPointer<SDL_Surface> & bmp)  {
	return LockSurface(bmp.get());
}

inline void UnlockSurface(SDL_Surface * bmp)  {
	if (SDL_MUSTLOCK(bmp))
		SDL_UnlockSurface(bmp);
}
inline void UnlockSurface(const SmartPointer<SDL_Surface> & bmp)  {
	UnlockSurface(bmp.get());
}

#define LOCK_OR_QUIT(bmp)	{ if(!LockSurface(bmp)) return; }
#define LOCK_OR_FAIL(bmp)	{ if(!LockSurface(bmp)) return false; }


////////////////////
// Returns number of bytes the surface takes in memory
inline size_t GetSurfaceMemorySize(SDL_Surface *surf)  {
	if (surf)
		return sizeof(SDL_Surface) + sizeof(SDL_PixelFormat) + surf->w * surf->h * surf->format->BytesPerPixel;
	else
		return 0;
}

//
// Clipping routines
//

/////////////////////
// Clip the line to the surface
bool ClipLine(SDL_Surface * dst, int * x1, int * y1, int * x2, int * y2);
inline bool ClipLine(const SmartPointer<SDL_Surface> & bmp, int * x1, int * y1, int * x2, int * y2){
	return ClipLine(bmp.get(), x1, y1, x2, y2);
}

class SDLRectBasic : public SDL_Rect {
public:
	typedef Sint16 Type;
	typedef Uint16 TypeS;
	
	SDLRectBasic() { this->SDL_Rect::x = this->SDL_Rect::y = this->SDL_Rect::w = this->SDL_Rect::h = 0; }
	SDLRectBasic(const SDL_Rect & r): SDL_Rect(r) {}
	SDLRectBasic(Type _x, Type _y, TypeS _w, TypeS _h) { SDL_Rect::x = _x; SDL_Rect::y = _y; SDL_Rect::w = _w; SDL_Rect::h = _h; }
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
class OLXRect : public _RectBasic {
public:

	OLXRect(const _RectBasic & r): _RectBasic(r) {}

	OLXRect(typename _RectBasic::Type x, typename _RectBasic::Type y, typename _RectBasic::TypeS w, typename _RectBasic::TypeS h): _RectBasic(x, y, w, h) {}

	class GetX2 {
	protected:
		const _RectBasic* base;
	public:
		GetX2(const _RectBasic* b) : base(b) {}
		operator typename _RectBasic::Type () const
		{ return base->x() + base->width(); }
	};
	class AssignX2 : public GetX2 {
	private:
		_RectBasic* base;
	public:
		AssignX2(_RectBasic* b) : GetX2(b), base(b) {}
		AssignX2& operator=(const typename _RectBasic::Type& v)
		{ base->width() = v - base->x(); return *this; }	
	};

	AssignX2 x2() { return AssignX2(this); }
	GetX2 x2() const { return GetX2(this); }

	class GetY2 {
	protected:
		const _RectBasic* base;
	public:
		GetY2(const _RectBasic* b) : base(b) {}
		operator typename _RectBasic::Type () const
		{ return base->y() + base->height(); }
	};
	class AssignY2 : public GetY2 {
	private:
		_RectBasic* base;
	public:
		AssignY2(_RectBasic* b) : GetY2(b), base(b) {}
		AssignY2& operator=(const typename _RectBasic::Type& v)
		{ base->height() = v - base->y(); return *this; }
	};
	AssignY2 y2() { return AssignY2(this); }
	GetY2 y2() const { return GetY2(this); }

	template<typename _ClipRect>
	bool clipWith(const _ClipRect& clip) {
		// Horizontal
		{
			typename OLXRect::Type orig_x2 = this->OLXRect::x2();
			this->OLXRect::x() = MAX( (typename OLXRect::Type)this->OLXRect::x(), (typename OLXRect::Type)clip.x() );
			this->OLXRect::x2() = MIN( orig_x2, (typename OLXRect::Type)clip.x2() );
			this->OLXRect::x2() = MAX( this->OLXRect::x(), (typename OLXRect::Type)this->OLXRect::x2() );
		}

		// Vertical
		{
			typename OLXRect::Type orig_y2 = this->OLXRect::y2();
			this->OLXRect::y() = MAX( (typename OLXRect::Type)this->OLXRect::y(), (typename OLXRect::Type)clip.y() );
			this->OLXRect::y2() = MIN( orig_y2, (typename OLXRect::Type)clip.y2() );
			this->OLXRect::y2() = MAX( this->OLXRect::y(), (typename OLXRect::Type)this->OLXRect::y2() );
		}

		return (this->OLXRect::width() && this->OLXRect::height());
	}
};


typedef OLXRect<SDLRectBasic> SDLRect;  // Use this for creating clipping rects from SDL

template<typename _Type, typename _TypeS, typename _ClipRect>
bool ClipRefRectWith(_Type& x, _Type& y, _TypeS& w, _TypeS& h, const _ClipRect& clip) {
	RefRectBasic<_Type, _TypeS> refrect = RefRectBasic<_Type, _TypeS>(x, y, w, h);
	return ((OLXRect<RefRectBasic<_Type, _TypeS> >&) refrect).clipWith(clip);
}

template<typename _ClipRect>
bool ClipRefRectWith(SDL_Rect& rect, const _ClipRect& clip) {
	RefRectBasic<Sint16,Uint16> refrect(rect.x, rect.y, rect.w, rect.h);
	return OLXRect< RefRectBasic<Sint16,Uint16> >(refrect).clipWith(clip);
}



/////////////////////////
// Performs one side (horizontal or vertical) clip
// c - x or y; d - width or height
template<typename T1, typename T2>
bool _OneSideClip(T1& c, T2& d, const T1 clip_c, const T2 clip_d)  {
	if (c < clip_c)  {
		d += c - clip_c;
		c = clip_c;
		if (d <= 0)  {
			d = 0;
			return false;
		}
	}
	
	if (c + d >= clip_c + clip_d)  {
		if (c >= clip_c + clip_d)  {
			d = 0;
			return false;
		}
		
		d = clip_c + clip_d - c;
	}
	
	return true;
}

template<typename T1, typename T2, typename T3, typename T4>
bool OneSideClip(T1& c, T2& d, const T3 clip_c, const T4 clip_d)  {
	return _OneSideClip(c, d, (T1)clip_c, (T2)clip_d);
}

////////////////
// Create a SDL rect
inline SDL_Rect MakeRect(int x, int y, int w, int h)
{
	SDL_Rect r = {(SDLRect::Type) x, (SDLRect::Type) y, (SDLRect::TypeS) w, (SDLRect::TypeS) h};
	return r;
}

////////////////
// Returns true if the given point is in the given rect
inline bool PointInRect(int x, int y, const SDL_Rect& r)
{
	return	(r.x <= x) && (x <= (r.x + r.w)) &&
			(r.y <= y) && (y <= (r.y + r.h));
}

//////////////////////
// Returns true if rect1 contains rect2
inline bool ContainsRect(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	return (rect1.x <= rect2.x) && (rect1.x + rect1.w >= rect2.x + rect2.w) && 
			(rect1.y <= rect2.y) && (rect1.y + rect1.h >= rect2.y + rect2.h);
}

//
// Image loading and saving
//

//////////////////
// Load an image
SmartPointer<SDL_Surface> LoadGameImage(const std::string& _filename, bool withalpha = false);

/////////////////
// Loads an image and quits with error if could not load
#define		LOAD_IMAGE(bmp,name)			{ if (!Load_Image(bmp,name)) return false; }
#define		LOAD_IMAGE_WITHALPHA(bmp,name)	{ if (!Load_Image_WithAlpha(bmp,name)) return false; }
#define		LOAD_IMAGE_WITHALPHA2(bmp,name1,name2)	{ if (!Load_Image_WithAlpha(bmp,name1) && !Load_Image_WithAlpha(bmp,name2)) return false; }
#define		LOAD_IMAGE_WITHALPHA__OR(bmp,name,img)	{ if (!Load_Image_WithAlpha(bmp,name) && ((bmp = (img)).get() == NULL)) return false; }

/////////////////
// Gets the colorkey from the surface
#define		COLORKEY(bmp) ((bmp)->format->colorkey)


/////////////////////
// Load an image, without alpha channel
inline bool Load_Image(SmartPointer<SDL_Surface>& bmp, const std::string& name)  {
	bmp = LoadGameImage(name);
	if (bmp.get() == NULL)  {
		warnings << "could not load alpha-image " << name << endl;
		return false;
	}
	return true;
}

////////////////////
// Load an image with alpha channel
inline bool Load_Image_WithAlpha(SmartPointer<SDL_Surface>& bmp, const std::string& name)  {
	bmp = LoadGameImage(name, true);
	if (bmp.get() == NULL)  {
		warnings << "could not load image " << name << endl;
		return false;
	}
	return true;
}

///////////////////
// Save surface in the specified format
bool SaveSurface(SDL_Surface * image, const std::string& FileName, int Format, const std::string& Data);
inline bool SaveSurface(const SmartPointer<SDL_Surface> & image, const std::string& FileName, int Format, const std::string& Data){
	return SaveSurface(image.get(), FileName, Format, Data);
}

//
// Surface stuff
//

//////////////////
// Creates a buffer with the same details as the screen
SmartPointer<SDL_Surface> gfxCreateSurface(int width, int height, bool forceSoftware = false);


///////////////////
// Creates an ARGB 32bit surface if screen supports no alpha or a surface like screen
SmartPointer<SDL_Surface> gfxCreateSurfaceAlpha(int width, int height, bool forceSoftware = false); 

////////////////////
// Destroys a surface
// Now with SmartPointer usage everywhere this function is forbidden!
/*
inline void gfxFreeSurface(const SmartPointer<SDL_Surface> & surf)  {
	if (surf == NULL)
		return;

	#ifdef DEBUG_SMARTPTR
	printf("gfxFreeSurface() %p\n", surf.get() );
	#endif
	SDL_FreeSurface(surf);
	//surf = NULL; // That's a hack that won't fix anything
}
*/

//
// Image drawing
//

///////////////
// Copies one surface to another (not blitting, so the alpha values are kept!)
void CopySurface(SDL_Surface * dst, SDL_Surface * src, int sx, int sy, int dx, int dy, int w, int h);
inline void CopySurface(SDL_Surface * dst, const SmartPointer<SDL_Surface> & src, int sx, int sy, int dx, int dy, int w, int h){
	CopySurface(dst, src.get(), sx, sy, dx, dy, w, h);
}

//////////////
// Draw the image with a huge amount of options
void DrawImageAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, SDL_Rect& rDest, SDL_Rect& rSrc);

inline void DrawImageAdv(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, SDL_Rect& rDest, SDL_Rect& rSrc) {
	DrawImageAdv(bmpDest, bmpSrc.get(), rDest, rSrc);
}

//////////////
// Draw the image with a huge amount of options
inline void DrawImageAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	SDL_Rect r1 = { (SDLRect::Type) dx, (SDLRect::Type) dy, 0, 0 };
	SDL_Rect r2 = { (SDLRect::Type) sx, (SDLRect::Type) sy, (SDLRect::TypeS) w, (SDLRect::TypeS) h };
	DrawImageAdv( bmpDest, bmpSrc, r1, r2);
}
inline void DrawImageAdv(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	DrawImageAdv(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}


///////////////
// Draw the image, with more options
inline void DrawImageEx(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int x, int y, int w, int h) {
	DrawImageAdv(bmpDest, bmpSrc, 0, 0, x, y, w, h);
}
inline void DrawImageEx(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int x, int y, int w, int h) {
	DrawImageEx(bmpDest, bmpSrc.get(), x, y, w, h);
}

///////////////
// Simply draw the image
inline void DrawImage(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, SDL_Rect& rDest) {
	if(!bmpDest) {
		errors << "DrawImage: bmpDest not set" << endl;
		return;
	}
	if(!bmpSrc) {
		errors << "DrawImage: bmpSrc not set" << endl;
		return;
	}
	SDL_BlitSurface(bmpSrc, NULL, bmpDest, &rDest);
}
inline void DrawImage(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, SDL_Rect& rDest) {
	DrawImage(bmpDest, bmpSrc.get(), rDest);
}

///////////////
// Simply draw the image
inline void DrawImage(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int x, int y) {
	SDL_Rect r = { (SDLRect::Type) x, (SDLRect::Type) y, 0, 0 };
	DrawImage( bmpDest, bmpSrc, r);
}
inline void DrawImage(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int x, int y) {
	DrawImage(bmpDest, bmpSrc.get(), x, y);
}

///////////////
// Draws image mirror flipped
// WARNING: passing invalid source x/y/w/h causes a segfault
void DrawImageAdv_Mirror(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
inline void DrawImageAdv_Mirror(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	DrawImageAdv_Mirror(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}

////////////////
// Draws the image doubly stretched (fast)
void DrawImageStretch2(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
inline void DrawImageStretch2(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	DrawImageStretch2(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}

/////////////////
// Draws the image doubly stretched while checking for colorkey
void DrawImageStretch2Key(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
inline void DrawImageStretch2Key(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	DrawImageStretch2Key(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}

/////////////////
// Draws image doubly stretched, mirrored and checking for colorkey
// WARNING: passing invalid source x/y/w/h causes a segfault
void DrawImageStretchMirrorKey(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
inline void DrawImageStretchMirrorKey(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	DrawImageStretchMirrorKey(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}

inline SmartPointer<SDL_Surface> GetCopiedImage(SDL_Surface* bmpSrc) {
	SmartPointer<SDL_Surface> result = SDL_CreateRGBSurface(
															bmpSrc->flags,
															bmpSrc->w, bmpSrc->h,
															bmpSrc->format->BitsPerPixel,
															bmpSrc->format->Rmask,
															bmpSrc->format->Gmask,
															bmpSrc->format->Bmask,
															bmpSrc->format->Amask);
	if (result.get() == NULL) return NULL;
	CopySurface(result.get(), bmpSrc, 0, 0, 0, 0, bmpSrc->w, bmpSrc->h);
	return result;
}
inline SmartPointer<SDL_Surface> GetCopiedImage(const SmartPointer<SDL_Surface> & bmpSrc) {
	return GetCopiedImage(bmpSrc.get());
}

/////////////////
// Creates a new surface of the same size and draws the image mirror flipped onto it
inline SmartPointer<SDL_Surface> GetMirroredImage(SDL_Surface* bmpSrc)  {
	SmartPointer<SDL_Surface> result = SDL_CreateRGBSurface(
															bmpSrc->flags,
															bmpSrc->w, bmpSrc->h,
															bmpSrc->format->BitsPerPixel,
															bmpSrc->format->Rmask,
															bmpSrc->format->Gmask,
															bmpSrc->format->Bmask,
															bmpSrc->format->Amask);
	if (result.get() == NULL) return NULL;
	DrawImageAdv_Mirror(result.get(), bmpSrc, 0, 0, 0, 0, bmpSrc->w, bmpSrc->h);
	return result;
}
inline SmartPointer<SDL_Surface> GetMirroredImage(const SmartPointer<SDL_Surface> & bmpSrc) {
	return GetMirroredImage(bmpSrc.get());
}

/////////////////
// Draws a sprite doubly stretched but not so advanced
inline void	DrawImageStretch(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int dx, int dy) {
	DrawImageStretch2(bmpDest,bmpSrc,0,0,dx,dy,bmpSrc->w,bmpSrc->h);
}
inline void	DrawImageStretch(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int dx, int dy) {
	DrawImageStretch(bmpDest, bmpSrc.get(), dx, dy);
}

/////////////////
// Draws a sprite doubly stretched, with a colour key and not so advanced
inline void	DrawImageStretchKey(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int dx, int dy) {
	DrawImageStretch2Key(bmpDest, bmpSrc, 0, 0, dx, dy, bmpSrc->w, bmpSrc->h);
}
inline void	DrawImageStretchKey(SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int dx, int dy) {
	DrawImageStretchKey(bmpDest, bmpSrc.get(), dx, dy);
}

/////////////////
// Draws the image resized according to ratios
void DrawImageResizedAdv( SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio);
void DrawImageResizedAdv( SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh);
inline void DrawImageResizedAdv( SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio) {
	DrawImageResizedAdv( bmpDest, bmpSrc.get(), sx, sy, dx, dy, sw, sh, xratio, yratio);
}
inline void DrawImageResizedAdv( SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh) {
	DrawImageResizedAdv( bmpDest, bmpSrc.get(), sx, sy, dx, dy, sw, sh, dw, dh);
}

/////////////////
// Draws the image nicely resampled, blur says how much the result should be blurred
void DrawImageResampledAdv( SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio);
void DrawImageResampledAdv( SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh);
inline void DrawImageResampledAdv( SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio) {
	DrawImageResampledAdv( bmpDest, bmpSrc.get(), sx, sy, dx, dy, sw, sh, xratio, yratio );
}
inline void DrawImageResampledAdv( SDL_Surface * bmpDest, const SmartPointer<SDL_Surface> & bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh)  {
	DrawImageResampledAdv( bmpDest, bmpSrc.get(), sx, sy, dx, dy, sw, sh, dw, dh );
}

//////////////////
// Draws the image in double size using the scale2x algorithm
void DrawImageScale2x(SDL_Surface* bmpDest, SDL_Surface* bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
inline void DrawImageScale2x(SDL_Surface* bmpDest, const SmartPointer<SDL_Surface>& bmpSrc, int sx, int sy, int dx, int dy, int w, int h)  {
	DrawImageScale2x(bmpDest, bmpSrc.get(), sx, sy, dx, dy, w, h);
}

void DrawImageScaleHalf(SDL_Surface* bmpDest, SDL_Surface* bmpSrc);

///////////////////
// Tiles the source image onto the dest image
void DrawImageTiled(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
void DrawImageTiledX(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
void DrawImageTiledY(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);

inline void DrawImageTiled(SDL_Surface *bmpDest, const SmartPointer<SDL_Surface>& bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)  {
	DrawImageTiled(bmpDest, bmpSrc.get(), sx, sy, sw, sh, dx, dy, dw, dh);
}
inline void DrawImageTiledX(SDL_Surface *bmpDest, const SmartPointer<SDL_Surface>& bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)  {
	DrawImageTiledX(bmpDest, bmpSrc.get(), sx, sy, sw, sh, dx, dy, dw, dh);
}

inline void DrawImageTiledY(SDL_Surface *bmpDest, const SmartPointer<SDL_Surface>& bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)  {
	DrawImageTiledX(bmpDest, bmpSrc.get(), sx, sy, sw, sh, dx, dy, dw, dh);
}

SmartPointer<SDL_Surface> GenerateShadowSurface(SDL_Surface *object, unsigned char opacity = 96);

//
// Pixel and color routines
//


////////////////////
// Get address of a pixel
inline Uint8 *GetPixelAddr(const SDL_Surface *surf, int x, int y) {
	return (Uint8 *)surf->pixels + y * surf->pitch + x * surf->format->BytesPerPixel;
}

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
inline void PutPixel(SDL_Surface * bmpDest, int x, int y, Uint32 color) {
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
// This function doesn't have "const SmartPointer<SDL_Surface> &" interface because it will slow it down
inline Uint32 GetPixel(SDL_Surface * bmpSrc, int x, int y) {
	return GetPixelFromAddr(
			(Uint8*)bmpSrc->pixels + y * bmpSrc->pitch + x * bmpSrc->format->BytesPerPixel,
			bmpSrc->format->BytesPerPixel);
}

////////////////
// Copy pixel from one surface to another, both surfaces must have same format
// WARNING: doesn't do clipping
// NOTE: dst must be locked before calling this
// This function doesn't have "const SmartPointer<SDL_Surface> &" interface because it will slow it down
inline void CopyPixel_SameFormat(
	SDL_Surface * dst, SDL_Surface * src,
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
// This function doesn't have "const SmartPointer<SDL_Surface> &" interface because it will slow it down
inline void CopyPixel_SameFormat(
	SDL_Surface * dst, SDL_Surface * src, int x, int y) {
	CopyPixel_SameFormat(dst, src, x, y, x, y);
}


////////////////
// Put pixel alpha blended with the background
// WARNING: passing invalid coordinates will cause a segfault
// NOTE: dst must be locked before calling this
void PutPixelA(SDL_Surface * bmpDest, int x, int y, Uint32 colour, Uint8 a);


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
inline bool IsTransparent(SDL_Surface * surf, Uint32 color)  {
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
void	DrawHLine(SDL_Surface * bmpDest, int x, int x2, int y, Color colour);

///////////////////
// Draw vertical line
void	DrawVLine(SDL_Surface * bmpDest, int y, int y2, int x, Color colour);

///////////////////
// Draw a line
void	DrawLine(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Color color);

//////////////////
// Draw the line nicely antialiased
void	AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8));

/////////////////////
// Draws a filled rectangle
void	DrawRectFill(SDL_Surface * bmpDest, int x, int y, int x2, int y2, Color color);

////////////////////
// Very fast routine for drawing 2x2 rects
void DrawRectFill2x2(SDL_Surface *bmpDest, int x, int y, Color color);
void DrawRectFill2x2_NoClip(SDL_Surface *bmpDest, int x, int y, Color color);

// x,y is center
void DrawCircleFilled(SDL_Surface* bmpDest, int x, int y, int rx, int ry, Color color);

// draws a cross ("X")
void DrawCross(SDL_Surface* bmpDest, int x, int y, int w, int h, Color c);

// draw a simple loading animation
enum LoadingAniType { LAT_CIRCLES, LAT_CAKE };
void DrawLoadingAni(SDL_Surface* bmpDest, int x, int y, int rx, int ry, Color fg, Color bg, LoadingAniType type);

struct ScopedBackgroundLoadingAni {
	struct Data; Data* data;
	ScopedBackgroundLoadingAni(int x, int y, int rx, int ry, Color fg, Color bg, LoadingAniType type = LAT_CIRCLES);
	~ScopedBackgroundLoadingAni();
};


/////////////////////
// Draws a simple linear gradient
void DrawLinearGradient(SDL_Surface *bmpDest, int x, int y, int w, int h, Color cl1, Color cl2, GradientDirection dir);

////////////////////
// Fills the surface with specified colour
inline void FillSurface(SDL_Surface * dst, Color colour) {
	if (dst->clip_rect.w > 0 && dst->clip_rect.h > 0)
		SDL_FillRect(dst, NULL, colour.get(dst->format));
}

inline void FillSurface(SDL_Surface * dst, Uint32 colour) {
	if (dst->clip_rect.w > 0 && dst->clip_rect.h > 0)
		SDL_FillRect(dst, NULL, colour);
}

////////////////////
// Fills the whole surface with a transparent color
inline void FillSurfaceTransparent(SDL_Surface * dst)  {
	// check alpha first as it has priority (if set, colorkey is ignored)
	if (dst->flags & SDL_SRCALPHA)
		FillSurface(dst, SDL_MapRGBA(dst->format, 255, 0, 255, SDL_ALPHA_TRANSPARENT));
	else if (dst->flags & SDL_SRCCOLORKEY)
		FillSurface(dst, COLORKEY(dst));
	else
		warnings("There's no possibility to make this surface transparent!\n");
}


////////////////////
// Draws a rectangle
inline void	DrawRect(SDL_Surface * bmpDest, int x, int y, int x2, int y2, Color colour) {
	DrawHLine(bmpDest, x, x2, y, colour);
	DrawHLine(bmpDest, x, x2, y2, colour);
	DrawVLine(bmpDest, y, y2, x, colour);
	DrawVLine(bmpDest, y, y2, x2, colour);
}

///////////////////
// Draws a rectangle with transparency
inline void DrawRectFillA(SDL_Surface * bmpDest, int x, int y, int x2, int y2, Uint32 color, Uint8 alpha)  {
	Color col(bmpDest->format, color); col.a = alpha;
	DrawRectFill(bmpDest, x, y, x2, y2, col);
}

inline void DrawRectFillA(SDL_Surface * bmpDest, int x, int y, int x2, int y2, Color color, Uint8 alpha)  {
	color.a = alpha;
	DrawRectFill(bmpDest, x, y, x2, y2, color);
}

//////////////////
// Draw a triangle
inline void DrawTriangle(SDL_Surface * bmpDest, int x1, int y1, int x2, int y2, int x3, int y3, Color colour) {
	DrawLine(bmpDest, x1, y1, x2, y2, colour);
	DrawLine(bmpDest, x2, y2, x3, y3, colour);
	DrawLine(bmpDest, x3, y3, x1, y1, colour);
}



//
// Special lines (rope, laser sight, beam)
//

void	DrawRope(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color);
void	DrawBeam(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color);
void	DrawLaserSight(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color);


//
// Colorkey handling
//

// sets alpha in a safe way for both non-alpha-surfaces and alpha-surfaces
// for non-alpha surfaces, it uses SDL_SetAlpha
// for real alphablended surfaces, that means this multiplies a/255 to each a-value
void SetPerSurfaceAlpha(SDL_Surface * dst, Uint8 a);

// set colorkey for both alpha-blended and non-alpha surfaces
// for non-alpha surfaces, SDL_SetAlpha is used
// for alpha surfaces, it applies to every pixel
void SetColorKey(SDL_Surface * dst, Uint8 r, Uint8 g, Uint8 b);

//////////////////
// Set's the game's default color key (pink) to the surface
// Works for both alpha and nonalpha surfaces
void SetColorKey(SDL_Surface * dst);

//////////////////
// Resets the alpha-channel and the colorkey
void ResetAlpha(SDL_Surface * dst);


struct ScopedSurfaceClip {
	SDL_Rect oldclip;
	SDL_Surface* surf;
	
	ScopedSurfaceClip(SDL_Surface* s, const SDL_Rect& rect) {
		surf = s;
		SDL_GetClipRect(s, &oldclip);
		SDL_SetClipRect(s, &rect);
	}
	~ScopedSurfaceClip() {
		SDL_SetClipRect(surf, &oldclip);		
	}
};



#endif  //  __GFXPRIMITIVES_H__
