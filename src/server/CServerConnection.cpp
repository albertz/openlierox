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


#include "LieroX.h"
#include "ProfileSystem.h"
#include "CServerConnection.h"
#include "CBonus.h"
#include "CWorm.h"
#include "Error.h"
#include "Protocol.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "EndianSwap.h"
#include "Version.h"
#include "CServer.h"
#include "AuxLib.h"
#include "Networking.h"
#include "CServerNetEngine.h"
#include "CChannel.h"
#include "Debug.h"






CServerConnection::CServerConnection( GameServer * _server ) {
	server = _server ? _server : cServer;

	cNetChan = NULL;
	bsUnreliable.Clear();
	iNetSpeed = 3;
	fLastUpdateSent = AbsTime();
	bLocalClient = false;

	fSendWait = 0;

	bMuted = false;	
	bGameReady = false;
	m_gusLoggedIn = false;
	
	cShootList.Initialize();

	fLastFileRequest = fConnectTime = tLX->currentTime;
	
	cNetEngine = new CServerNetEngine( server, this );
}

void CServerConnection::resetChannel() {
	if(cNetChan) delete cNetChan;
	cNetChan = NULL;
}

///////////////////
// Clear the client details
void CServerConnection::Clear()
{
	if( cNetChan )
		delete cNetChan;
	cNetChan = NULL;
	iNetStatus = NET_DISCONNECTED;
	bsUnreliable.Clear();
	bLocalClient = false;
	bGameReady = bMuted = m_gusLoggedIn = false;
	
	fLastReceived = AbsTime::Max();
	fSendWait = 0;
	fLastUpdateSent = AbsTime();

	cShootList.Shutdown();
	cShootList.Initialize();
	
	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
	getUdpFileDownloader()->allowFileRequest(true);
}


///////////////////
// Clear the client for another game
void CServerConnection::MinorClear()
{
	iNetStatus = NET_CONNECTED;
	fLastReceived = AbsTime::Max();

	fSendWait = 0;

	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
}

int CServerConnection::getConnectionArrayIndex() {
	if(server == NULL) {
		errors << "CServerConnection::getConnectionArrayIndex: server is not set" << endl;
		return -1;
	}
	
	if(this < &server->getClients()[0] || this >= &server->getClients()[MAX_CLIENTS]) {
		errors << "CServerConnection::getConnectionArrayIndex: outside of the array" << endl;
		return -1;
	}
	
	return this - &server->getClients()[0];
}


///////////////////
// Return true if we own the worm
int CServerConnection::OwnsWorm(int id)
{
	CWorm* w = game.wormById(id, false);
	if(!w) return false;
	return w->getClient() == this;
}


///////////////////
// Shutdown the client
void CServerConnection::Shutdown()
{	
	// Shooting list
	cShootList.Shutdown();

	// Net engine
	if (cNetEngine)
		delete cNetEngine;
	cNetEngine = NULL;

}

void CServerConnection::setClientVersion(const Version& v)
{
	cClientVersion = v;
}

void CServerConnection::resetNetEngine() {
	if(cNetEngine) delete cNetEngine; cNetEngine = NULL;	
}

void CServerConnection::setNetEngineFromClientVersion()
{
	resetNetEngine();
	
	if( getClientVersion() >= OLXBetaVersion(0,58,1) )
		cNetEngine = new CServerNetEngineBeta9( server, this );
	else if( getClientVersion() >= OLXBetaVersion(8) )
		cNetEngine = new CServerNetEngineBeta8( server, this );
	else if( getClientVersion() >= OLXBetaVersion(7) )
		cNetEngine = new CServerNetEngineBeta7( server, this );
	else if( getClientVersion() >= OLXBetaVersion(5) )
		cNetEngine = new CServerNetEngineBeta5( server, this );
	else if( getClientVersion() >= OLXBetaVersion(3) )
		cNetEngine = new CServerNetEngineBeta3( server, this );
	else
		cNetEngine = new CServerNetEngine( server, this );
}

CChannel * CServerConnection::createChannel(const Version& v)
{
	if( cNetChan )
		delete cNetChan;
	if( v >= OLXBetaVersion(0,58,1) )
		cNetChan = new CChannel3();
	else if( v >= OLXBetaVersion(6) )
		cNetChan = new CChannel2();
	else
		cNetChan = new CChannel_056b();
	return cNetChan;
}


std::string CServerConnection::getAddrAsString() {
	std::string addr = "?.?.?.?";
	if(isLocalClient())
		addr = "local";
	else if(!getChannel())  {
		warnings << "CServerConnection::getAddrAsString(): getChannel() == NULL" << endl;
	} else if(!NetAddrToString(getChannel()->getAddress(), addr))  {
		warnings << "CServerConnection::getAddrAsString(): NetAddrToString failed" << endl;
	}
	return addr;
}

std::string CServerConnection::debugName(bool withWorms) {	
	std::string ret = getAddrAsString();
	ret += "(" + cClientVersion.asString() + ")";
	
	if(withWorms) {
		std::string worms = "no worms";
		if(game.wormsOfClient(this)->size() > 0) {
			worms = "";
			bool first = true;
			for_each_iterator(CWorm*, w, game.wormsOfClient(this)) {
				if(!first) worms += ", ";
				worms += itoa(w->get()->getID());
				worms += " '";
				worms += w->get()->getName();
				worms += "'";
				first = false;
			}
		}
		
		ret += "(" + worms + ")";
	}

	return ret;
}

IpInfo CServerConnection::ipInfo() {
	IpInfo info;
	std::string addr;
	if(isLocalClient()) {
		info.countryName = "local";
		info.continent = "local";
		return info;
	}
	else if(!getChannel()) {
		info.countryName = "NO CONNECTION";
		info.continent = "NO CONNECTION";
		return info;
	}
	else if(!NetAddrToString(getChannel()->getAddress(), addr)) {
		info.countryName = "INVALID CONNECTION";
		info.continent = "INVALID CONNECTION";
		return info;
	}
	
	info = tIpToCountryDB->GetInfoAboutIP(addr);
	return info;
}

int CServerConnection::getPing() { return cNetChan->getPing(); }
void CServerConnection::setPing(int _p) { cNetChan->setPing(_p); }


void CServerConnection::logError(const std::string& err) const {
	errors << "CServerConnection::" << err << endl;
}

