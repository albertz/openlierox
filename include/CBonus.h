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
		iUsed = false;
	}


private:
	// Attributes

	int		iUsed;
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
	
	void	Draw(SDL_Surface *bmpDest, CViewport *v, int showname);
	void	Simulate(CMap *map, float dt);
	void	Collide(int x, int y);



	// variables
	int		getUsed(void)		{ return iUsed; }
	void	setUsed(int _u)		{ iUsed = _u; }

	int		getType(void)		{ return iType; }
	int		getWeapon(void)		{ return iWeapon; }

	float	getSpawnTime(void)	{ return fSpawnTime; }

	CVec	getPosition(void)	{ return vPos; }




};




#endif  //  __CBONUS_H__
