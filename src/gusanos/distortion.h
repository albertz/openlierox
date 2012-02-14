#ifndef DISTORTION_H
#define DISTORTION_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "CVec.h"
#include <string>
#include <utility>
#include <map>
#include <vector>

struct ALLEGRO_BITMAP;

struct DistortionMap
{
	typedef std::vector<std::pair<int, int> > CachedMapT;
	std::vector<Vec> map;
	CachedMapT quantMap;
	CachedMapT const& compileMap();
	
	int width;
};

class Distortion
{
	public:

	Distortion(DistortionMap* map);
	~Distortion();
	
	void apply( ALLEGRO_BITMAP* where, int x, int y, float multiply );
	
	//void applyFast( ALLEGRO_BITMAP* where, int x, int y, float multiply );
	
	private:
	
	int width;
	int height;
	DistortionMap* m_map;
	ALLEGRO_BITMAP* buffer;
	
};

DistortionMap* lensMap(int radius);
DistortionMap* swirlMap(int radius);
DistortionMap* spinMap(int radius);
DistortionMap* rippleMap(int radius, int frequency = 3);
DistortionMap* randomMap(int radius);
DistortionMap* bitmapMap(const std::string &filename);

#endif // _DISTORTION_H_

