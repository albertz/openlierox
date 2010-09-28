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

struct SDL_Surface;
class CViewport;
class CGameScript;

#include "CVec.h"



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
	AbsTime	fSpawnTime;
	float	fFlashTime;


public:
	// Methods


	void	Spawn(CVec pos, int type, int weapon, CGameScript *gs);
	
	void	Draw(SDL_Surface * bmpDest, CViewport *v, bool showname);


	// variables
	bool	getUsed()		{ return bUsed; }
	void	setUsed(bool _u);
	
	int		getType()		{ return iType; }
	int		getWeapon()		{ return iWeapon; }

	AbsTime	getSpawnTime()	{ return fSpawnTime; }

	CVec	getPosition()	{ return vPos; }

	CVec&	pos()				{ return vPos; }
	CVec&	velocity()			{ return vVelocity; }
	float&	life()				{ return fLife; }
	float&	flashTime()			{ return fFlashTime; }

	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateBonuses()
	// there which simulates all bonuses together
	AbsTime	fLastSimulationTime;

};




#endif  //  __CBONUS_H__
