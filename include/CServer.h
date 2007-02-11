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

#include "CBanList.h"

#ifndef __CSERVER_H__
#define	__CSERVER_H__


#define		MAX_CHALLENGES		1024
// Challenge structure
typedef struct {
	NetworkAddr	Address;
	float		fTime;
	int			iNum;
} challenge_t;

// Server state
enum {
	SVS_LOBBY=0,		// Lobby
	SVS_GAME,			// Game, waiting for players to load
	SVS_PLAYING			// Actually playing
};

// Client leaving reasons
enum {
	CLL_QUIT=0,
	CLL_TIMEOUT,
    CLL_KICK,
	CLL_BAN
};

// Structure for logging worms
typedef struct log_worm_s {
	char		sName[64];
	char		sSkin[256];
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
	char		sIP[32];
} log_worm_t;

// Game log structure
typedef struct game_log_s {
	log_worm_t	*tWorms;
	int			iNumWorms;
	float		fGameStart;
	char		sGameStart[64];
} game_log_t;

class CServer {
public:
	// Constructor
	CServer() {
		Clear();
	}


private:
	// Attributes

	// General
	char		sName[32];	
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
	char		sModName[128];
	CGameScript	cGameScript;
    CWpnRest    cWeaponRestrictions;

	bool		bTournament;

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
	char		sMapFilename[512];
	CMap		*cMap;

	// Simulation
	float		fServertime;
	int			iServerFrame;

	float		fLastBonusTime;

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
	int			StartServer(char *name, int port, int maxplayers, bool regserver);
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
	bool		SendUpdate(CClient *cl);
	bool		checkBandwidth(CClient *cl);
	void		RegisterServer(void);
	void		ProcessRegister(void);
	void		CheckRegister(void);
	bool		DeRegisterServer(void);
	bool		ProcessDeRegister(void);
	void		CheckTimeouts(void);
	void		DropClient(CClient *cl, int reason);
    void        kickWorm(int wormID);
    void        kickWorm(char *szWormName);
	void		banWorm(int wormID);
	void		banWorm(char *szWormName);
	void		muteWorm(int wormID);
	void		muteWorm(char *szWormName);
	void		unmuteWorm(int wormID);
	void		unmuteWorm(char *szWormName);
    void        CheckReadyClient(void);
	void		GetCountryFromIP(char *Address, char *Result);


	// Sending
	void		SendGlobalPacket(CBytestream *bs);
	void		SendGlobalText(char *text, int type);
	void		SendDisconnect(void);
    void        SendWormLobbyUpdate(void);
	void		UpdateGameLobby(void);

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

	void		ParseConnectionlessPacket(CBytestream *bs);
	void		ParseGetChallenge(void);
	void		ParseConnect(CBytestream *bs);
	void		ParsePing(void);
	void		ParseQuery(CBytestream *bs);
    void        ParseGetInfo(void);
	void		ParseWantsJoin(CBytestream *bs);


	// Variables
	inline int			getState(void)		{ return iState; }
	inline CWorm		*getWorms(void)		{ return cWorms; }
	inline game_lobby_t *getLobby(void)	{ return &tGameLobby; }
	inline CMap		*getMap(void)		{ return cMap; }
	inline CBanList	*getBanList(void)	{ return &cBanList; }
	CClient		*getClient(int iWormID);
	inline char		*getName(void)		{ return &sName[0]; }
	inline void	setName(char *_name){ fix_strncpy(sName,_name); }
	inline int	getMaxWorms(void)	{ return iMaxWorms; }
	inline void	setMaxWorms(int _n) { iMaxWorms = _n; }
	inline bool		getGameOver(void)	{ return iGameOver != 0; }
	inline float		getGameOverTime(void) { return fGameOverTime; }
	inline bool		getTakeScreenshot(void)	{ return bTakeScreenshot; }
	inline void		setTakeScreenshot(bool _s) { bTakeScreenshot = _s; }
	inline bool		getScreenshotToken(void) { return bScreenshotToken; }
	inline void		setScreenshotToken(bool _s) { bScreenshotToken = _s; }
};




#endif  //  __CSERVER_H__
