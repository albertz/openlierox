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
#include "Networking.h"
#include "SmartPointer.h"
#include "CGameScript.h"
#include "CBonus.h"
#include "CShootList.h"
#include "HTTP.h"
#include "Timer.h"
#include "CBanList.h"
#include "CWpnRest.h"
#include "LieroX.h" // for game_lobby_t

class CWorm;
class CServerConnection;
class CMap;
class Version;


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
	GameServer();

	~GameServer();

private:
	// Attributes

	// General
	std::string	sName;
	int			iState;

	// TODO: merge this with game_t (tGameInfo variable)
	// Game rules
	bool		bGameOver;
	float		fGameOverTime;
	int			iGameType;
	int			iLives;
	int			iMaxKills;
	float		fTimeLimit;
	int			iTagLimit;
	int			iMaxWorms;
	bool		bBonusesOn;
	bool		bShowBonusName;
	int			iLoadingTimes;
	std::string	sModName;
	SmartPointer<CGameScript> cGameScript;
	std::string	sWeaponRestFile;
    CWpnRest    cWeaponRestrictions;

	bool		bTournament;

	int			iFlagHolders [MAX_WORMS];	// The ID of which worm holds each flag (an array for if Team CTF is made)
	float		fLastCTFScore;				// The last time someone scored in CTF (for when the map doesn't have a base)

	// Special messages
	bool		bFirstBlood;	// True if no-one has been killed yet

	// Clients
	CServerConnection *cClients;		// TODO: use std::list or vector

	// Worms
	int			iNumPlayers;
	CWorm		*cWorms;		// TODO: use std::list or vector

	// Projectiles
	//CProjectile	*cProjectiles;

	// Bonuses
	CBonus		cBonuses[MAX_BONUSES];  // TODO: use std::list or vector

	// Map
	bool		bRandomMap;		// TODO: what is this good for
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
	NetworkSocket	tNatTraverseSockets[MAX_CLIENTS];
	float			fNatTraverseSocketsLastAccessTime[MAX_CLIENTS];	// So two clients won't fight for one socket
	challenge_t		tChallenges[MAX_CHALLENGES]; // TODO: use std::list or vector
	game_lobby_t	tGameLobby;
	CShootList		cShootList;
	CHttp			tHttp;
	bool			bLocalClientConnected;
	int				iSuicidesInPacket;

	CBanList	cBanList;
	float		fLastUpdateSent;

	bool		bRegServer;
	bool		bServerRegistered;
	float		fLastRegister;
	std::string sCurrentUrl;
	std::list<std::string>::iterator	tCurrentMasterServer;
	std::list<std::string>				tMasterServers;
	float		fLastRegisterUdp;
	std::vector<std::string>			tUdpMasterServers;
	float		fWeaponSelectionTime;
	int			iWeaponSelectionTime_Warning;
	float		fLastRespawnWaveTime;
	
	friend class CServerNetEngine;
	friend class CServerNetEngineBeta7;
	static void SendConnectHereAfterTimeout (Timer::EventData ev);
	
public:
	// Methods


	void		Clear(void);
	int			StartServer(const std::string& name, int port, int maxplayers, bool regserver);
	void		Shutdown(void);

    void        notifyLog(const std::string& msg);

	// Game
	void		Frame();
	int			StartGame();
	void		BeginMatch(CServerConnection* cl = NULL); // if NULL, begin match for everybody; or only for cl
	void		GameOver(int winner);

	void		SpawnWorm(CWorm *Worm, CVec * _pos = NULL);
	void		SpawnWorm(CWorm *Worm, CVec pos, CServerConnection *cl);
	void		SpawnWave();	// Respawn all dead worms at once
	void		SimulateGame(void);
	// TODO: Give this a better name (I couldn't think of what to call it)
	void		SimulateGameSpecial();
	CVec		FindSpot(void);
	void		SpawnBonus(void);
	static void	WormShoot(CWorm *w, GameServer* gameserver);
    void        DemolitionsGameOver(int winner);
    void        RecheckGame(void);

	void		gotoLobby(void);

	void		TagWorm(int id);
	void		TagRandomWorm(void);


	// Network
	void		ReadPackets(void);
	void		SendPackets(void);
	bool		SendUpdate();
	void		SendWeapons(CServerConnection* cl = NULL); // if NULL, send globally, else only to that client
	bool		checkBandwidth(CServerConnection *cl);
	static bool	checkUploadBandwidth(float fCurUploadRate); // used by client/server to check upload
	void		RegisterServer(void);
	void		RegisterServerUdp(void);
	void		ProcessRegister(void);
	void		CheckRegister(void);
	bool		DeRegisterServer(void);
	void		DeRegisterServerUdp(void);
	bool		ProcessDeRegister(void);
	void		CheckTimeouts(void);
	void		CheckWeaponSelectionTime(void);
	void		DropClient(CServerConnection *cl, int reason, const std::string& sReason = "");
	void		RemoveClient(CServerConnection *cl);
	void		RemoveClientWorms(CServerConnection* cl);
	void		kickWorm(int wormID, const std::string& sReason = "");
    void        kickWorm(const std::string& szWormName, const std::string& sReason = "");
	void		banWorm(int wormID, const std::string& sReason = "");
	void		banWorm(const std::string& szWormName, const std::string& sReason = "");
	void		muteWorm(int wormID);
	void		muteWorm(const std::string& szWormName);
	void		unmuteWorm(int wormID);
	void		unmuteWorm(const std::string& szWormName);
	void		authorizeWorm(int wormID);
	void		killWorm(int victimID, int killerID, int suicidesCount = 0); // suicidesCount is ignored if victimID != killerID
    void		cloneWeaponsToAllWorms(CWorm* worm);
	
	void        CheckReadyClient(void);
	float		GetDownload();
	float		GetUpload(float timeRange = 2.0f);

	void		checkVersionCompatibilities(bool dropOut);
	bool		checkVersionCompatibility(CServerConnection* cl, bool dropOut, bool makeMsg = true);
	bool		forceMinVersion(CServerConnection* cl, const Version& ver, const std::string& reason, bool dropOut, bool makeMsg = true);

	// Sending
	void		SendPacket(CBytestream *bs, CServerConnection *cl);
	void		SendGlobalPacket(CBytestream *bs);
	void		SendText(CServerConnection *cl, const std::string& text, int type);
	void		SendGlobalText(const std::string& text, int type);
	void		SendChatCommandCompletionSolution(CServerConnection* cl, const std::string& startStr, const std::string& solution);
	void		SendChatCommandCompletionList(CServerConnection* cl, const std::string& startStr, const std::list<std::string>& solutions);
	void		SendWormsOut(const std::list<byte>& ids);
	void		SendDisconnect();
    void        SendWormLobbyUpdate(CServerConnection* receiver = NULL); // if NULL, to everybody, or only to cl
	void		SendPrepareGame(CServerConnection* cl);
	void		SendClientReady(CServerConnection* receiver, CServerConnection* cl);
	void		UpdateGameLobby(CServerConnection* cl = NULL); // if NULL, to everybody, or only to cl
	void		UpdateWorms();
#ifdef FUZZY_ERROR_TESTING
	void		SendRandomPacket();
#endif
	void		SendFiles();
	void		SendEmptyWeaponsOnRespawn( CWorm * Worm );

	void		ParseConnectionlessPacket(NetworkSocket tSocket, CBytestream *bs, const std::string& ip);
	void		ParseGetChallenge(NetworkSocket tSocket, CBytestream *bs);
	void		ParseConnect(NetworkSocket tSocket, CBytestream *bs);
	void		ParsePing(NetworkSocket tSocket);
	void		ParseTime(NetworkSocket tSocket);
	void		ParseQuery(NetworkSocket tSocket, CBytestream *bs, const std::string& ip);
    void        ParseGetInfo(NetworkSocket tSocket);
	void		ParseWantsJoin(NetworkSocket tSocket, CBytestream *bs, const std::string& ip);
	void		ParseTraverse(NetworkSocket tSocket, CBytestream *bs, const std::string& ip);
	void		ParseServerRegistered(NetworkSocket tSocket);


	// Variables
	int				getState(void)			{ return iState; }
	CWorm			*getWorms(void)			{ return cWorms; }
	game_lobby_t	*getLobby(void)			{ return &tGameLobby; }
	CMap			*getMap(void)			{ return cMap; }
	CBanList		*getBanList(void)		{ return &cBanList; }
	CServerConnection *getClient(int iWormID);
	std::string		getName(void)			{ return sName; }
	void			setName(const std::string& _name){ sName = _name; }
	int				getMaxWorms(void)		{ return iMaxWorms; }
	void			setMaxWorms(int _n)		{ iMaxWorms = _n; }
	bool			getGameOver(void)		{ return bGameOver; }
	float			getGameOverTime(void)	{ return fGameOverTime; }
	CHttp *getHttp()  { return &tHttp; }
	CServerConnection *getClients() { return cClients; }
	float	getServerTime() { return fServertime; }

	// TODO: change the name of these functions; the sense should be clear
	int		getFlagHolder(int _w)			{ return iFlagHolders[_w]; }
	void	setFlagHolder(int _f, int _w)	{ iFlagHolders[_w] = _f; }

	int		getNumPlayers(void)			{ return iNumPlayers; }
	
	bool	serverChoosesWeapons();
	bool	serverAllowsConnectDuringGame();
};

extern	GameServer		*cServer;



#endif  //  __CSERVER_H__
