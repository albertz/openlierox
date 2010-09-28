/*
	OpenLieroX
	
	physic simulation interface
	
	code under LGPL
	created on 9/2/2008
*/

#ifndef __PHYSICSLX56_H__
#define __PHYSICSLX56_H__

#include "Physics.h"

PhysicsEngine* CreatePhysicsEngineLX56();
int getCurrentLX56PhysicsFPS();

#define	LX56PhysicsFixedFPS	getCurrentLX56PhysicsFPS()
#define	LX56PhysicsDT	TimeDiff(1000 / LX56PhysicsFixedFPS)

#endif
