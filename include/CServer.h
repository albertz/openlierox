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
#include "HTTP.h"
#include "FileDownload.h"

#define		MAX_CHALLENGES		1024


// Challenge structure
class challenge_t { public:
	NetworkAddr	Address;
	float		fTime;
	int			iNum;
	std::string	sClientVersion;
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

	int			iFlagHolders [MAX_WORMS];	// The ID of which worm holds each flag (an array for if Team CTF is made)
	float		fLastCTFScore;				// The last time someone scored in CTF (for when the map doesn't have a base)

	// Special messages
	bool		bFirstBlood;	// True if no-one has been killed yet

	// Clients
	CClient		*cClients;		// TODO: use std::list or vector

	// Worms
	int			iNumPlayers;
	CWorm		*cWorms;		// TODO: use std::list or vector

	// Projectiles
	//CProjectile	*cProjectiles;

	// Bonuses
	CBonus		cBonuses[MAX_BONUSES];  // TODO: use std::list or vector

	// Map
	int			iRandomMap;		// TODO: what is this good for
	std::string	sMapFilename;
	CMap		*cMap;

	// Simulation
	float		fServertime;	// TODO: what is this good for
	int			iServerFrame;	// TODO: what is this good for

	float		fLastBonusTime;

	int			iLastVictim;	// TODO: what is this good for

	// Network
	NetworkSocket	tSocket;
	int				nPort;
	NetworkAddr		tSTUNAddress;
	challenge_t		tChallenges[MAX_CHALLENGES]; // TODO: use std::list or vector
	game_lobby_t	tGameLobby;
	CShootList		cShootList;
	CHttp			tHttp;

	CBanList	cBanList;
	float		fLastUpdateSent;

	bool		bRegServer;	
	int			bServerRegistered;
	float		fLastRegister;
	std::string sCurrentUrl;
	std::list<std::string>::iterator	tCurrentMasterServer;
	std::list<std::string>				tMasterServers;
	bool		bDedicated;

	
public:
	// Methods


	void		Clear(void);
	int			StartServer(const std::string& name, int port, int maxplayers, bool regserver);
	void		Shutdown(void);	

    void        notifyLog(char *fmt, ...);

	// Game
	void		Frame();
	int			StartGame( bool dedicated = false ); // In dedicated server don't spawn the first worm - it's local player
	void		BeginMatch();
	void		GameOver(int winner);

	void		SpawnWorm(CWorm *Worm);
	void		SpawnWorm(CWorm *Worm, CVec pos, CClient *cl);
	void		SimulateGame(void);
	// TODO: Give this a better name (I couldn't think of what to call it)
	void		SimulateGameSpecial();
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
	void		DropClient(CClient *cl, int reason, const std::string& sReason = "");
	void		kickWorm(int wormID, const std::string& sReason = "");
    void        kickWorm(const std::string& szWormName, const std::string& sReason = "");
	void		banWorm(int wormID, const std::string& sReason = "");
	void		banWorm(const std::string& szWormName, const std::string& sReason = "");
	void		muteWorm(int wormID);
	void		muteWorm(const std::string& szWormName);
	void		unmuteWorm(int wormID);
	void		unmuteWorm(const std::string& szWormName);
    void        CheckReadyClient(void);
	float		GetDownload();
	float		GetUpload();
	bool		ParseChatCommand(const std::string& message, CClient *cl);
	bool		CreateFakeZombieWormsToAllowConnectDuringGame( CBytestream *bs );
	bool		DropFakeZombieWormsToCleanUpLobby( CBytestream *bs );

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
	void		SendDirtUpdate( CClient * cl );
	void		SendFiles();

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
	void		ParseSendFile(CClient *cl, CBytestream *bs);

	void		ParseConnectionlessPacket(CBytestream *bs, const std::string& ip);
	void		ParseGetChallenge(CBytestream *bs);
	void		ParseConnect(CBytestream *bs);
	void		ParsePing(void);
	void		ParseQuery(CBytestream *bs, const std::string& ip);
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
	inline CHttp *getHttp()  { return &tHttp; }
	CClient *getClients() { return cClients; }
	
	// TODO: change the name of these functions; the sense should be clear
	inline int		getFlagHolder(int _w)			{ return iFlagHolders[_w]; }
	inline void		setFlagHolder(int _f, int _w)	{ iFlagHolders[_w] = _f; }

	inline int		getNumPlayers(void)			{ return iNumPlayers; }
	inline bool		getDedicated(void)			{ return bDedicated; }
};

extern	GameServer		*cServer;



#endif  //  __CSERVER_H__
