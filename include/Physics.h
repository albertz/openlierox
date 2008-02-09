/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <string>
#include "CVec.h"

class CWorm;
class CProjectile;
class CBonus;
class CNinjaRope;
class CMap;

// HINT: I call this class PhysicsEngine, though it doesn't matches the meaning
// of engine completely as I intented it for Hirudo. But it can probably easily
// reused later.

class PhysicsEngine {
public:
	static void Init(); // init general stuff and set default engine
	static void UnInit(); // deletes standard engine etc.
	
	// get/set the current instance of the PhysicsEngine (for example PhysicsLX56)
	static PhysicsEngine* Get();	
	static void Set(PhysicsEngine* engine);

// ------------
	virtual ~PhysicsEngine() {}
	
	virtual std::string name() = 0; // get a name of the implementation

	// TODO: later, we should have a class World and all objects and the map are included there
	virtual void simulateWorm(float dt, CWorm* worm, CWorm *worms, int local) = 0;
	virtual void simulateWormWeapon(float dt, CWorm* worm) = 0;
	virtual void simulateProjectile(float dt, CProjectile* worm, CMap *map, CWorm *worms, int *wormid, int* result) = 0;
	virtual void simulateNinjarope(float dt, CNinjaRope* rope, CMap *map, CVec playerpos, CWorm *worms, int owner) = 0;
	virtual void simulateBonus(float dt, CBonus* bonus, CMap* map) = 0;
};

#endif
