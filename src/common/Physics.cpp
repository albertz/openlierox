/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#include "Physics.h"
#include "PhysicsLX56.h"
#include "Debug.h"
#include "NewNetEngine.h"



static PhysicsEngine* engine = NULL;
PhysicsEngine* PhysicsEngine::Get() { return engine; }
void PhysicsEngine::Set(PhysicsEngine* e) { engine = e; }

void PhysicsEngine::Init() {
	engine = CreatePhysicsEngineLX56();
	
	notes << "PhysicsEngine " << engine->name() << " loaded" << endl;
}

void PhysicsEngine::UnInit() {
	notes << "unloading PhysicsEngine " << engine->name() << " .." << endl;
	delete engine;
	engine = NULL;
}

AbsTime GetPhysicsTime()
{
	return NewNet::GetCurTime();
}
