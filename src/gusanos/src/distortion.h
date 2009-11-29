#ifndef DISTORTION_H
#define DISTORTION_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "util/vec.h"
#include <string>
#include <utility>
#include <map>
#include <vector>

struct BITMAP;

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
	
	void apply( BITMAP* where, int x, int y, float multiply );
	
	//void applyFast( BITMAP* where, int x, int y, float multiply );
	
	private:
	
	int width;
	int height;
	DistortionMap* m_map;
	BITMAP* buffer;
	
};

DistortionMap* lensMap(int radius);
DistortionMap* swirlMap(int radius);
DistortionMap* spinMap(int radius);
DistortionMap* rippleMap(int radius, int frequency = 3);
DistortionMap* randomMap(int radius);
DistortionMap* bitmapMap(const std::string &filename);

#endif // _DISTORTION_H_

