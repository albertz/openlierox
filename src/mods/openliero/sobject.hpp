#ifndef LIERO_SOBJECT_HPP
#define LIERO_SOBJECT_HPP

#include "math.hpp"
#include "objectList.hpp"

struct Worm;

struct SObjectType
{
	void create(int x, int y, Worm* owner);
	
	int startSound;
	int numSounds;
	int animDelay;
	int startFrame;
	int numFrames;
	int detectRange;
	int damage;
	int blowAway;
	bool shadow;
	int shake;
	int flash;
	int dirtEffect;
	
	int id;
};

struct SObject : ObjectListBase
{
	void process();
	
	fixed x, y;
	int id; // type
	Worm* owner;
	int curFrame;
	int animDelay;
};

#endif // LIERO_SOBJECT_HPP
