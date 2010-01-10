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

/*
The dummy physics engine does nothing itself.

It is used right now for Gusanos because we call the gusanos logic frame handler
independently from the physics interface in each frame (because the gusanos logic
frame handler may also do other things independently (scripted maps), so we need
it in any case, even with LX56 physics).

Thus, gusanos don't need extra handling from the physics interface.
*/

PhysicsEngine* CreatePhysicsEngineDummy(const std::string& enginename = "dummy physics engine");

#endif
