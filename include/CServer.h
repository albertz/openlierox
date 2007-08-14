/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class
// Created 28/6/02
// Jason Boettcher


#ifndef __CSERVER_H__
#define	__CSERVER_H__

#include <string>
#include "CWorm.h"
#include "CBanList.h"
#include "CBonus.h"
#include "CClient.h"
#include "CBytestream.h"


#define		MAX_CHALLENGES		1024


// Challenge structure
class challenge_t { public:
	NetworkAddr	Address;
	float		fTime;
	int			iNum;
};

// Server state
enum {
	SVS_LOBBY=0,		// Lobby
	SVS_GAME,			// Game, waiting for players to load
	SVS_PLAYING			// Currently playing
};

// Client leaving reasons
enum {
	CLL_QUIT=0,
	CLL_TIMEOUT,
    CLL_KICK,
	CLL_BAN
};

// Structure for logging worms
class log_worm_t { public:
	std::string	sName;
	std::string	sSkin;
	int			iLives;
	int			iKills;
	int			iID;
	int			iSuicides;
	int			iTeam;
	bool		bTagIT;
	float		fTagTime;
	bool		bLeft;
	int			iLeavingReason;
	float		fTimeLeft;
	int			iType;
	std::string	sIP;
};

// Game log structure
class game_log_t { public:
	log_worm_t	*tWorms;
	int			iNumWorms;
	float		fGameStart;
	std::string	sGameStart;
};

class GameServer {
public:
	// Constructor
	GameServer() {
		Clear();
	}

	~GameServer()  {

	}


private:
	// Attributes

	// General
	std::string	sName;
	int			iState;

	// Logging
	game_log_t	*tGameLog;
	bool		bTakeScreenshot;
	bool		bScreenshotToken;

	// Game rules
	int			iGameOver;
	float		fGameOverTime;
	int			iGameType;
	int			iLives;
	int			iMaxKills;
	int			iTimeLimit;
	int			iTagLimit;
	int			iMaxWorms;
	int			iBonusesOn;
	int			iShowBonusName;
	int			iLoadingTimes;
	std::string	sModName;
	CGameScript	cGameScript;
    CWpnRest    cWeaponRestrictions;

	bool		bTournament;

	int			iFlagHolder [4];
	float		fLastFlagPoint;
	CVec		FlagSpawn[4];
	CVec		TeamSpawn[4];
	CVec		BasePos[4];
	CVec		BaseSize[4];

	// Special messages
	bool		bFirstBlood;

	// Clients
	CClient		*cClients;

	// Worms
	int			iNumPlayers;
	CWorm		*cWorms;

	// Projectiles
	//CProjectile	*cProjectiles;

	// Bonuses
	CBonus		cBonuses[MAX_BONUSES];

	// Map
	int			iRandomMap;
	std::string	sMapFilename;
	CMap		*cMap;

	// Simulation
	float		fServertime;
	int			iServerFrame;

	float		fLastBonusTime;

	int			iLastVictim;

	// Network
	NetworkSocket	tSocket;
	int				nPort;
	challenge_t		tChallenges[MAX_CHALLENGES];
	game_lobby_t	tGameLobby;
	CShootList		cShootList;

	CBanList	cBanList;
	float		fLastUpdateSent;

	bool		bRegServer;	
	int			bServerRegistered;
	float		fLastRegister;

public:
	// Methods


	void		Clear(void);
	int			StartServer(const std::string& name, int port, int maxplayers, bool regserver);
	void		Shutdown(void);	

    void        notifyLog(char *fmt, ...);

	// Logging
	void				ShutdownLog(void);
	log_worm_t			*GetLogWorm(int id);
	bool				WriteLogToFile(FILE *f);



	// Game
	void		Frame(void);
	int			StartGame(void);
	void		BeginMatch(void);

	void		SpawnWorm(CWorm *Worm);
	void		SpawnWorm(CWorm *Worm, CVec pos, CClient *cl);
	void		SimulateGame(void);
	CVec		FindSpot(void);
	void		SpawnBonus(void);
	void		WormShoot(CWorm *w);
	void		ShootBeam(CWorm *w);
    void        DemolitionsGameOver(int winner);
    void        RecheckGame(void);

	void		gotoLobby(void);

	void		TagWorm(int id);
	void		TagRandomWorm(void);


	// Network
	void		ReadPackets(void);
	void		SendPackets(void);
	bool		SendUpdate();
	bool		checkBandwidth(CClient *cl);
	void		RegisterServer(void);
	void		ProcessRegister(void);
	void		CheckRegister(void);
	bool		DeRegisterServer(void);
	bool		ProcessDeRegister(void);
	void		CheckTimeouts(void);
	void		DropClient(CClient *cl, int reason);
    void        kickWorm(int wormID);
    void        kickWorm(const std::string& szWormName);
	void		banWorm(int wormID);
	void		banWorm(const std::string& szWormName);
	void		muteWorm(int wormID);
	void		muteWorm(const std::string& szWormName);
	void		unmuteWorm(int wormID);
	void		unmuteWorm(const std::string& szWormName);
    void        CheckReadyClient(void);
	float		GetDownload();
	float		GetUpload();
	bool		ParseChatCommand(const std::string& message, CClient *cl);

	// Sending
	void		SendPacket(CBytestream *bs, CClient *cl);
	void		SendGlobalPacket(CBytestream *bs);
	void		SendText(CClient *cl, const std::string& text, int type);
	void		SendGlobalText(const std::string& text, int type);
	void		SendDisconnect(void);
    void        SendWormLobbyUpdate(void);
	void		UpdateGameLobby(void);
	void		UpdateWorms(void);
#ifdef DEBUG
	void		SendRandomPacket();
#endif

	// Parsing
	void		ParseClientPacket(CClient *cl, CBytestream *bs);
	void		ParsePacket(CClient *cl, CBytestream *bs);
	void		ParseImReady(CClient *cl, CBytestream *bs);
	void		ParseUpdate(CClient *cl, CBytestream *bs);
	void		ParseDeathPacket(CClient *cl, CBytestream *bs);
	void		ParseChatText(CClient *cl, CBytestream *bs);
	void		ParseUpdateLobby(CClient *cl, CBytestream *bs);
	void		ParseDisconnect(CClient *cl);
	void		ParseWeaponList(CClient *cl, CBytestream *bs);
	void		ParseGrabBonus(CClient *cl, CBytestream *bs);

	void		ParseConnectionlessPacket(CBytestream *bs, const std::string& ip);
	void		ParseGetChallenge(void);
	void		ParseConnect(CBytestream *bs);
	void		ParsePing(void);
	void		ParseQuery(CBytestream *bs);
    void        ParseGetInfo(void);
	void		ParseWantsJoin(CBytestream *bs, const std::string& ip);


	// Variables
	inline int			getState(void)		{ return iState; }
	inline CWorm		*getWorms(void)		{ return cWorms; }
	inline game_lobby_t *getLobby(void)	{ return &tGameLobby; }
	inline CMap		*getMap(void)		{ return cMap; }
	inline CBanList	*getBanList(void)	{ return &cBanList; }
	CClient		*getClient(int iWormID);
	inline std::string   getName(void)		{ return sName; }
	inline void	setName(const std::string& _name){ sName = _name; }
	inline int	getMaxWorms(void)	{ return iMaxWorms; }
	inline void	setMaxWorms(int _n) { iMaxWorms = _n; }
	inline bool		getGameOver(void)	{ return iGameOver != 0; }
	inline float		getGameOverTime(void) { return fGameOverTime; }
	inline bool		getTakeScreenshot(void)	{ return bTakeScreenshot; }
	inline void		setTakeScreenshot(bool _s) { bTakeScreenshot = _s; }
	inline bool		getScreenshotToken(void) { return bScreenshotToken; }
	inline void		setScreenshotToken(bool _s) { bScreenshotToken = _s; }

	inline int		getFlag(int team=0)			{ return iFlagHolder[team]; }
	inline void		setFlag(int _f, int team=0)	{ iFlagHolder[team] = _f; }
	inline void		setFlagSpawn(int i, CVec _v){ FlagSpawn[i] = _v; }
	inline void		setTeamSpawn(int i, CVec _v){ TeamSpawn[i] = _v; }
	inline void		setBasePos(int i, CVec _v)	{ BasePos[i] = _v; }
	inline void		setBaseSize(int i, CVec _v)	{ BaseSize[i] = _v; }
	inline CVec		getFlagSpawn(int i)			{ return FlagSpawn[i]; }
	inline CVec		getTeamSpawn(int i)			{ return TeamSpawn[i]; }
	inline CVec		getBasePos(int i)			{ return BasePos[i]; }
	inline CVec		getBaseSize(int i)			{ return BaseSize[i]; }
};

extern	GameServer		*cServer;



#endif  //  __CSERVER_H__
