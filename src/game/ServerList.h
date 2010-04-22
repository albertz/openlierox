/*
 *  ServerList.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.04.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_SERVERLIST_H__
#define __OLX_SERVERLIST_H__

#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include "Networking.h"
#include "CScriptableVars.h"
#include "ThreadVar.h"
#include "Version.h"

enum {
	SVRLIST_TIMEOUT =  7000, 
	MAX_QUERIES = 3
};


enum SvrListFilterType { SLFT_CustomSettings, SLFT_Lan, SLFT_Favourites };

struct SvrListSettingsFilter {
	typedef boost::shared_ptr<SvrListSettingsFilter> Ptr;
	
	std::map<std::string, ScriptVar_t> settings;
	
	bool loadFromFile(const std::string& cfgfile);
	static Ptr Load(const std::string& cfgfile) {
		Ptr s( new SvrListSettingsFilter() );
		if(s->loadFromFile(cfgfile)) return s;
		return Ptr((SvrListSettingsFilter*)NULL);
	}
};


// Server structure
struct server_t {
	server_t() {
		SetNetAddrValid(sAddress, false);
		bAllowConnectDuringGame = false;
		bBehindNat = false;
		lastPingedPort = 0;
		isLan = isFavourite = false;
	}
	
	bool	bIgnore;
	bool	bProcessing;
    bool    bManual;
	int		nPings;
	int		nQueries;
	bool	bgotPong;
	bool	bgotQuery;
	AbsTime	fInitTime;
	bool	bAddrReady;
	AbsTime	fLastPing;
	AbsTime	fLastQuery;
    AbsTime	fQueryTimes[MAX_QUERIES+1];
	
	// Server address
	std::string	szAddress;
	NetworkAddr	sAddress; // Does not include port
	
	// Server details
	std::string	szName;
	int		nState;
	int		nNumPlayers;
	int		nMaxPlayers;
	int		nPing;
	bool	bAllowConnectDuringGame;
	Version tVersion;
	
	// First int is port, second is UDP masterserver idx
	// If server responds to ping the port which responded moved to the top of the list
	std::vector< std::pair<int, int> > ports;
	int		lastPingedPort;
	bool	bBehindNat;	// Accessible only from UDP masterserver
	
	bool	isLan;
	bool	isFavourite;
	
	std::string serverID;
	typedef std::map<std::string, ScriptVar_t> SettingsMap;
	SettingsMap settings;	
	ScriptVar_t getSetting(const std::string& name); // will handle also cases where it is not set and return unset
	
	bool	matches(SvrListFilterType filterType, SvrListSettingsFilter::Ptr settingsFilter = SvrListSettingsFilter::Ptr((SvrListSettingsFilter*)NULL));
	
	typedef boost::shared_ptr<server_t> Ptr;
};


class CBytestream;

// AbsTime to wait before pinging/querying the server again (in milliseconds)
#define	PingWait  1000
#define	QueryWait  1000

// Server list
void		SvrList_Init();
void		SvrList_Save();
bool		SvrList_Process();

server_t::Ptr	SvrList_FindServer(const NetworkAddr& addr, const std::string & name = "");
void        SvrList_RemoveServer(const std::string& szAddress);
void		SvrList_Clear(SvrListFilterType filterType);

void        SvrList_ClearAuto();
void		SvrList_Shutdown();
void		SvrList_PingLAN();
server_t::Ptr	SvrList_AddServer(const std::string& address, bool bManual, const std::string & name = "Untitled", int udpMasterserverIndex = -1);
void		SvrList_AddFavourite(const std::string& szName, const std::string& szAddress);
server_t::Ptr	SvrList_FindServerStr(const std::string& szAddress, const std::string & name = "");
void		SvrList_PingServer(server_t::Ptr svr);
bool		SvrList_RemoveDuplicateNATServers(server_t::Ptr defaultServer);
bool		SvrList_RemoveDuplicateDownServers(server_t::Ptr defaultServer);
void		SvrList_WantsJoin(const std::string& Nick, server_t::Ptr svr);
void		SvrList_QueryServer(server_t::Ptr svr);
void		SvrList_GetServerInfo(server_t::Ptr svr);

void		SvrList_UpdateList();
void		SvrList_UpdateUDPList();
void		SvrList_RefreshList();
void        SvrList_RefreshServer(server_t::Ptr s, bool updategui = true);

// Returns non-empty UDP masterserver address if server is registered on this UDP masterserver and won't respond on pinging
struct UdpMasterserverInfo {
	std::string name; int index;
	UdpMasterserverInfo() : index(-1) {}
	UdpMasterserverInfo(const std::string& n, int i) : name(n), index(i) {}
	operator bool() { return index >= 0; }
};
UdpMasterserverInfo	SvrList_GetUdpMasterserverForServer(const std::string& szAddress);

typedef ThreadVar< std::list<server_t::Ptr> > SvrList;
bool		SvrList_IsProcessing();
SvrList&	SvrList_currentServerList();


namespace DeprecatedGUI { class CListview; }

void Menu_SvrList_FillList(DeprecatedGUI::CListview *lv, SvrListFilterType filterType, SvrListSettingsFilter::Ptr settingsFilter = SvrListSettingsFilter::Ptr((SvrListSettingsFilter*)NULL));


#endif
