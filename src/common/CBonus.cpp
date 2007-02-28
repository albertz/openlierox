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


#include "defs.h"
#include "LieroX.h"


///////////////////
// Spawn the bonus
void CBonus::Spawn(CVec pos, int type, int weapon, CGameScript *gs)
{
	iUsed = true;
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
void CBonus::Draw(SDL_Surface *bmpDest, CViewport *v, int showname)
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


///////////////////
// Simulate the bonus
void CBonus::Simulate(CMap *map, float dt)
{
	int x,  y;
	int mw, mh;
	int px, py;


	fLife += dt;
	if(fLife > BONUS_LIFETIME-3) {
		fFlashTime += dt;

		if(fFlashTime>0.3f)
			fFlashTime=0;
	}

	//
	// Position & Velocity
	//
	vVelocity = vVelocity + CVec(0,80)*dt;
	vPos = vPos + vVelocity*dt;



	//
	// Check if we are hitting the ground
	//


	px = (int)vPos.x;
	py = (int)vPos.y;

	// Initialize
	x = px-2;
	y = py-2;

	
	mw = map->GetWidth();
	mh = map->GetHeight();

	for(y=py-2; y<=py+2; y++) {

		// Clipping
		if(y<0)
			continue;
		if(y>=mh) {
			Collide(x,y);
			return;
		}

		const uchar *pf = map->GetPixelFlags() + y*mw + px-2;

		for(x=px-2; x<=px+2; x++) {

			// Clipping
			if(x<0) {
				pf++;
				continue;
			}
			if(x>=mw) {
				Collide(x,y);
				return;
			}

			if(*pf & PX_DIRT || *pf & PX_ROCK) {
				Collide(x,y);
				return;
			}

			pf++;
		}
	}
}


///////////////////
// Collide with the map
void CBonus::Collide(int x, int y)
{
	vVelocity = CVec(0,0);
	vPos = CVec(vPos.x, (float)y-2);
}
