#ifndef LEVEL_H
#define LEVEL_H

#include "material.h"
#include "../util/vec.h"

#include <allegro.h>
#include <string>
#include <vector>
#include <cmath>
#include <boost/array.hpp>
using boost::array;

class Level
{
	public:
		
	Level();
	~Level();
	/*
	bool load(const std::string &name);
	bool loadLiero(const std::string &name);*/
	void unload();
	bool isLoaded();
#ifndef DEDSERV
	void draw(BITMAP* where, int x, int y);
#endif
	int width();
	int height();
	
	const std::string &getPath();
	const std::string &getName();
	void setName(const std::string &_name);
	/*
	const Material& getMaterial(int x, int y);*/
	
	Vec getSpawnLocation();
	
	Material const& getMaterial(unsigned int x, unsigned int y)
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return m_materialList[(unsigned char)material->line[y][x]];
		else
			return m_materialList[0];
	}
	
	Material const& unsafeGetMaterial(unsigned int x, unsigned int y)
	{
		return m_materialList[(unsigned char)material->line[y][x]];
	}
	
	bool isInside(unsigned int x, unsigned int y)
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return true;
		else
			return false;
	}
	
	template<class PredT>
	bool trace(long srcx, long srcy, long destx, long desty, PredT predicate);
	
	template<class PredT>
	bool preciseTrace(float srcx, float srcy, long destx, long desty, PredT predicate);
	
	// applies the effect and returns true if it actually changed something on the map
	
	void loaderSucceeded();
	
	//private:
		
	bool loaded;
	
	BITMAP* material;
	std::string name;
	std::string path;
	array<Material, 256> m_materialList;
	
	struct ParticleBlockPredicate
	{
		bool operator()(Material const& m)
		{
			return !m.particle_pass;
		}
	};
};

#define SIGN(x_) ((x_) < 0 ? -1 : (x_) > 0 ? 1 : 0)

template<class PredT>
bool Level::trace(long x, long y, long destx, long desty, PredT predicate)
{
	if(!isInside(x, y))
	{
		if(predicate(m_materialList[0]))
			return true;
		else
		{
			return true; //TODO: Clip the beginning of the line instead of returning
		}
	}
	if(!isInside(destx, desty))
	{
		if(predicate(m_materialList[0]))
			return true;
		else
		{
			return true; //TODO: Clip the end of the line instead of returning
		}
	}
		
	long xdiff = destx - x;
	long ydiff = desty - y;
	
	long sx = SIGN(xdiff);
	long sy = SIGN(ydiff);

	xdiff = labs(xdiff);
	ydiff = labs(ydiff);
	
	#define WORK(a, b) { \
		long i = a##diff >> 1; \
		long c = a##diff; \
		while(c-- >= 0) { \
			if(predicate(unsafeGetMaterial(x, y))) return true; \
			i -= b##diff; \
			a += s##a; \
			if(i < 0) b += s##b, i += a##diff; } }
	
	if(xdiff > ydiff)
		WORK(x, y)
	else
		WORK(y, x)

	#undef WORK

	return false;
}

template<class PredT>
bool Level::preciseTrace(float x, float y, long destx_, long desty_, PredT predicate)
{
	if(!isInside(x, y))
	{
		return true;
	}
	
	if ( predicate( getMaterial( (int)x, (int)y ) ) )
	{
		return true;
	}
	
	if ( predicate( getMaterial( destx_, desty_ ) ) )
	{
		return true;
	}
	
	float destx = destx_ + 0.5f;
	float desty = desty_ + 0.5f;
	
	Vec myLine = Vec(destx-x,desty-y);
	
	if ( x < destx )
	{
		if (  y < desty )
		{
			int mX = floor(x);
			int mY = floor(y);
		
			while ( mX <= destx_ && mY <= desty_ )
			{
			
				if ( predicate( getMaterial( mX, mY ) ) ) return true;
				
				Vec myPoint = Vec( (mX+1) - x, (mY+1) - y );
				
				if( myLine.perpDotProduct(myPoint) > 0 )
				{
					++mX;
				}else
				{
					++mY;
				}
				
			
			}
		}else
		{
			int mX = floor(x);
			int mY = ceil(y);
			
			while ( mX <= destx_ && mY >= desty_ )
			{
			
				if ( predicate( getMaterial( mX, mY ) ) ) return true;
				
				Vec myPoint = Vec( (mX+1) - x, (mY-1) - y );
				
				if( myLine.perpDotProduct(myPoint) < 0 )
				{
					++mX;
				}else
				{
					--mY;
				}
			
			}
		}
	}
	else 
	{
		if ( y < desty )
		{
			int mX = ceil(x);
			int mY = floor(y);
			
			while ( mX >= destx_ && mY <= desty_ )
			{
				
				if ( predicate( getMaterial( mX, mY ) ) ) return true;
				
				Vec myPoint = Vec( (mX-1) - x, (mY+1) - y );
				
				if( myLine.perpDotProduct(myPoint) < 0 )
				{
					--mX;
				}else
				{
					++mY;
				}
			
			}
		}else
		{
			int mX = ceil(x);
			int mY = ceil(y);
			
			while ( mX >= destx_ && mY >= desty_ )
			{
			
				if ( predicate( getMaterial( mX, mY ) ) ) return true;
				
				Vec myPoint = Vec( (mX-1) - x, (mY-1) - y );
				
				if( myLine.perpDotProduct(myPoint) > 0 )
				{
					--mX;
				}else
				{
					--mY;
				}
			
			}
		}
	}
	
	return false;
	
}

/*template<class PredT>
bool Level::preciseTrace(float x, float y, long destx_, long desty_, PredT predicate)
{
	if(!isInside(x, y))
	{
		return true;
	}
	
	if ( predicate( getMaterial( (int)x, (int)y ) ) )
	{
		return true;
	}
	
	float destx = destx_ + 0.5f;
	float desty = desty_ + 0.5f;
	
	float xDiff = x - destx;
	float yDiff = y - desty;
	
	if ( xDiff == 0 and yDiff == 0 )
	{
		return true;
	}
	
	
	if ( xDiff == 0 )
	{
		if ( trace((int)x, (int)y, destx_, desty_, predicate ) ) return true;
	}else if ( x < destx )
	{
		int myX = floor(x)+1;
		float myY = y + ( ( x - myX ) * yDiff / xDiff );
		float incY = yDiff / xDiff;
		
		while ( myX < destx )
		{
			if ( predicate( getMaterial( myX, floor(myY) ) ) )
			{
				return true;
			}
			++myX;
			myY += incY;
		}
	}
	else if ( x > destx )
	{
		int myX = floor(x);
		float myY = y + ( ( x - myX ) * yDiff / xDiff );
		float incY = yDiff / xDiff;
		
		while ( myX > destx )
		{
			if ( predicate( getMaterial( myX, floor(myY) ) ) )
			{
				return true;
			}
			--myX;
			myY -= incY;
		}
	}
	
	if ( yDiff == 0 )
	{
		if ( trace((int)x, (int)y, destx_, desty_, predicate ) ) return true;
	}else if ( y < desty )
	{
		int myY = floor(y)+1;
		float myX = x + ( ( y - myY ) * xDiff / yDiff );
		float incX = xDiff / yDiff;
		
		while ( myY < desty )
		{
			if ( predicate( getMaterial( floor(myX), myY ) ) )
			{
				return true;
			}
			++myY;
			myX += incX;
		}
	}else if ( y > desty )
	{
		int myY = floor(y);
		float myX = x + ( ( y - myY ) * xDiff / yDiff );
		float incX = xDiff / yDiff;
		
		while ( myY > desty )
		{
			if ( predicate( getMaterial( floor(myX), myY ) ) )
			{
				return true;
			}
			--myY;
			myX -= incX;
		}
	}

	return false;
}*/

#undef SIGN


#endif // _LEVEL_H_
