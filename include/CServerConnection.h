/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class
// Created 28/6/02
// Jason Boettcher


#ifndef __CSERVER_CONNECTION_H__
#define __CSERVER_CONNECTION_H__

// Server representation of CClient

#include "CChannel.h"
#include "CShootList.h"
#include "Consts.h"
#include "Version.h"
#include "FileDownload.h"

class CWorm;
class GameServer;
class CServerNetEngine;

// Client rights on a server
class ClientRights { public:
	ClientRights(): NameChange(false), Kick(false), Ban(false), Mute(false), ChooseLevel(false), ChooseMod(false), StartGame(false), Authorize(false), Override(false) {}
	void Everything ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = true; }
	void Nothing ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = Override = false; }

	bool NameChange;
	bool Kick;
	bool Ban;
	bool Mute;
	bool ChooseLevel;
	bool ChooseMod;
	bool StartGame;
	bool Authorize;
	bool Override;
};

class CServerConnection {
public:
	// Constructor
	CServerConnection( GameServer * _server = NULL );
	
	~CServerConnection()  {
		Shutdown();
		Clear();
	}

private:
	// Attributes
	GameServer 	*server;
	CServerNetEngine *cNetEngine;
	
	// Local Worms (pointers to specific CServer::cWorms)
	uint		iNumWorms;
	CWorm		*cLocalWorms[MAX_PLAYERS];

	// Remote worms
	CWorm		*cRemoteWorms;
	
	bool		bGameReady;
	bool		bMuted;

	int			iNetSpeed;
	int			iNetStatus;

	float		fLastReceived;
	NetworkSocket	tSocket;
	CChannel	* cNetChan;
	CBytestream	bsUnreliable;

	CShootList	cShootList;

    float       fZombieTime;
	float		fSendWait;
	float		fLastUpdateSent;

	ClientRights tRights;

	Version		cClientVersion;

	bool		bLocalClient;

	CUdpFileDownloader	cUdpFileDownloader;
	float		fLastFileRequest;
	float		fLastFileRequestPacketReceived;
	
	float		fConnectTime;

public:
	// Methods

	void		Clear();
	void 		MinorClear();
	int			Initialize();
	void		Shutdown();

	void		RemoveWorm(int id);

	// Variables
	CChannel	*getChannel()				{ return cNetChan; }
	CChannel	*createChannel(const Version& v);
	
	CServerNetEngine * getNetEngine()		{ return cNetEngine; }
	void		setNetEngineFromClientVersion();
	
	int			getStatus()					{ return iNetStatus; }
	void		setStatus(int _s)			{ iNetStatus = _s; }
	CBytestream	*getUnreliable()			{ return &bsUnreliable; }

	int			OwnsWorm(int id);
	int			getNumWorms()				{ return iNumWorms; }
	void		setNumWorms(int _w)			{ iNumWorms = _w; }

	CWorm		*getWorm(int w)				{ return cLocalWorms[w]; }
	void		setWorm(int i, CWorm *w)	{ cLocalWorms[i] = w; }

	CWorm		*getRemoteWorms()			{ return cRemoteWorms; }
	bool		getGameReady()				{ return bGameReady; }
	void		setGameReady(bool _g)		{ bGameReady = _g; }

	float		getLastReceived()			{ return fLastReceived; }
	void		setLastReceived(float _l)	{ fLastReceived = _l; }

	int			getNetSpeed()				{ return iNetSpeed; }
	void		setNetSpeed(int _n)			{ iNetSpeed = _n; }

	CShootList	*getShootList()				{ return &cShootList; }

    void        setZombieTime(float z)      { fZombieTime = z; }
    float       getZombieTime()      	   { return fZombieTime; }

    void        setConnectTime(float z)      { fConnectTime = z; }
    float       getConnectTime()	         { return fConnectTime; }

	bool		getMuted()					{ return bMuted; }
	void		setMuted(bool _m)			{ bMuted = _m; }

	ClientRights *getRights()				{ return &tRights; }

	int	getPing();
	void setPing(int _p);

	const Version& getClientVersion()				{ return cClientVersion; }
	void setClientVersion(const Version& v);

	CUdpFileDownloader * getUdpFileDownloader()	{ return &cUdpFileDownloader; };
	float		getLastFileRequest()					{ return fLastFileRequest; };
	void		setLastFileRequest( float _f ) 			{ fLastFileRequest = _f; };
	float		getLastFileRequestPacketReceived()		{ return fLastFileRequestPacketReceived; };
	void		setLastFileRequestPacketReceived( float _f ) { fLastFileRequestPacketReceived = _f; };

	bool		isLocalClient()			{ return bLocalClient; }
	void		setLocalClient(bool _l)	{ bLocalClient = _l; }
	
	std::string	debugName();
};

#endif  //  __CSERVER_CONNECTION_H__
