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
#include "DeprecatedGUI/Graphics.h"
#include "GfxPrimitives.h"
#include "WeaponDesc.h"
#include "CViewport.h"
#include "CGameScript.h"
#include "CClient.h"
#include "CMap.h"
#include "game/Game.h"


///////////////////
// Spawn the bonus
void CBonus::Spawn(CVec ppos, int type, int weapon, CGameScript *gs)
{
	bUsed = true;
	fLastSimulationTime = tLX->currentTime;
	vPos = ppos;
	vVelocity = CVec(0,0);
	iType = type;
	iWeapon = weapon;
	fLife = 0;
	fSpawnTime = tLX->currentTime;
	fFlashTime = 0;

	if(type == BNS_WEAPON)
		sWeapon = (gs->GetWeapons()+weapon)->Name;
}


///////////////////
// Draw the bonus
void CBonus::Draw(SDL_Surface * bmpDest, CViewport *v, bool showname)
{
	CMap* map = game.gameMap();
	VectorD2<int> p = v->physicToReal(vPos, cClient->getGameLobby()[FT_InfiniteMap], map->GetWidth(), map->GetHeight());

	// If we are in a flashing mode, don't show it on an 'off' flash mode
	if(fLife > BONUS_LIFETIME-3) {
		if(fFlashTime > 0.15f)
			return;
	}


	switch(iType) {

		// Health
		case BNS_HEALTH:
			DrawImage(bmpDest, DeprecatedGUI::gfxGame.bmpHealth,p.x-5,p.y-5);
			break;

		// Weapon
		case BNS_WEAPON:
			DrawImage(bmpDest, DeprecatedGUI::gfxGame.bmpBonus, p.x-5,p.y-5);
			
			if(showname)
				tLX->cOutlineFont.DrawCentre(bmpDest, p.x, p.y-20, tLX->clPlayerName, sWeapon);
			break;
	}
}

void CBonus::setUsed(bool _u) {
	bUsed = _u; if(_u) fLastSimulationTime = tLX->currentTime;
}

