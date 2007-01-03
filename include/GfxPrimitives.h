/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Graphics primitives
// Created 12/11/01
// By Jason Boettcher


#ifndef __GFXPRIMITIVES_H__
#define __GFXPRIMITIVES_H__


// Surface stuff
SDL_Surface *gfxCreateSurface(int width, int height);

// Image drawing
void	DrawImage(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y);
void	DrawImageEx(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int x, int y, int w, int h);
void	DrawImageAdv(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
void	DrawImageAdv_Mirror(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
void	DrawImageStretch(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy);
void	DrawImageStretch2(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h);
void	DrawImageStretchKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy, Uint16 key);
void	DrawImageStretch2Key(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint16 key);
void	DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint16 key);


// Solid drawing
void	DrawRectFill(SDL_Surface *bmpDest, int x, int y, int x2, int y2, int colour);
void	DrawRect(SDL_Surface *bmpDest, int x, int y, int x2, int y2, int colour);
void    DrawRectFillA(SDL_Surface *bmpDest, int x, int y, int x2, int y2, int color, int alpha);

void	DrawHLine(SDL_Surface *bmpDest, int x, int x2, int y, int colour);
void	DrawVLine(SDL_Surface *bmpDest, int x, int x2, int y, int colour);

void	DrawTriangle(SDL_Surface *bmpDest, int x1, int y1, int x2, int y2, int x3, int y3, Uint32 colour);
//void	DrawLine(SDL_Surface *bmpDest, int x, int y, int x2, int y2, int colour);


// Pixel drawing
void	PutPixel(SDL_Surface *bmpDest, int x, int y, int colour);
Uint32	GetPixel(SDL_Surface *bmpSrc, int x, int y);
Uint32	GetPixelFromAddr(Uint8 *p, int bpp);
void	RopePutPixel(SDL_Surface *bmpDest, int x, int y, int colour);
void	BeamPutPixel(SDL_Surface *bmpDest, int x, int y, int colour);
void	LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, int colour);


// Colour component
Uint32	MakeColour(Uint8 r, Uint8 g, Uint8 b);
void	GetColour4(Uint32 pixel, SDL_Surface *img, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);


// Line drawing
static int clipEncode (Sint16 x, Sint16 y, Sint16 left, Sint16 top, Sint16 right, Sint16 bottom);
static int clipLine(SDL_Surface *dst, Sint16 *x1, Sint16 *y1, Sint16 *x2, Sint16 *y2);
int		DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);

void	DrawRope(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawBeam(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	DrawLaserSight(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void	do_line(SDL_Surface *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(SDL_Surface *, int, int, int));

bool	SaveSurface(SDL_Surface *Image, char *FileName, int Format, bool Tournament);
//bool	SaveBMP(SDL_Surface *image, char *FileName)  {return SDL_SaveBMP(image,FileName) != -1; }

#endif  //  __GFXPRIMITIVES_H__
