#ifndef DEDSERV

#include "distortion.h"

#include <allegro.h>
#include <vector>
#include "util/math_func.h"
#include "gfx.h"
//#include "blitters/blitters.h"
#include "blitters/colors.h"

#include <string>

#include <iostream>
using std::cerr;
using std::endl;
using std::string;
//using namespace std;

DistortionMap* lensMap(int radius)
{
	DistortionMap* lens = new DistortionMap;

	for ( int y = 0; y < radius*2; ++y )
	for ( int x = 0; x < radius*2; ++x )
	{
		Vec delta = Vec( x - radius, y - radius );
		
		if ( delta.length() <= radius )
		{
			if (delta.length() != 0)
			{
				float newLength = ( radius - sqrt( radius * radius - delta.length() * delta.length() )) - delta.length();
				lens->map.push_back( delta.normal()*newLength );
			}else
				lens->map.push_back( Vec(0,0) );
		}else
			lens->map.push_back( Vec (0,0) );
	}
	lens->width = radius*2;
	return lens;
}

DistortionMap* swirlMap(int radius)
{
	DistortionMap* swirl = new DistortionMap;

	for ( int y = 0; y < radius*2; ++y )
	for ( int x = 0; x < radius*2; ++x )
	{
		Vec delta = Vec( x - radius, y - radius );
		Vec deltaPerp = delta.perp();
		
		float perpFactor;
		float normFactor;
		
		if ( delta.length() <= radius )
		{
			if (delta.length() != 0)
			{
				normFactor = delta.length() / radius;
				perpFactor = 1 - normFactor;
				
				Vec newPos = delta*normFactor + deltaPerp*perpFactor;
				
				swirl->map.push_back( newPos - delta );
			}else
				swirl->map.push_back( Vec(0,0) );
		}else
			swirl->map.push_back( Vec (0,0) );
	}
	swirl->width = radius*2;
	return swirl;
}

DistortionMap* spinMap(int radius)
{
	DistortionMap* spin = new DistortionMap;

	for ( int y = 0; y < radius*2; ++y )
	for ( int x = 0; x < radius*2; ++x )
	{
		Vec delta = Vec( x - radius, y - radius );
		
		float factor;
		
		if ( delta.length() <= radius )
		{
			if (delta.length() != 0)
			{
				factor = 1 - delta.length() / radius;
				
				Vec newPos = Vec( delta.getAngle() + Angle(180.0 * factor), delta.length());
				
				spin->map.push_back( newPos - delta );
			}else
				spin->map.push_back( Vec(0,0) );
		}else
			spin->map.push_back( Vec (0,0) );
	}
	spin->width = radius*2;
	return spin;
}

DistortionMap* rippleMap(int radius, int frequency)
{
	DistortionMap* ripple = new DistortionMap;

	for ( int y = 0; y < radius*2; ++y )
	for ( int x = 0; x < radius*2; ++x )
	{
		Vec delta = Vec( x - radius, y - radius );
		
		float rippleFactor;
		
		if ( delta.length() <= radius )
		{
			if (delta.length() != 0)
			{
				rippleFactor = -cos(delta.length() * 2 * frequency * PI / radius) + 1;
				
				ripple->map.push_back( delta.normal() * rippleFactor * radius / frequency);
			}else
				ripple->map.push_back( Vec(0,0) );
		}else
			ripple->map.push_back( Vec (0,0) );
	}
	ripple->width = radius * 2;
	return ripple;
}

DistortionMap* randomMap(int radius)
{
	DistortionMap* lens = new DistortionMap;

	for ( int y = 0; y < radius*2; ++y )
	for ( int x = 0; x < radius*2; ++x )
	{
		lens->map.push_back( Vec(Angle(rnd()*360.0), rnd() * 2.0 ) );
	}
	lens->width = radius*2;
	return lens;
}

DistortionMap* bitmapMap(const string &filename)
{
	int currVdepth = get_color_depth();
	
	set_color_depth(32);
	
	DistortionMap* lens = new DistortionMap;
	
	BITMAP* heightMap = load_bitmap(filename.c_str(),0);

	if ( heightMap )
	{
		for ( int y = 0; y < heightMap->h; ++y )
		for ( int x = 0; x < heightMap->w; ++x )
		{
			Vec distort;
			float value;
			float s,h;
			int c = getpixel(heightMap,x-1,y);
			rgb_to_hsv(getr(c), getg(c), getb(c), &h, &s, &value);
			distort+= Vec(-1,0)*value;
			c = getpixel(heightMap,x+1,y);
			rgb_to_hsv(getr(c), getg(c), getb(c), &h, &s, &value);
			distort+= Vec(1,0)*value;
			c = getpixel(heightMap,x,y-1);
			rgb_to_hsv(getr(c), getg(c), getb(c), &h, &s, &value);
			distort+= Vec(0,-1)*value;
			c = getpixel(heightMap,x,y+1);
			rgb_to_hsv(getr(c), getg(c), getb(c), &h, &s, &value);
			distort+= Vec(0,1)*value;
			lens->map.push_back( distort*10 );
		}
		lens->width = heightMap->w;
		destroy_bitmap(heightMap);
	}else
	{
		lens->width = 0;
	}
	
	set_color_depth(currVdepth);
	
	return lens;
}

DistortionMap::CachedMapT const& DistortionMap::compileMap()
{
	std::vector<Vec>::const_iterator m = map.begin();
	
	CachedMapT& c = quantMap;
	c.clear();

	for(; m != map.end(); ++m)
	{
		int x = int(m->x * 256.f);
		int y = int(m->y * 256.f);
		c.push_back(std::make_pair(x, y));
	}
	
	return c;
}

Distortion::Distortion(DistortionMap* map)
: m_map(map)
{
	m_map->compileMap();
	width = m_map->width;
	height = m_map->map.size() / m_map->width;
	buffer = create_bitmap( width, height);
}

Distortion::~Distortion()
{
	destroy_bitmap( buffer );
	delete m_map;
}
/*
void Distortion::apply( BITMAP* where, int _x, int _y, float multiply = 1)
{
	_x -= width / 2;
	_y -= height / 2;
	int y = 0;
	Vec offset;
	while ( y * m_map->width < m_map->map.size() )
	{
		for ( int x = 0; x < m_map->width; ++x )
		{
			offset = m_map->map[x+y*m_map->width] * multiply;
			putpixel(buffer, x,y, getpixel(where,x+_x+offset.x,y+_y+offset.y));
		}
		++y;
	}
	blit(buffer, where, 0, 0, _x, _y, buffer->w, buffer->h);
}
*/
void Distortion::apply( BITMAP* where, int destx, int desty, float multiply = 1.f)
{
	destx -= width / 2;
	desty -= height / 2;

	DistortionMap::CachedMapT const& map = m_map->quantMap;
	
	int fmag = int(multiply * 256);
	
	int x1, y1, w, h;

	if(destx < 0)
	{
		x1 = -destx;
		w = width + destx;
		destx = 0;
	}
	else
	{
		x1 = 0;
		w = width;
	}
	if(desty < 0)
	{
		y1 = -desty;
		h = height + desty;
		desty = 0;
	}
	else
	{
		y1 = 0;
		h = height;
	}
	
	if(destx + w > where->w)
		w = where->w - destx;

	if(desty + h > where->h)
		h = where->h - desty;
	
	if(w <= 0 || h <= 0)
		return;

	std::pair<int, int> const* m = &map[x1 + y1 * width];
	
	size_t wrap = (width - w) * sizeof(std::pair<int, int>);
	
	int orgW = w;
	int orgH = h;
	
	unsigned char** buffer_line = buffer->line;
	unsigned char** where_line = where->line;
	unsigned int where_w = (unsigned int)where->w - 1;
	unsigned int where_h = (unsigned int)where->h - 1;
	
	int orgDestX = destx;
	int orgDestY = desty;

	if(!gfx.m_distortionAA)
	{
		#define SET16(v_) *dest++ = v_
		#define GET16(t_) (((unsigned int)(x_) < where_w && (unsigned int)(y_) < where_h) ? ((t_ *)where_line[y_])[x_] : 0)
		#define SET32(v_) *dest++ = v_
		#define GET32(t_) (((unsigned int)(x_) < where_w && (unsigned int)(y_) < where_h) ? ((t_ *)where_line[y_])[x_] : 0)
		#define WORK(get_, set_, t_) \
		for ( int y = y1; h-- > 0; ++y, ++desty ) { \
			w = orgW; \
			destx = orgDestX; \
			t_* dest = ((t_ *)buffer_line[y]) + x1; \
			for ( ; w-- > 0; ++m, ++destx) { \
				int x_ = destx + ((m->first * fmag) >> 16); \
				int y_ = desty + ((m->second * fmag) >> 16); \
				set_(get_(t_)); } \
			m = (std::pair<int, int> *)(((char *)m) + wrap); }
			
		#define WORK_GENERAL() \
		for ( int y = y1; h-- > 0; ++y, ++desty ) { \
			w = orgW; \
			destx = orgDestX; \
			for ( int x = x1; w-- > 0; ++m, ++destx, ++x) { \
				int x_ = destx + ((m->first * fmag) >> 16); \
				int y_ = desty + ((m->second * fmag) >> 16); \
				putpixel(buffer, x, y, getpixel(where, x_, y_)); } \
			m = (std::pair<int, int> *)(((char *)m) + wrap); }
		
		switch(bitmap_color_depth(buffer))
		{
			case 32: WORK(GET32, SET32, Pixel32); break;
			case 16: WORK(GET16, SET16, Pixel16); break;
			default: WORK_GENERAL(); break;
		}
		
		#undef WORK
		#undef GET32
		#undef SET32
	}
	else
	{
		switch(bitmap_color_depth(buffer))
		{
			case 32:
			{
				for ( int y = y1; h-- > 0; ++y, ++desty ) 
				{
					w = orgW;
					destx = orgDestX;
					Pixel32* dest = ((Pixel32 *)buffer_line[y]) + x1;
					for ( ; w-- > 0; ++m, ++destx)
					{
						int rx = (m->first * fmag);
						int ry = (m->second * fmag);
						int px = destx + (rx >> 16);
						int py = desty + (ry >> 16);
						
						if((unsigned int)px < where_w && (unsigned int)py < where_h)
						{
							rx = (rx >> 8) & 0xFF;
							ry = (ry >> 8) & 0xFF;
							Pixel32* urow = (Pixel32 *)where_line[py];
							Pixel32* lrow = (Pixel32 *)where_line[py + 1];
							Pixel ul = urow[px];
							Pixel ur = urow[px + 1];
							Pixel ll = lrow[px];
							Pixel lr = lrow[px + 1];
							
							Pixel u = Blitters::blendColorsFact_32(ul, ur, rx);
							Pixel l = Blitters::blendColorsFact_32(ll, lr, rx);
							Pixel p = Blitters::blendColorsFact_32(u, l, ry);
							
							//p = Blitters::blendColorsFact_32(((Pixel32 *)where_line[desty])[destx], p, 90);
							*dest++ = p;
						}
						else
						{
							*dest++ = 0;
						}
					}
					m = (std::pair<int, int> *)(((char *)m) + wrap);
				}
			}
			break;
			
			case 16:
			{
				for ( int y = y1; h-- > 0; ++y, ++desty ) 
				{
					w = orgW;
					destx = orgDestX;
					Pixel16* dest = ((Pixel16 *)buffer_line[y]) + x1;
					for ( ; w-- > 0; ++m, ++destx)
					{
						int rx = (m->first * fmag);
						int ry = (m->second * fmag);
						int px = destx + (rx >> 16);
						int py = desty + (ry >> 16);
						
						if((unsigned int)px < where_w && (unsigned int)py < where_h)
						{
							rx = (rx >> 11) & 31;
							ry = (ry >> 11) & 31;
							Pixel16* urow = (Pixel16 *)where_line[py];
							Pixel16* lrow = (Pixel16 *)where_line[py + 1];
							Pixel u = *(Pixel16_2 *)(urow + px);
							Pixel l = *(Pixel16_2 *)(lrow + px);
							
							// Merge the top row with the bottom row
							Pixel v = Blitters::blendColorsFact_16_2(u, l, ry);
							
							// Merge both columns
							// WARNING: Endian-assumption here, change (32-rx) to (rx) if on a big endian machine
							// We use 32-rx rather than 31-rx to avoid ugly artifacts with identity distortions
							Pixel p = Blitters::blendColorsFact_16(v, 32-rx);
							
							*dest++ = p;
						}
						else
						{
							*dest++ = 0;
						}
					}
					m = (std::pair<int, int> *)(((char *)m) + wrap);
				}
			}
			break;
		}
	}
	
	blit(buffer, where, x1, y1, orgDestX, orgDestY, orgW, orgH);
	//drawSprite_blend(where, buffer, orgDestX, orgDestY, 90);
}

#endif
