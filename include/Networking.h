/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Common networking routines to help us
// Created 18/12/02
// Jason Boettcher


#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include <string>
#include <cassert>
#include <SDL.h>
#include "olx-types.h"
#include "InternDataClass.h"
#include "Event.h"
#include "util/Result.h"

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


#define		LX_SVRLIST		"/svr_list.php"
#define		LX_SVRREG		"/svr_register.php"
#define		LX_SVRDEREG		"/svr_deregister.php"


/* Shplorbs host, using freesql
#define		LX_SITE			"shplorb.ods.org"
#define		LX_SVRLIST		"/~jackal/svr_list.php"
#define		LX_SVRREG		"/~jackal/svr_register.php"
#define		LX_SVRDEREG		"/~jackal/svr_deregister.php"
*/


/* Old host
#define		LX_SITE			"host.deluxnetwork.com"
#define		LX_SVRLIST		"/~lierox/gamefiles/svr_list.php"
#define		LX_SVRREG		"/~lierox/gamefiles/svr_register.php"
#define		LX_SVRDEREG		"/~lierox/gamefiles/svr_deregister.php"
*/

#define		LX_SVTIMEOUT	35
#define		LX_CLTIMEOUT	30
#define     DNS_TIMEOUT		10

// socket address; this type will be given around as pointer
DEFINE_INTERNDATA_CLASS(NetworkAddr);

bool	IsNetAddrValid(const NetworkAddr& addr);
bool	IsNetAddrAvailable(const NetworkAddr& addr);
bool	SetNetAddrValid(NetworkAddr& addr, bool valid);
void	ResetNetAddr(NetworkAddr& addr);
Result	StringToNetAddr(const std::string& string, NetworkAddr& addr, std::string* errorStr = NULL);
NetworkAddr StringToNetAddr(const std::string& string);
Result	NetAddrToString(const NetworkAddr& addr, std::string& string);
std::string NetAddrToString(const NetworkAddr& addr);
unsigned short GetNetAddrPort(const NetworkAddr& addr);
Result	SetNetAddrPort(NetworkAddr& addr, unsigned short port, std::string* errorStr = NULL);
bool	AreNetAddrEqual(const NetworkAddr& addr1, const NetworkAddr& addr2);
bool	GetNetAddrFromNameAsync(const std::string& name, NetworkAddr& addr);
void	AddToDnsCache(const std::string& name, const NetworkAddr& addr, TimeDiff expireTime = TimeDiff(600.0f));
bool	GetFromDnsCache(const std::string& name, NetworkAddr& addr);



// general networking
bool	InitNetworkSystem();
bool	QuitNetworkSystem();


class NetworkSocket {
public:
	enum Type { NST_INVALID, NST_TCP, NST_UDP, NST_UDPBROADCAST };
	enum State { NSS_NONE, NSS_CONNECTED, NSS_LISTENING };
	typedef unsigned short Port;
	
	static std::string TypeStr(Type t) {
		switch(t) {
			case NST_INVALID: return "Invalid";
			case NST_TCP: return "TCP";
			case NST_UDP: return "UDP";
			case NST_UDPBROADCAST: return "UDP Broadcast";
		}
		return "BADTYPE";
	}
	
	static std::string StateStr(State s) {
		switch(s) {
			case NSS_NONE: return "None";
			case NSS_CONNECTED: return "Connected";
			case NSS_LISTENING: return "Listening";
		}
		return "BADSTATE";
	}
	
private:
	Type m_type;
	State m_state;
	bool m_withEvents;
	struct InternSocket; InternSocket* m_socket;
	friend struct InternSocket;
	struct EventHandler; friend struct EventHandler;
	void checkEventHandling();
	
	// Don't copy instances of this class! Use SmartPointer if you want to have multiple references to a socket.
	// You can swap two NetworkSockets though.
	NetworkSocket(const NetworkSocket&) { assert(false); }
	NetworkSocket& operator=(const NetworkSocket&) { assert(false); return *this; }	
public:
	NetworkSocket(); ~NetworkSocket();
	void swap(NetworkSocket& sock);
	
	Type type() const { return m_type; }
	State state() const { return m_state; }
	bool isOpen() const { return m_type != NST_INVALID; }

	bool withEvents() const { return m_withEvents; }
	void setWithEvents(bool v);
	Event<> OnNewData;
	Event<> OnError;
	
	Port localPort() const;
	Port remotePort() const;
	NetworkAddr localAddress() const;
	NetworkAddr remoteAddress() const;
	Result setRemoteAddress(const NetworkAddr& addr);
	void reapplyRemoteAddress();
	
	Result OpenReliable(Port port);
	Result OpenUnreliable(Port port);
	Result OpenBroadcast(Port port);
	void Close();
	void Clear() { if(isOpen()) Close(); }
	
	Result Connect(const NetworkAddr& addr);
	Result Listen();
	
	bool isReady() const;
	int Write(const void* buffer, int nbytes);
	int Write(const std::string& buffer) { return Write(buffer.data(), (int)buffer.size()); }
	int Read(void* buffer, int nbytes);
	
	bool isDataAvailable(); // Slow!

	// WARNING: Don't use!
	void	WaitForSocketWrite(int timeout);
	void	WaitForSocketRead(int timeout);
	void	WaitForSocketReadOrWrite(int timeout);
	
	std::string debugString() const;
};



int		GetSocketErrorNr();
std::string	GetSocketErrorStr(int errnr);
std::string	GetLastErrorStr();
bool	IsMessageEndSocketErrorNr(int errnr);
void	ResetSocketError();


#endif  //  __NETWORKING_H__
