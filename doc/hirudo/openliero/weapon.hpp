#ifndef LIERO_WEAPON_HPP
#define LIERO_WEAPON_HPP

#include "math.hpp"
#include "objectList.hpp"
#include <string>

struct Weapon
{
	enum
	{
		STNormal,
		STDType1,
		STSteerable,
		STDType2,
		STLaser
	};
	
	void fire(int angle, fixed velX, fixed velY, int speed, fixed x, fixed y, int owner);

	int detectDistance;
	bool affectByWorm;
	int blowAway;
	fixed gravity;
	bool shadow;
	bool laserSight;
	int launchSound;
	int loopSound;
	int exploSound;
	int speed;
	fixed addSpeed;
	int distribution;
	int parts;
	int recoil;
	int multSpeed;
	int delay;
	int loadingTime;
	int ammo;
	int createOnExp;
	int dirtEffect;
	int leaveShells;
	int leaveShellDelay;
	bool playReloadSound;
	bool wormExplode;
	bool explGround;
	bool wormCollide;
	int fireCone;
	bool collideWithObjects;
	bool affectByExplosions;
	int bounce;
	int timeToExplo;
	int timeToExploV;
	int hitDamage;
	int bloodOnHit;
	int startFrame;
	int numFrames;
	bool loopAnim;
	int shotType;
	int colourBullets;
	int splinterAmount;
	int splinterColour;
	int splinterType;
	int splinterScatter;
	int objTrailType;
	int objTrailDelay;
	int partTrailType;
	int partTrailObj;
	int partTrailDelay;
	
	int id;
	int computedLoadingTime;
	std::string name;
};

struct WObject : ObjectListBase
{
	void blowUpObject(int owner);
	void process();
	
	fixed x, y;
	fixed velX, velY;
	int id;
	int owner;
	int curFrame;
	int timeLeft;
};

#endif // LIERO_WEAPON_HPP
