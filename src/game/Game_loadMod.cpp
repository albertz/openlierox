//
//  Game_loadMod.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 02.01.12.
//  code under LGPL
//

#include <SDL.h>
#include "Game.h"
#include "CGameScript.h"
#include "Cache.h"
#include "game/Settings.h"
#include "game/Mod.h"

Result Game::loadMod() {
	m_gameMod = NULL;

	// reset all mod settings - *before* we load the mod because Gusanos mods may already customize it
	modSettings.makeSet(false);
	
	float timer = SDL_GetTicks()/1000.0f;
	
	// for client (tLX->iGameType == GME_JOIN): client->getGameLobby()[FT_Mod].as<ModInfo>()->name
	SmartPointer<CGameScript> cGameScript = cCache.GetMod( gameSettings[FT_Mod].as<ModInfo>()->path );
	if( cGameScript.get() == NULL )
	{
	gameScriptCreate:
		cGameScript = new CGameScript();
		if(cGameScript.get() == NULL) {
			errors << "Game::game.gameMap: cannot allocate gamescript" << endl;
			if(cCache.GetEntryCount() > 0) {
				hints << "current cache size is " << cCache.GetCacheSize() << ", we are clearing it now" << endl;
				cCache.Clear();
				goto gameScriptCreate;
			}
			return "Out of memory while loading mod";
		}
		
		int result = cGameScript->Load( gameSettings[FT_Mod].as<ModInfo>()->path );
		if(result != GSE_OK) {
			errors << "Game::game.gameMap: Could not load the game script \"" << gameSettings[FT_Mod].as<ModInfo>()->path << "\"" << endl;
			return "Could not load the game script \"" + gameSettings[FT_Mod].as<ModInfo>()->path + "\"";
		}
		
		cCache.SaveMod( gameSettings[FT_Mod].as<ModInfo>()->path, cGameScript );
	}
	else
		notes << "used cached version of mod, ";
	notes << "Mod loadtime: " << (float)((SDL_GetTicks()/1000.0f) - timer) << " seconds" << endl;
	
	m_gameMod = cGameScript;
	return true;
}
