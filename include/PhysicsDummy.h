/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#ifndef __PHYSICSDUMMY_H__
#define __PHYSICSDUMMY_H__

#include "Physics.h"
#include <string>

PhysicsEngine* CreatePhysicsEngineDummy(const std::string& enginename = "dummy physics engine");

#endif
