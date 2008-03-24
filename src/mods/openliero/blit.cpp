#include "gfx.hpp"
#include "game.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <cstring>
#include <cassert>

void drawBar(int x, int y, int width, int colour)
{
	if(width > 0)
	{
		std::memset(&gfx.getScreenPixel(x, y), colour, width);
		std::memset(&gfx.getScreenPixel(x, y+1), colour, width);
	}
}

void drawRoundedBox(int x, int y, int colour, int height, int width)
{
	height--;
	std::memset(&gfx.getScreenPixel(x+1,y), colour, width+1);
	for(long i=1; i<height; i++)
	{
		std::memset(&gfx.getScreenPixel(x,y+i), colour, width+3);
	}
	std::memset(&gfx.getScreenPixel(x+1,y+height), colour, width+1);
}

#define CLIP_IMAGE(clip) \
{ \
	int top = y - (clip).y; \
	if(top < 0)	{ \
		mem += -top * pitch; \
		height += top; \
		y = (clip).y; } \
	int bottom = y + height - ((clip).y + (clip).h); \
	if(bottom > 0) height -= bottom; \
	int left = x - (clip).x; \
	if(left < 0) { \
		mem -= left; \
		width += left; \
		x = (clip).x; } \
	int right = x + width - ((clip).x + (clip).w); \
	if(right > 0) width -= right; \
	if(width <= 0 || height <= 0) return; \
}

void blitImageNoKeyColour(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height, int pitch)
{
	CLIP_IMAGE(scr->clip_rect);

	PalIdx* scrptr = static_cast<PalIdx*>(scr->pixels) + y*scr->pitch + x;

	for(int y = 0; y < height; ++y)
	{
		std::memcpy(scrptr, mem, width);

		scrptr += scr->pitch;
		mem += pitch;
	}
}

void blitImage(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height)
{
	int pitch = width;
	
	CLIP_IMAGE(scr->clip_rect);

	PalIdx* scrptr = static_cast<PalIdx*>(scr->pixels) + y*scr->pitch + x;

	for(int y = 0; y < height; ++y)
	{
		PalIdx* rowdest = scrptr;
		PalIdx* rowsrc = mem;
		
		for(int x = 0; x < width; ++x)
		{
			PalIdx c = *rowsrc;
			if(c)
				*rowdest = c;
			++rowsrc;
			++rowdest;
		}

		scrptr += scr->pitch;
		mem += pitch;
	}
}

#define BLIT(body) do { \
	PalIdx* scrptr = static_cast<PalIdx*>(scr->pixels) + y*scr->pitch + x; \
	for(int y = 0; y < height; ++y)	{ \
		PalIdx* rowdest = scrptr; \
		PalIdx* rowsrc = mem; \
		for(int x = 0; x < width; ++x) { \
			PalIdx c = *rowsrc; \
			body \
			++rowsrc; \
			++rowdest; } \
		scrptr += scr->pitch; \
		mem += pitch; } } while(false)
		
#define BLIT2(pixels, destpitch, body) do { \
	PalIdx* scrptr = static_cast<PalIdx*>(pixels) + y*(destpitch) + x; \
	for(int y = 0; y < height; ++y)	{ \
		PalIdx* rowdest = scrptr; \
		PalIdx* rowsrc = mem; \
		for(int x = 0; x < width; ++x) { \
			PalIdx c = *rowsrc; \
			body \
			++rowsrc; \
			++rowdest; } \
		scrptr += (destpitch); \
		mem += pitch; } } while(false)

void blitImageR(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height)
{
	int pitch = width;
	
	CLIP_IMAGE(scr->clip_rect);

	PalIdx* scrptr = static_cast<PalIdx*>(scr->pixels) + y*scr->pitch + x;

	for(int y = 0; y < height; ++y)
	{
		PalIdx* rowdest = scrptr;
		PalIdx* rowsrc = mem;
		
		for(int x = 0; x < width; ++x)
		{
			PalIdx c = *rowsrc;
			if(c && (PalIdx(*rowdest - 160) < 8))
				*rowdest = c;
			++rowsrc;
			++rowdest;
		}

		scrptr += scr->pitch;
		mem += pitch;
	}
}

void blitFireCone(SDL_Surface* scr, int fc, PalIdx* mem, int x, int y)
{
	int width = 16;
	int height = 16;
	int pitch = width;
	
	CLIP_IMAGE(scr->clip_rect);
	
	switch(fc)
	{
		case 0:
			BLIT( { if(c > 116) *rowdest = c - 5; } );
		break;
		
		case 1:
			BLIT( { if(c > 114) *rowdest = c - 3; } );
		break;
		
		case 2:
			BLIT( { if(c > 112) *rowdest = c - 1; } );
		break;
		
		default:
			BLIT( { if(c) *rowdest = c; } );
		break;
	}
}

void blitImageOnMap(PalIdx* mem, int x, int y, int width, int height)
{
	int pitch = width;
	SDL_Rect clipRect = {0, 0, game.level.width, game.level.height};
	
	CLIP_IMAGE(clipRect);
	
	BLIT2(&game.level.data[0], game.level.width,
	{
		if(c)
		{
			if(game.materials[*rowdest].dirtBack())
				*rowdest = c;
			else
				*rowdest = c + 3;
		}
	});
}

void blitShadowImage(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height)
{
	int pitch = width;
	
	CLIP_IMAGE(scr->clip_rect);

	PalIdx* scrptr = static_cast<PalIdx*>(scr->pixels) + y*scr->pitch + x;

	for(int y = 0; y < height; ++y)
	{
		PalIdx* rowdest = scrptr;
		PalIdx* rowsrc = mem;
		
		for(int x = 0; x < width; ++x)
		{
			PalIdx c = *rowsrc;
			if(c && game.materials[*rowdest].seeShadow()) // TODO: Speed up this test?
				*rowdest += 4;
			++rowsrc;
			++rowdest;
		}

		scrptr += scr->pitch;
		mem += pitch;
	}
}

void blitStone(bool p1, PalIdx* mem, int x, int y)
{
	int width = 16;
	int height = 16;
	int pitch = width;
	
	SDL_Rect clip = {0, 0, game.level.width, game.level.height};
	
	CLIP_IMAGE(clip);
	
	PalIdx* dest = &game.level.pixel(x, y);
	
	if(p1)
	{
		for(int y = 0; y < height; ++y)
		{
			PalIdx* rowdest = dest;
			PalIdx* rowsrc = mem;
			
			for(int x = 0; x < width; ++x)
			{
				PalIdx c = *rowsrc;
				if(c && game.materials[*rowdest].dirtBack()) // TODO: Speed up this test?
					*rowdest = c;
				else
					*rowdest = c + 3;
				++rowsrc;
				++rowdest;
			}

			dest += game.level.width;
			mem += pitch;
		}
	}
	else
	{
		for(int y = 0; y < height; ++y)
		{
			PalIdx* rowdest = dest;
			PalIdx* rowsrc = mem;
			
			for(int x = 0; x < width; ++x)
			{
				PalIdx c = *rowsrc;
				if(c)
					*rowdest = c;
				
				++rowsrc;
				++rowdest;
			}

			dest += game.level.width;
			mem += pitch;
		}
	}
}

void drawDirtEffect(int dirtEffect, int x, int y)
{
	Texture& tex = game.textures[dirtEffect];
	PalIdx* tFrame = gfx.largeSprites.spritePtr(tex.sFrame + game.rand(tex.rFrame));
	PalIdx* mFrame = gfx.largeSprites.spritePtr(tex.mFrame);
	
	// TODO: Optimize this
	
	if(tex.nDrawBack)
	{
		for(int cy = 0; cy < 16; ++cy)
		{
			int my = cy + y;
			if(my >= game.level.height - 1)
				break;
				
			if(my < 0)
				continue;
			
			for(int cx = 0; cx < 16; ++cx)
			{
				int mx = cx + x;
				if(mx >= game.level.width)
					break;
					
				if(mx < 0)
					continue;
					
				switch(mFrame[(cy << 4) + cx])
				{
				case 6:
					if(game.materials[game.level.pixel(mx, my)].anyDirt())
					{
						game.level.pixel(mx, my) = tFrame[((my & 15) << 4) + (mx & 15)];
					}
				break;
				
				case 1:
					PalIdx& pix = game.level.pixel(mx, my);
					if(game.materials[pix].dirt())
						pix = 1;
					if(game.materials[pix].dirt2())
						pix = 2;
				}
			}
		}
	}
	else
	{
		for(int cy = 0; cy < 16; ++cy)
		{
			int my = cy + y;
			if(my >= game.level.height - 1)
				break;
				
			if(my < 0)
				continue;
			
			for(int cx = 0; cx < 16; ++cx)
			{
				int mx = cx + x;
				if(mx >= game.level.width)
					break;
					
				if(mx < 0)
					continue;
					
				switch(mFrame[(cy << 4) + cx])
				{
				case 10:
				case 6:
					if(game.materials[game.level.pixel(mx, my)].background())
					{
						game.level.pixel(mx, my) = tFrame[((my & 15) << 4) + (mx & 15)];
					}
				break;
				
				case 2:
				{
					PalIdx& pix = game.level.pixel(mx, my);
					if(game.materials[pix].background())
						pix = 2;
				}
				break;
				
				case 1:
				{
					PalIdx& pix = game.level.pixel(mx, my);
					if(game.materials[pix].background())
						pix = 1;
				}
				break;
				}
			}			
		}
	}
}

void correctShadow(Rect rect)
{
	rect.intersect(Rect(0, 3, game.level.width - 3, game.level.height));
	
	Level& lev = game.level;
	
	for(int x = rect.x1; x < rect.x2; ++x)
	for(int y = rect.y1; y < rect.y2; ++y)
	{
		PalIdx& pix = lev.pixel(x, y);
		if(game.materials[pix].seeShadow()
		&& game.materials[lev.pixel(x + 3, y - 3)].dirtRock())
		{
			pix += 4;
		}
		else if(pix >= 164 // Remove shadow
		&& pix <= 167
		&& !game.materials[lev.pixel(x + 3, y - 3)].dirtRock())
		{
			pix -= 4;
		}
	}
}

inline int sign(int v) { return v < 0 ? -1 : (v > 0 ? 1 : 0); }

bool isInside(SDL_Rect const& rect, int x, int y)
{
	return static_cast<unsigned int>(x - rect.x) < rect.w
	    && static_cast<unsigned int>(y - rect.y) < rect.h;
}

#define DO_LINE(body_) { \
int cx = fromX; \
int cy = fromY; \
int dx = toX - fromX; \
int dy = toY - fromY; \
int sx = sign(dx); \
int sy = sign(dy); \
dx = std::abs(dx); \
dy = std::abs(dy); \
if(dx > dy) { \
	int c = -(dx >> 1); \
	while(cx != toX) { \
		c += dy; \
		cx += sx; \
		if(c > 0) { \
			cy += sy; \
			c -= dx; } \
		body_ } \
} else { \
	int c = -(dy >> 1); \
	while(cy != toY) { \
		c += dx; \
		cy += sy; \
		if(c > 0) { \
			cx += sx; \
			c -= dy; } \
		body_ } } }

void drawNinjarope(int fromX, int fromY, int toX, int toY)
{
	int colour = C[NRColourBegin];
	
	SDL_Rect& clip = gfx.screen->clip_rect;
	PalIdx* ptr = gfx.screenPixels;
	unsigned int pitch = gfx.screenPitch;
	
	
	DO_LINE({
		if(++colour == C[NRColourEnd])
			colour = C[NRColourBegin];
			
		if(isInside(clip, cx, cy))
			ptr[cy*pitch + cx] = colour;
	});
}

void drawLaserSight(int fromX, int fromY, int toX, int toY)
{
	SDL_Rect& clip = gfx.screen->clip_rect;
	PalIdx* ptr = gfx.screenPixels;
	unsigned int pitch = gfx.screenPitch;
	
	
	DO_LINE({
		
		if(gfx.rand(5) == 0)
		{
			if(isInside(clip, cx, cy))
				ptr[cy*pitch + cx] = gfx.rand(2) + 83;
		}
	});
}

void drawShadowLine(int fromX, int fromY, int toX, int toY)
{
	SDL_Rect& clip = gfx.screen->clip_rect;
	PalIdx* ptr = gfx.screenPixels;
	unsigned int pitch = gfx.screenPitch;
	
	
	DO_LINE({
		if(isInside(clip, cx, cy))
		{
			PalIdx& pix = ptr[cy*pitch + cx];
			if(game.materials[pix].seeShadow())
				pix += 4;
		}
	});
}

void drawLine(int fromX, int fromY, int toX, int toY, int colour)
{
	SDL_Rect& clip = gfx.screen->clip_rect;
	PalIdx* ptr = gfx.screenPixels;
	unsigned int pitch = gfx.screenPitch;
	
	
	DO_LINE({
		if(isInside(clip, cx, cy))
		{
			ptr[cy*pitch + cx] = colour;
		}
	});
}
