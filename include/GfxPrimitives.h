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

#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>


// Surface stuff

// Creates a buffer with the same details as the screen
inline SDL_Surface* gfxCreateSurface(int width, int height) {
	SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;

	return SDL_CreateRGBSurface(iSurfaceFormat, width, height, 
		fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
}


// Creates a buffer with the same details as screen, but with alpha channel
inline SDL_Surface* gfxCreateSurfaceAlpha(int width, int height) {
	SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 alpha = 0x000000ff;
#else
	// strange, isn't it?
	const Uint32 alpha = 0xff000000;
#endif

	return SDL_CreateRGBSurface(iSurfaceFormat | SDL_SRCALPHA,
		width, height, fmt->BitsPerPixel + 8,
		fmt->Rmask, fmt->Gmask, fmt->Bmask, alpha);
}



// Image drawing

// Simply draw the image
inline void DrawImage(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y) {
	static SDL_Rect	rDest;
	rDest.x = x; rDest.y = y;
	SDL_BlitSurface(bmpSrc,NULL,bmpDest,&rDest);
}

// Draw the image, with more options
inline void DrawImageEx(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y, int w, int h) {
	static SDL_Rect	rDest, rSrc;
	rDest.x = x; rDest.y = y;
	rSrc.x = 0; rSrc.y = 0;
	rSrc.w = w; rSrc.h = h;
	SDL_BlitSurface(bmpSrc,&rSrc,bmpDest,&rDest);
}

// Draw the image with a huge amount of options
inline void DrawImageAdv(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	static SDL_Rect	rDest, rSrc;
	rDest.x = dx; rDest.y = dy;
	rSrc.x = sx; rSrc.y = sy;
	rSrc.w = w; rSrc.h = h;
	SDL_BlitSurface(bmpSrc,&rSrc,bmpDest,&rDest);
}

// Draw the image mirrored with a huge amount of options
inline SDL_Surface* GetMirroredImage(SDL_Surface *bmpSrc) {
	return rotozoomSurfaceXY(bmpSrc, 0, -1, 1, 0);
}


inline SDL_Surface* GetStretched2Image(SDL_Surface* src) {
	return zoomSurface(src, 2, 2, 0);
}

inline SDL_Surface* GetStretched2Image(SDL_Surface* src, int x, int y, int w, int h) {
	if(x == 0 && y == 0 && w == src->w && h == src->h)
		return zoomSurface(src, 2, 2, 0);
	else {
		SDL_Surface* tmp = gfxCreateSurface(w,h);
		DrawImageEx(tmp, src, x, y, w, h);
		SDL_Surface* stretched_surf = zoomSurface(tmp, 2, 2, 0);
		SDL_FreeSurface(tmp);
		return stretched_surf;
	}
}

inline void DrawImageStretch2(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h) {
	SDL_Surface* stretched_surf = GetStretched2Image(bmpSrc, sx, sy, w, h);
	DrawImageEx(bmpDest, stretched_surf, dx, dy, w, h);
	SDL_FreeSurface(stretched_surf);	
}

inline void DrawImageStretch2Key(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint32 key) {
	SDL_Surface* stretched_surf = GetStretched2Image(bmpSrc, sx, sy, w, h);
	SDL_SetColorKey(stretched_surf, SDL_SRCCOLORKEY, key);
	DrawImageEx(bmpDest, stretched_surf, dx, dy, w, h);
	SDL_FreeSurface(stretched_surf);
}

inline void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint32 key) {
	SDL_Surface* stretched_surf = GetStretched2Image(bmpSrc, sx, sy, w, h);
	SDL_Surface* mirrored_surf = GetMirroredImage(stretched_surf);
	SDL_FreeSurface(stretched_surf);
	SDL_SetColorKey(mirrored_surf, SDL_SRCCOLORKEY, key);
	DrawImageEx(bmpDest, mirrored_surf, dx, dy, w, h);
	SDL_FreeSurface(mirrored_surf);
}



// Draws a sprite doubly stretched but not so advanced
inline void	DrawImageStretch(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy) {
	DrawImageStretch2(bmpDest,bmpSrc,0,0,dx,dy,bmpSrc->w,bmpSrc->h);
}

// Draws a sprite doubly stretched, with a colour key and not so advanced
inline void	DrawImageStretchKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy, Uint32 key) {
	DrawImageStretch2Key(bmpDest,bmpSrc,0,0,dx,dy,bmpSrc->w,bmpSrc->h,key);
}


// Solid drawing
inline void	DrawRectFill(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 color) {
	boxColor(bmpDest, x,y,x2,y2, color);
}

inline void	DrawRect(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 colour) {
	rectangleColor(bmpDest, x,y,x2,y2, colour);
}

inline void DrawRectFillA(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Uint32 color, int alpha) {
	Uint8 r,g,b;
	SDL_GetRGB(color, SDL_GetVideoSurface()->format, &r,&g,&b);
	rectangleRGBA(bmpDest, x,y,x2,y2, r,g,b,alpha);
}

inline void	DrawHLine(SDL_Surface *bmpDest, int x, int x2, int y, Uint32 colour) {
	hlineColor(bmpDest, x,x2,y, colour);
}

inline void	DrawVLine(SDL_Surface *bmpDest, int y, int y2, int x, Uint32 colour) {
	vlineColor(bmpDest, x,y,y2, colour);
}

inline void DrawTriangle(SDL_Surface *bmpDest, int x1, int y1, int x2, int y2, int x3, int y3, Uint32 colour) {
	trigonColor(bmpDest, x1,y1, x2,y2, x3,y3, colour);
}


// Pixel drawing
inline void PutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour) {
	pixelColor(bmpDest, x,y, colour);
}


// Get a pixel from an 8bit address
inline Uint32 GetPixelFromAddr(Uint8 *p, int bpp) {
	switch(bpp) {
		// 8 bpp
		case 1:
			return *p;

		// 16 bpp
		case 2:
			return *(Uint16 *)p;

		// 24 bpp
		case 3:
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				return p[0] << 16 | p[1] << 8 | p[2];
			#else
				return p[0] | p[1] << 8 | p[2] << 16;
			#endif

		// 32 bpp
		case 4:
			return *(Uint32 *)p;

		default:
			return 0;
	}
}

// Get a pixel from the surface
inline Uint32 GetPixel(SDL_Surface *bmpSrc, int x, int y) {
    int bpp = bmpSrc->format->BytesPerPixel;
	return GetPixelFromAddr((Uint8 *)bmpSrc->pixels + y * bmpSrc->pitch + x * bpp, bpp);
}




// Extract 4 colour components from a packed int
inline void GetColour4(Uint32 pixel, SDL_Surface *img, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
	SDL_GetRGBA(pixel,img->format,r,g,b,a);
}

// Creates a int colour based on the 3 components
inline Uint32 MakeColour(Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(SDL_GetVideoSurface()->format,r,g,b);
}

inline Uint32 MakeColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_MapRGBA(SDL_GetVideoSurface()->format,r,g,b,a);
}

// Line drawing
inline void DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	aalineColor(dst, x1,y1, x2,y2, color);
}

// Line drawing
inline void FastDrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	lineColor(dst, x1,y1, x2,y2, color);
}




void	RopePutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour);
void	BeamPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour);
void	LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour);

void	DrawRope(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawBeam(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawLaserSight(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);

bool	SaveSurface(SDL_Surface *Image, const std::string& FileName, int Format, bool Tournament);

#endif  //  __GFXPRIMITIVES_H__
