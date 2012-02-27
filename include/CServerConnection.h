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

#include <string>
#include "CBytestream.h"
#include "CShootList.h"
#include "Consts.h"
#include "olx-types.h"
#include "Version.h"
#include "FileDownload.h"
#include "IpToCountryDB.h"

class CChannel;
class CWorm;
class GameServer;
class CServerNetEngine;
class GameState;


// Client rights on a server
struct ClientRights {
	ClientRights(): NameChange(false), Kick(false), Ban(false), Mute(false), ChooseLevel(false), ChooseMod(false), StartGame(false), Authorize(false), Override(false), Dedicated(false), Script(false), SetVar(false) {}
	void Everything ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = Dedicated = Script = SetVar = true; }
	void Nothing ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = Override = Dedicated = Script = SetVar = false; }

	bool NameChange;
	bool Kick;
	bool Ban;
	bool Mute;
	bool ChooseLevel;
	bool ChooseMod;
	bool StartGame;
	bool Authorize;
	bool Override;
	bool Dedicated;
	bool Script;
	bool SetVar;
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
	
	bool		bGameReady;
	bool		bMuted;
	bool		m_gusLoggedIn;

	int			iNetSpeed;
	int			iNetStatus;

	AbsTime		fLastReceived;
	CChannel	* cNetChan;
	CBytestream	bsUnreliable;

	CShootList	cShootList;
public:
	GameState*	gameState;
private:

    AbsTime       fZombieTime;
	AbsTime		fLastUpdateSent;

	ClientRights tRights;

	Version		cClientVersion;

	bool		bLocalClient;

	CUdpFileDownloader	cUdpFileDownloader;
	AbsTime		fLastFileRequest;
	AbsTime		fLastFileRequestPacketReceived;
	
	AbsTime		fConnectTime;

public:
	// Methods

	void		Clear();
	void 		MinorClear();
	void		Shutdown();

	// Variables
	CChannel	*getChannel()				{ return cNetChan; }
	void		resetChannel();
	CChannel	*createChannel(const Version& v);
	
	CServerNetEngine * getNetEngine()		{ return cNetEngine; }
	void		resetNetEngine();
	void		setNetEngineFromClientVersion();
	
	int			getStatus()					{ return iNetStatus; }
	void		setStatus(int _s)			{ iNetStatus = _s; }

	bool		isUnset() const	{ return iNetStatus == NET_DISCONNECTED; }
	bool		isUsed() const		{
		if(iNetStatus == NET_DISCONNECTED) return false;
		if(iNetStatus == NET_ZOMBIE) return false;
		if(cNetEngine == NULL) { logError("isUsed: netengine is unset"); return false; }
		if(cNetChan == NULL) { logError("isUsed: netchannel is unset"); return false; }
		return true;
	}
	bool		isConnected() const		{
		if(iNetStatus != NET_CONNECTED) return false;
		if(cNetEngine == NULL) { logError("isConnected: netengine is unset"); return false; }
		if(cNetChan == NULL) { logError("isConnected: netchannel is unset"); return false; }
		return true;
	}
	
	CBytestream	*getUnreliable()			{ return &bsUnreliable; }

	int			OwnsWorm(int id);

	bool		getGameReady()				{ return bGameReady; }
	void		setGameReady(bool _g)		{ bGameReady = _g; }

	AbsTime		getLastReceived()			{ return fLastReceived; }
	void		setLastReceived(const AbsTime& _l)	{ fLastReceived = _l; }

	int			getNetSpeed()				{ return iNetSpeed; }
	void		setNetSpeed(int _n)			{ iNetSpeed = _n; }

	CShootList	*getShootList()				{ return &cShootList; }

    void        setZombieTime(const AbsTime& z)      { fZombieTime = z; }
    AbsTime       getZombieTime()      	   { return fZombieTime; }

    void        setConnectTime(const AbsTime& z)      { fConnectTime = z; }
    AbsTime       getConnectTime()	         { return fConnectTime; }

	bool		getMuted()					{ return bMuted; }
	void		setMuted(bool _m)			{ bMuted = _m; }

	ClientRights *getRights()				{ return &tRights; }

	int	getPing();
	void setPing(int _p);

	const Version& getClientVersion()				{ return cClientVersion; }
	void setClientVersion(const Version& v);

	CUdpFileDownloader * getUdpFileDownloader()	{ return &cUdpFileDownloader; };
	AbsTime		getLastFileRequest()					{ return fLastFileRequest; };
	void		setLastFileRequest( const AbsTime& _f ) 			{ fLastFileRequest = _f; };
	AbsTime		getLastFileRequestPacketReceived()		{ return fLastFileRequestPacketReceived; };
	void		setLastFileRequestPacketReceived( const AbsTime& _f ) { fLastFileRequestPacketReceived = _f; };

	bool		isLocalClient()			{ return bLocalClient; }
	void		setLocalClient(bool _l)	{ bLocalClient = _l; }	
	
	int			getConnectionArrayIndex();

	bool&		gusLoggedIn() { return m_gusLoggedIn; }
	
	IpInfo ipInfo();
	std::string getAddrAsString();
	std::string	debugName(bool withWorms = true);

private:
	void logError(const std::string& err) const;
};

#endif  //  __CSERVER_CONNECTION_H__
