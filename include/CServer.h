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
class CGameMode;

#define		MAX_CHALLENGES		1024


// Challenge structure
class challenge_t { public:
	NetworkAddr	Address;
	AbsTime		fTime;
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
	int			iState;

	// TODO: merge this with game_t (tGameInfo variable)
	// Game rules
	bool		bGameOver;
	AbsTime		fGameOverTime;
	
	SmartPointer<CGameScript> cGameScript;
	std::string	sWeaponRestFile;
    CWpnRest    cWeaponRestrictions;
    
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
	CMap		*cMap;

	// Simulation
	TimeDiff	fServertime;	// TODO: what is this good for
	int			iServerFrame;	// TODO: what is this good for
	int			lastClientSendData;
	
	AbsTime		fLastBonusTime;

	int			iLastVictim;	// TODO: what is this good for

	// Network
	NetworkSocket	tSocket;
	int				nPort;
	NetworkSocket	tNatTraverseSockets[MAX_CLIENTS];
	AbsTime			fNatTraverseSocketsLastAccessTime[MAX_CLIENTS];	// So two clients won't fight for one socket
	challenge_t		tChallenges[MAX_CHALLENGES]; // TODO: use std::list or vector
	CShootList		cShootList;
	CHttp			tHttp;
	bool			bLocalClientConnected;
	int				iSuicidesInPacket;

	CBanList	cBanList;
	AbsTime		fLastUpdateSent;

	bool		bServerRegistered;
	AbsTime		fLastRegister;
	std::string sCurrentUrl;
	std::list<std::string>::iterator	tCurrentMasterServer;
	std::list<std::string>				tMasterServers;
	AbsTime		fLastRegisterUdp;
	std::vector<std::string>			tUdpMasterServers;
	AbsTime		fWeaponSelectionTime;
	int			iWeaponSelectionTime_Warning;
	
	static void SendConnectHereAfterTimeout (Timer::EventData ev);

	friend class CServerNetEngine;
	friend class CServerNetEngineBeta7;
	friend class CServerNetEngineBeta9;
	
public:
	// Methods


	void		Clear(void);
	int			StartServer();
	void		Shutdown(void);

    void        notifyLog(const std::string& msg);

	// Game
	void		Frame();
	int			StartGame();
	void		BeginMatch(CServerConnection* cl = NULL); // if NULL, begin match for everybody; or only for cl
	void		GameOver();

	void		SpawnWorm(CWorm *Worm, CVec * _pos = NULL, CServerConnection * client = NULL);
	void		SimulateGame(void);
	CVec		FindSpot(void);
	void		SpawnBonus(void);
	static void	WormShoot(CWorm *w, GameServer* gameserver);
    void        RecheckGame(void);

	void		gotoLobby(void);


	// Network
	void		ReadPackets(void);
	void		SendPackets(void);

	int			getPort() { return nPort; }
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
	bool		clientsConnected_less(const Version& ver); // true if clients < ver are connected
	
	ScriptVar_t isNonDamProjGoesThroughNeeded(const ScriptVar_t& preset);
	
	// Sending
	void		SendGlobalPacket(CBytestream *bs); // TODO: move this to CServerNetEngine
	void		SendGlobalText(const std::string& text, int type);
	void		SendWormsOut(const std::list<byte>& ids);
	void		SendDisconnect();
    void        SendWormLobbyUpdate(CServerConnection* receiver = NULL, CServerConnection *target = NULL); // if NULL, to everybody, or only to cl. If target is NULL send info about all worms
	void		UpdateGameLobby(CServerConnection* cl = NULL); // if NULL, to everybody, or only to cl
	void		UpdateWorms();
#ifdef FUZZY_ERROR_TESTING
	void		SendRandomPacket();
#endif
	void		SendFiles();
	void		SendEmptyWeaponsOnRespawn( CWorm * Worm );
	bool		SendUpdate();
	void		SendWeapons(CServerConnection* cl = NULL); // if NULL, send globally, else only to that client
	void		SendWormTagged(CWorm *w);

	// Connectionless packets only here
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
	CGameMode		*getGameMode() const	{ return tLXOptions->tGameInfo.gameMode; }
	int				getState()			{ return iState; }
	CWorm			*getWorms()			{ return cWorms; }
	CMap			*getMap()			{ return cMap; }
	void			resetMap()			{ cMap = NULL; }
	CBanList		*getBanList()		{ return &cBanList; }
	CServerConnection *getClient(int iWormID);
	std::string		getName()			{ return tLXOptions->sServerName; }
	void			setName(const std::string& _name){ tLXOptions->sServerName = _name; }
	bool			getGameOver()		{ return bGameOver; }
	AbsTime			getGameOverTime()	{ return fGameOverTime; }
	CHttp *getHttp()  { return &tHttp; }
	CServerConnection *getClients() { return cClients; }
	TimeDiff	getServerTime() { return fServertime; }

	int		getNumPlayers() const		{ return iNumPlayers; }
	int		getFirstEmptyTeam() const; // -1 if there is no empty team; only possible teams by gamemode
	bool	isTeamEmpty(int t) const;
	
	bool	serverChoosesWeapons();
	bool	serverAllowsConnectDuringGame();
};

extern	GameServer		*cServer;



#endif  //  __CSERVER_H__
