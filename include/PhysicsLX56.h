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

#define	LX56PhysicsFixedFPS	84
#define	LX56PhysicsDT	TimeDiff(1000 / LX56PhysicsFixedFPS)

#endif
