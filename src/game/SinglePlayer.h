/*
 *  SinglePlayer.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.01.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_SINGLEPLAYER_H__
#define __OLX_SINGLEPLAYER_H__

#include "SmartPointer.h"
#include "PreInitVar.h"
#include "game/Mod.h"
#include "game/Level.h"
#include "CGameMode.h"
#include <string>

struct SDL_Surface;

struct SinglePlayerGame : CGameMode {
	PIVar(bool,false) currentGameValid;
	std::string currentGame;
	PIVar(int,1) currentLevel;
	PIVar(bool,false) levelSucceeded;
	
	std::string description;
	SmartPointer<SDL_Surface> image;
	LevelInfo levelInfo;
	ModInfo modInfo;
	
	void setGame(const std::string& game);
	bool setLevel(int level);
	bool startGame();
	
	void setLevelSucceeded();
	
	int maxAllowedLevelForCurrentGame();

	// CGameMode virtual callbacks
	std::string Name() { return "Game: " + currentGame; }
	void Simulate();
	bool CheckGameOver();

};

extern SinglePlayerGame singlePlayerGame;

#endif

