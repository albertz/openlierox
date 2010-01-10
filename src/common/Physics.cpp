/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#include "Physics.h"
#include "PhysicsLX56.h"
#include "PhysicsDummy.h"
#include "Debug.h"
#include "NewNetEngine.h"
#include "LieroX.h"
#include "CWorm.h"
#include "CClient.h"
#include "CProjectile.h"
#include "CBonus.h"
#include "game/Game.h"
#include "CGameScript.h"


static PhysicsEngine* engine = NULL;
PhysicsEngine* PhysicsEngine::Get() { return engine; }
void PhysicsEngine::Set(PhysicsEngine* e) { engine = e; }

void PhysicsEngine::Init() {
	if(game.gameScript()->gusEngineUsed())
		engine = CreatePhysicsEngineDummy("Gusanos physics");
	else
		engine = CreatePhysicsEngineLX56();
	
	notes << "PhysicsEngine " << engine->name() << " loaded" << endl;
	engine->initGame();
}

void PhysicsEngine::UnInit() {
	if(engine) {
		engine->uninitGame();
		notes << "unloading PhysicsEngine " << engine->name() << " .." << endl;
		delete engine;
		engine = NULL;
	}
	else
		errors << "PhysicsEngine::UnInit: physics engine not loaded" << endl;
}

AbsTime GetPhysicsTime() {
	return NewNet::Active() ? NewNet::GetCurTime() : tLX->currentTime;
}



void PhysicsEngine::skipWorm(CWorm* worm) {
	worm->fLastSimulationTime += tLX->fRealDeltaTime;
}

void PhysicsEngine::skipProjectiles(Iterator<CProjectile*>::Ref projs) {
	cClient->fLastSimulationTime += tLX->fRealDeltaTime;
	
	for(Iterator<CProjectile*>::Ref i = projs; i->isValid(); i->next()) {
		CProjectile* p = i->get();
		p->fLastSimulationTime += tLX->fRealDeltaTime;
	}
}

void PhysicsEngine::skipBonuses(CBonus* bonuses, size_t count) {
	if(!cClient->getGameLobby()->bBonusesOn) return;
	CBonus *b = bonuses;
	for(size_t i=0; i < count; i++,b++) {
		if(!b->getUsed()) continue;
		b->fLastSimulationTime += tLX->fRealDeltaTime;
	}	
}
