//
//  Game_loadWeaponRestrictions.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 02.01.12.
//  code under LGPL
//

#include "Game.h"
#include "CWpnRest.h"
#include "Settings.h"
#include "Mod.h"
#include "CGameScript.h"
#include "LieroX.h"

Result Game::loadWeaponRestrictions() {
	m_wpnRest = NULL;

	notes << "Weapon restriction: " << gameSettings[FT_WeaponRest] << endl;
	m_wpnRest = new CWpnRest();

	if(game.isServer())
		m_wpnRest->loadList(gameSettings[FT_WeaponRest], gameSettings[FT_Mod].as<ModInfo>()->path);
	
	if(m_gameMod.get())
		m_wpnRest->updateList(m_gameMod->GetWeaponList());
	else
		errors << "Game::loadWeaponRestrictions: mod not loaded" << endl;
	
	return true;
}
