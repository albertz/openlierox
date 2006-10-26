/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Simulation class
// Created 21/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"




///////////////////
// Simulate
void CSimulation::Simulate(void)
{
	float dt = tLX->fDeltaTime;

	vVelocity = vVelocity + vGravity;
	vPosition = vPosition + vVelocity * dt;
}
