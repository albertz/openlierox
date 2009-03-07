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
#include "Iterator.h"


class CClient;
class CWorm;
class CProjectile;
class CBonus;
class CNinjaRope;
class CMap;

// HINT: I call this class PhysicsEngine, though it doesn't matches the meaning
// of engine completely as I intented it for Hirudo. But it can probably easily be
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

	virtual void initGame() = 0; // init a new game
	virtual void uninitGame() = 0; // gives just a hint to the engine that the game isn't runnign anymore
	virtual bool isInitialised() = 0; // tells if the engine is inited

	// TODO: later, we should have a class World and all objects and the map are included there
	// in the end, I want to have one single simulate(CWorld* world);
	virtual void simulateWorm(CWorm* worm, CWorm *worms, bool local, float simulationTime) = 0;
	virtual void simulateWormWeapon(float dt, CWorm* worm) = 0;
	virtual void simulateProjectiles(Iterator<CProjectile*>::Ref projs, float fCurTime) = 0;
	virtual void simulateBonuses(CBonus* bonuses, size_t count) = 0;
};

#endif
