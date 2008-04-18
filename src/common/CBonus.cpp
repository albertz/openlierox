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


#include "LieroX.h"
#include "CBonus.h"
#include "Graphics.h"
#include "GfxPrimitives.h"


///////////////////
// Spawn the bonus
void CBonus::Spawn(CVec pos, int type, int weapon, CGameScript *gs)
{
	bUsed = true;
	fLastSimulationTime = tLX->fCurTime;
	vPos = pos;
	vVelocity = CVec(0,0);
	iType = type;
	iWeapon = weapon;
	fLife = 0;
	fSpawnTime = tLX->fCurTime;
	fFlashTime = 0;

	if(type == BNS_WEAPON)
		sWeapon = (gs->GetWeapons()+weapon)->Name;
}


///////////////////
// Draw the bonus
void CBonus::Draw(const SmartPointer<SDL_Surface> & bmpDest, CViewport *v, int showname)
{
	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();

	int x=((int)vPos.x-wx)*2+l;
	int y=((int)vPos.y-wy)*2+t;

	// If we are in a flashing mode, don't show it on an 'off' flash mode
	if(fLife > BONUS_LIFETIME-3) {
		if(fFlashTime > 0.15f)
			return;
	}


	switch(iType) {

		// Health
		case BNS_HEALTH:
			DrawImage(bmpDest,gfxGame.bmpHealth,x-5,y-5);
			break;

		// Weapon
		case BNS_WEAPON:
			DrawImage(bmpDest, gfxGame.bmpBonus, x-5,y-5);
			
			if(showname)
				tLX->cOutlineFont.DrawCentre(bmpDest, x, y-20, tLX->clPlayerName, sWeapon);
			break;
	}
}

