/*
 OpenLieroX
 
 gamemode interface
 
 created 2009-02-09
 code under LGPL
 */

#ifndef __CGAMEMODE_H__
#define __CGAMEMODE_H__

#include <string>
#include "CVec.h"
#include "Iter.h"
#include "Consts.h"
#include "Options.h"
#include "Version.h"

class CWorm;
class GameServer;
class CServerConnection;
struct Flag;

class CGameMode {
public:
	CGameMode() {}
	virtual ~CGameMode() {}

	// This is the game mode name as shown in the lobby for clients
	virtual std::string Name() = 0;
	virtual int  GeneralGameType(); // this is the game type which is sent over network
	virtual Version MinNeededVersion() { return Version(); }
	
	virtual int  GameTeams(); // amount of teams in gamemode
	bool isTeamGame() { return GameTeams() > 1; }
	virtual std::string TeamName(int t);
	
	virtual void PrepareGame();
	virtual void BeginMatch() {}
	virtual void PrepareWorm(CWorm* worm) {}
	// If Spawn returns false then no spawn packet will be sent
	virtual bool Spawn(CWorm* worm, CVec& pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	// If Shoot returns false then no shot will be fired (used both server&client side)
	virtual bool Shoot(CWorm* worm) { return true; }
	virtual void hitFlag(CWorm* worm, Flag* flag) {}
	virtual void hitFlagSpawnPoint(CWorm* worm, Flag* flag) {}
	virtual void Drop(CWorm* worm);
	virtual void Carve(CWorm* worm, int d) {}
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm) { return true; }
	virtual void GameOver() {}
	
	virtual int CompareWormsScore(CWorm* w1, CWorm* w2);
	virtual int CompareTeamsScore(int t1, int t2);
	virtual int Winner();
	virtual int WinnerTeam();
	virtual int TeamScores(int t);

	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_GameModeSpecific_Start; };

	virtual float TimeLimit();
	virtual float FlagPointRadius() { return 6.0f; }
	virtual float FlagRadius() { return 6.0f; }
	
	virtual bool HaveTargetPos(CWorm* w) { return false; }
	virtual CVec TargetPos(CWorm* w) { return CVec(); }

	// helper functions
	int WormsAliveInTeam(int t);
	int TeamKills(int t);
	float TeamDamage(int t);
	int HighestScoredWorm();
	int HighestScoredTeam();
	
protected:
	bool bFirstBlood;
	int	iKillsInRow[MAX_WORMS];
	int	iDeathsInRow[MAX_WORMS];
	int lastTimeLimitReport;
	int lastFragsLeftReport;
};

void InitGameModes();
CGameMode* GameMode(GameModeIndex i); // GM_* is parameter
CGameMode* GameMode(const std::string& name);
GameModeIndex GetGameModeIndex(CGameMode* gameMode, GameModeIndex fallback = GM_DEATHMATCH);
Iterator<CGameMode*>::Ref GameModeIterator();
std::string guessGeneralGameTypeName(int iGeneralGameType);

#endif

