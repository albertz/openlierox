/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
	CLL_QUIT,
	CLL_TIMEOUT,
    CLL_KICK,
	CLL_BAN
};


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
	char		sMapFilename[256];
	CMap		*cMap;

	// Simulation
	float		fServertime;
	int			iServerFrame;

	float		fLastBonusTime;

	// Network
	NetworkSocket	tSocket;
	int			nPort;
	challenge_t	tChallenges[MAX_CHALLENGES];
	game_lobby_t tGameLobby;
	CShootList	cShootList;

	CBanList	cBanList;

	bool		bRegServer;	
	int			bServerRegistered;
	float		fLastRegister;

public:
	// Methods


	void		Clear(void);
	int			StartServer(char *name, int port, int maxplayers, bool regserver);
	void		Shutdown(void);	

    void        notifyLog(char *fmt, ...);


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
	int			getState(void)		{ return iState; }
	CWorm		*getWorms(void)		{ return cWorms; }
	game_lobby_t *getLobby(void)	{ return &tGameLobby; }
	CMap		*getMap(void)		{ return cMap; }
	CBanList	*getBanList(void)	{ return &cBanList; }
	CClient		*getClient(int iWormID);
	char		*getName(void)		{ return &sName[0]; }
	void		setName(char *_name){ strncpy(sName,_name,sizeof(sName)); sName[sizeof(sName)-1] = '\0'; }
	int			getMaxWorms(void)	{ return iMaxWorms; }
	void		setMaxWorms(int _n) { iMaxWorms = _n; }
};




#endif  //  __CSERVER_H__
