/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Bonus class
// Created 5/8/02
// Jason Boettcher


#ifndef __CBONUS_H__
#define __CBONUS_H__

#include "LieroX.h"
#include "CGameScript.h"


#define		MAX_BONUSES		32
#define		BONUS_SPAWNFREQ	30
#define		BONUS_LIFETIME	60


// Bonus types
#define		BNS_WEAPON		0
#define		BNS_HEALTH		1
#define		BNS_FULLCHARGE	2




class CBonus {
public:
	// Constructor
	CBonus() {
		bUsed = false;
		fLastSimulationTime = -9999;
	}


private:
	// Attributes

	bool	bUsed;
	int		iType;
	int		iWeapon;

	std::string	sWeapon;

	CVec	vPos;
	CVec	vVelocity;
	float	fLife;
	float	fSpawnTime;
	float	fFlashTime;


public:
	// Methods


	void	Spawn(CVec pos, int type, int weapon, CGameScript *gs);
	
	void	Draw(const SmartPointer<SDL_Surface> & bmpDest, CViewport *v, int showname);


	// variables
	bool	getUsed(void)		{ return bUsed; }
	void	setUsed(bool _u)		{ bUsed = _u; if(_u) fLastSimulationTime = tLX->fCurTime; }

	int		getType(void)		{ return iType; }
	int		getWeapon(void)		{ return iWeapon; }

	float	getSpawnTime(void)	{ return fSpawnTime; }

	CVec	getPosition(void)	{ return vPos; }

	CVec&	pos()				{ return vPos; }
	CVec&	velocity()			{ return vVelocity; }
	float&	life()				{ return fLife; }
	float&	flashTime()			{ return fFlashTime; }

	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateBonuses()
	// there which simulates all bonuses together
	float	fLastSimulationTime;

};




#endif  //  __CBONUS_H__
