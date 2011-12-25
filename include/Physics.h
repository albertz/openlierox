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
#include "Iter.h"
#include "olx-types.h"
#include "CodeAttributes.h"


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
	virtual void simulateWorm(CWorm* worm, CWorm *worms, bool local) = 0;
	virtual void simulateWormWeapon(CWorm* worm) = 0;
	virtual void simulateProjectiles(Iterator<CProjectile*>::Ref projs) = 0;
	virtual void simulateBonuses(CBonus* bonuses, size_t count) = 0;
	
	// skips simulation for one frame (but increments simulationtime)
	void skipWorm(CWorm* worm);
	void skipProjectiles(Iterator<CProjectile*>::Ref projs);
	void skipBonuses(CBonus* bonuses, size_t count);	
};

AbsTime GetPhysicsTime(); // Returns tLX->currentTime, or NewNet::GetCurTime() if new net engine is active


INLINE void warpSimulationTimeForDeltaTimeCap(AbsTime& simulationTime, TimeDiff deltaTime, TimeDiff realDeltaTime) {
	simulationTime += realDeltaTime - deltaTime;
}

INLINE void applyFriction(CVec& vel, float dt, float area, float mass, float dragCoeff, float airCoeff, CVec airVel = CVec(0,0)) {
	const CVec relAirVel = airVel - vel;
	vel += relAirVel * relAirVel.GetLength() * area * dragCoeff * airCoeff * 0.5f * dt / mass;
}


#endif
