//
//  Game_loadMap.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 02.01.12.
//  code under LGPL
//

#include "Game.h"
#include "game/CMap.h"
#include "Cache.h"
#include "Level.h"
#include "Settings.h"
#include "CClient.h" // cClient->getGameLobby()

Result Game::loadMap() {
	m_gameMap = NULL;

	// Create the map
mapCreate:
	m_gameMap = new CMap();
	if(m_gameMap.get() == NULL) {
		errors << "Game::loadMap: Out of memory!" << endl;
		if(cCache.GetEntryCount() > 0) {
			hints << "current cache size is " << cCache.GetCacheSize() << ", we are clearing it now" << endl;
			cCache.Clear();
			goto mapCreate;
		}
		return "Out of memory while loading map";
	}
		
	{
		float timer = SDL_GetTicks()/1000.0f;
		std::string sMapFilename = "levels/" + cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path;
		if(!m_gameMap->Load(sMapFilename)) {
			errors << "Game::loadMap: Could not load the level " << cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path << endl;
			return "Could not load level " + cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path;
		}
		notes << "Map loadtime: " << (float)((SDL_GetTicks()/1000.0f) - timer) << " seconds" << endl;
	}
	
	return true;
}

bool Game::isMapReady() const {
	return m_gameMap.get() && m_gameMap->isLoaded();
}
