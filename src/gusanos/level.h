#ifndef LEVEL_H
#define LEVEL_H

#include "material.h"
#include "resource_locator.h"
#include "util/math_func.h"
//#include "sprite.h"

#include "encoding.h"

#include "gusanos/allegro.h"
#include <string>
#include <vector>
#include <list>
#include <cmath>
#include "events.h"
#include <boost/array.hpp>
using boost::array;

class Sprite;
class LevelEffect;
class CViewport;
class CWormInputHandler;
struct BlitterContext;

struct WaterParticle
{
	WaterParticle( IVec pos, unsigned char material_ ) : x(pos.x), y(pos.y), mat(material_), count(0)
	{
		dir = rndInt(2) != 0;
	}
	
	WaterParticle( int x_, int y_, unsigned char material_ ) : x(x_), y(y_), mat(material_), count(0)
	{
		dir = rndInt(2) != 0;
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
	: pos( pos_ ), team(team_) {}
	
	Vec pos;
	int team;
	
	// more poop later
};

struct LevelConfig
{
	LevelConfig() : darkMode(false)	{}
	
	std::vector<SpawnPoint> teamBases;
	std::vector<SpawnPoint> spawnPoints;
	boost::shared_ptr<GameEvent> gameStart;
	boost::shared_ptr<GameEvent> gameEnd;
	bool darkMode;
};

class CMap;
extern ResourceLocator<CMap> levelLocator;

#endif // _LEVEL_H_
