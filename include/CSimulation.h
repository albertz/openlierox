/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Simulation class
// Created 28/6/02
// Jason Boettcher


#ifndef __CSIMULATION_H__
#define __CSIMULATION_H__


class CSimulation {
public:
	// Constructor
	CSimulation() {

	}


private:
	// Attributes

	CVec		vPosition;
	CVec		vVelocity;
	

public:
	// Methods

	void		Setup(CVec pos, CVec vel)			{ vPosition=pos; vVelocity=vel; }

	void		Simulate(void);


	// Variables
	void		setPos(CVec v)						{ vPosition = v; }
	CVec		getPos(void)						{ return vPosition; }
};




#endif  //  __CSIMULATION_H__
