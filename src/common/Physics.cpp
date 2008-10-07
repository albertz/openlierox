/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#include <iostream>

#include "Physics.h"
#include "PhysicsLX56.h"

using namespace std;

static PhysicsEngine* engine = NULL;
PhysicsEngine* PhysicsEngine::Get() { return engine; }
void PhysicsEngine::Set(PhysicsEngine* e) { engine = e; }

void PhysicsEngine::Init() {
	engine = CreatePhysicsEngineLX56();
	
	cout << "PhysicsEngine " << engine->name() << " loaded" << endl;
}

void PhysicsEngine::UnInit() {
	cout << "unloading PhysicsEngine " << engine->name() << " .." << endl;
	delete engine;
	engine = NULL;
}
