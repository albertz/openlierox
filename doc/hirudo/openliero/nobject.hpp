#ifndef LIERO_NOBJECT_HPP
#define LIERO_NOBJECT_HPP

#include "math.hpp"
#include "objectList.hpp"

struct NObjectType
{
	void create1(fixed velX, fixed velY, int x, int y, int colour, int owner);
	void create2(int angle, fixed velX, fixed velY, fixed x, fixed y, int colour, int owner);

	int detectDistance;
	fixed gravity;
	int speed;
	int speedV;
	int distribution;
	int blowAway;
	int bounce;
	int hitDamage;
	bool wormExplode;
	bool explGround;
	bool wormDestroy;
	int bloodOnHit;
	int startFrame;
	int numFrames;
	bool drawOnMap;
	int colourBullets;
	int createOnExp;
	bool affectByExplosions;
	int dirtEffect;
	int splinterAmount;
	int splinterColour;
	int splinterType;
	bool bloodTrail;
	int bloodTrailDelay;
	int leaveObj;
	int leaveObjDelay;
	int timeToExplo;
	int timeToExploV;
	
	int id;
};

struct NObject : ObjectListBase
{
	void process();
	
	fixed x, y;
	fixed velX, velY;
	int timeLeft;
	int id;
	int owner;
	int curFrame;
};

#endif // LIERO_NOBJECT_HPP
