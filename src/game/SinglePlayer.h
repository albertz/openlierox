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
	SinglePlayerGame() : standardGameMode(NULL) {}
	
	PIVar(bool,false) currentGameValid;
	std::string currentGame;
	PIVar(int,1) currentLevel;
	PIVar(bool,false) levelSucceeded;
	
	std::string description;
	SmartPointer<SDL_Surface> image;
	LevelInfo levelInfo;
	ModInfo modInfo;
	CGameMode* standardGameMode;
	
	void setGame(const std::string& game);
	bool setLevel(int level);
	bool startGame();
	
	void setLevelSucceeded();
	
	int maxSelectableLevelForCurrentGame();

	// CGameMode virtual callbacks
	std::string Name() { return "Game: " + currentGame; }
	void Simulate();
	bool CheckGameOver();
	int Winner();
	void GameOver();
	
#define CGWRAPPER(rettype, name, params, pvalue) rettype name params { if(standardGameMode) return standardGameMode->name pvalue; return CGameMode::name pvalue; }
#define CGWRAPPER_void(name, params, pvalue) void name params { if(standardGameMode) standardGameMode->name pvalue; else CGameMode::name pvalue; }
#define CGWRAPPER0(rettype, name) CGWRAPPER(rettype, name, (), ())
#define CGWRAPPER1(rettype, name, p1) CGWRAPPER(rettype, name, (p1 _p1), (_p1))
#define CGWRAPPER2(rettype, name, p1, p2) CGWRAPPER(rettype, name, (p1 _p1, p2 _p2), (_p1, _p2))
#define CGWRAPPER0v(name) CGWRAPPER_void(name, (), ())
#define CGWRAPPER1v(name, p1) CGWRAPPER_void(name, (p1 _p1), (_p1))
#define CGWRAPPER2v(name, p1, p2) CGWRAPPER_void(name, (p1 _p1, p2 _p2), (_p1, _p2))
	
	CGWRAPPER0(int, GeneralGameType);
	CGWRAPPER0(Version, MinNeededVersion);
	CGWRAPPER0(int, GameTeams);
	CGWRAPPER1(std::string, TeamName, int);
	CGWRAPPER0v(PrepareGame);
	CGWRAPPER0v(BeginMatch);
	CGWRAPPER1v(PrepareWorm, CWorm*);
	CGWRAPPER2(bool, Spawn, CWorm*, CVec&);
	CGWRAPPER2v(Kill, CWorm*, CWorm*);
	CGWRAPPER1(bool, Shoot, CWorm*);
	CGWRAPPER2v(hitFlag, CWorm*, Flag*);
	CGWRAPPER2v(hitFlagSpawnPoint, CWorm*, Flag*);
	CGWRAPPER1v(Drop, CWorm*);
	CGWRAPPER2v(Carve, CWorm*, int);
	CGWRAPPER2(bool, NeedUpdate, CServerConnection*, CWorm*);
	CGWRAPPER2(int, CompareWormsScore, CWorm*, CWorm*);
	CGWRAPPER2(int, CompareTeamsScore, int, int);
	CGWRAPPER0(int, WinnerTeam);
	CGWRAPPER1(int, TeamScores, int);
	CGWRAPPER0(GameInfoGroup, getGameInfoGroupInOptions);
	CGWRAPPER0(float, TimeLimit);
	CGWRAPPER0(float, FlagPointRadius);
	CGWRAPPER0(float, FlagRadius);
	
#undef CGWRAPPER
#undef CGWRAPPER_void
#undef CGWRAPPER0
#undef CGWRAPPER1
#undef CGWRAPPER2
#undef CGWRAPPER0v
#undef CGWRAPPER1v
#undef CGWRAPPER2v
};

extern SinglePlayerGame singlePlayerGame;

#endif

