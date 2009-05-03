#ifndef LEVEL_H
#define LEVEL_H

#include "material.h"
#include "resource_locator.h"
#include "util/math_func.h"
//#include "sprite.h"

#include "encoding.h"

#include <allegro.h>
#include <string>
#include <vector>
#include <list>
#include <cmath>
#include "events.h"
#include <boost/array.hpp>
using boost::array;

class Sprite;
class LevelEffect;
class Viewport;
class BasePlayer;
struct BlitterContext;

struct WaterParticle
{
	WaterParticle( IVec pos, unsigned char material_ ) : x(pos.x), y(pos.y), mat(material_), count(0)
	{
		dir = static_cast<bool>( rndInt(2) );
	}
	
	WaterParticle( int x_, int y_, unsigned char material_ ) : x(x_), y(y_), mat(material_), count(0)
	{
		dir = static_cast<bool>( rndInt(2) );
	}
	
	int x;
	int y;
	bool dir; // true is right false is left
	unsigned char mat;
	int count;
};

struct SpawnPoint
{
	SpawnPoint( const Vec& pos_, int team_ )
	: pos( pos_ ), team(team_)
	{
	}
	Vec pos;
	int team;
	
	// more poop later
};

struct LevelConfig
{
	LevelConfig()
	: gameStart(0), gameEnd(0)
	, darkMode(false)
	{
	}
	
	~LevelConfig()
	{
		delete gameStart;
		delete gameEnd;
	}
	
	std::vector<SpawnPoint> spawnPoints;
	Event* gameStart;
	Event* gameEnd;
	bool darkMode;
};

class Level
{
	public:
		
	Level();
	~Level();
	/*
	bool load(const std::string &name);
	bool loadLiero(const std::string &name);*/
	void think();
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
	
	Vec getSpawnLocation(BasePlayer* player);
	
	Material const& getMaterial(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return m_materialList[(unsigned char)material->line[y][x]];
		else
			return m_materialList[0];
	}
	
	Material const& unsafeGetMaterial(unsigned int x, unsigned int y) const
	{
		return m_materialList[(unsigned char)material->line[y][x]];
	}
	
	unsigned char getMaterialIndex(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return (unsigned char)material->line[y][x];
		else
			return 0;
	}
	
	void putMaterial( unsigned char index, unsigned int x, unsigned int y )
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			material->line[y][x] = index;
	}
	
	void putMaterial( Material const& mat, unsigned int x, unsigned int y )
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			material->line[y][x] = mat.index;
	}
	
	bool isInside(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return true;
		else
			return false;
	}
	
	template<class PredT>
	bool trace(long srcx, long srcy, long destx, long desty, PredT predicate);
	
#ifndef DEDSERV
	void specialDrawSprite(Sprite* sprite, BITMAP* where, const IVec& pos, const IVec& matPos, BlitterContext const& blitter );
	
	void culledDrawSprite(Sprite* sprite, Viewport* viewport, const IVec& pos, int alpha);
	void culledDrawLight(Sprite* sprite, Viewport* viewport, const IVec& pos, int alpha);
	
	//void culledDrawLight(Sprite* sprite, BITMAP* where, const IVec& pos, const IVec& matPos);
#endif
	// applies the effect and returns true if it actually changed something on the map
	bool applyEffect( LevelEffect* effect, int x, int y);
	
	void loaderSucceeded();

	
#ifndef DEDSERV
	BITMAP* image;
	BITMAP* background;
	BITMAP* paralax;
	BITMAP* lightmap; // This has to be 8 bit.
	BITMAP* watermap; // How water looks in each pixel of the map
#endif
	BITMAP* material;
	Encoding::VectorEncoding vectorEncoding;
	Encoding::VectorEncoding intVectorEncoding;
	Encoding::DiffVectorEncoding diffVectorEncoding;
	
	struct ParticleBlockPredicate
	{
		bool operator()(Material const& m)
		{
			return !m.particle_pass;
		}
	};
	
	std::string name;
	std::string path;
	
	bool loaded;
	
	void setEvents( LevelConfig* events )
	{
		delete m_config;
		m_config = events;
	}
	
	LevelConfig* config()
	{ return m_config; }
	
private:
	
	
	void checkWBorders( int x, int y );
	
	array<Material, 256> m_materialList;
	
	LevelConfig *m_config;
	bool m_firstFrame;
	
	std::list<WaterParticle> m_water;
	static const float WaterSkipFactor = 0.05f;
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

#undef SIGN

extern ResourceLocator<Level> levelLocator;

#endif // _LEVEL_H_
