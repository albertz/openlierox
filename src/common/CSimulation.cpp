/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
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
