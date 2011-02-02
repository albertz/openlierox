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
class CHttp;

// Returns non-empty UDP masterserver address if server is registered on this UDP masterserver and won't respond on pinging
struct UdpMasterserverInfo {
	std::string name; int index;
	UdpMasterserverInfo() : index(-1) {}
	UdpMasterserverInfo(const std::string& n, int i) : name(n), index(i) {}
	operator bool() { return index >= 0; }
};

typedef ThreadVar< std::list<server_t::Ptr> > SvrList;
namespace DeprecatedGUI { class CListview; }

// Server list
class ServerList  {
public:
	typedef boost::shared_ptr<ServerList> Ptr;

	class Action  {
	public:
		virtual bool handle(server_t::Ptr& s) { return true; }
		virtual bool handleConst(const server_t::Ptr& s) { return true; }
	};
private:
	friend struct ServerListUpdater;
	friend struct UdpUpdater;

	SvrList psServerList;
	static Ptr m_instance;

	void saveList(const std::string& szFilename, SvrListFilterType filterType, SvrListSettingsFilter::Ptr settingsFilter = SvrListSettingsFilter::Ptr((SvrListSettingsFilter*)NULL));
	void loadList(const std::string& szFilename, SvrListFilterType filterType);
	void mergeWithNewInfo(server_t::Ptr found, const std::string& address, const std::string & name, int udpMasterserverIndex);
	bool parsePacket(CBytestream *bs, const SmartPointer<NetworkSocket>& sock, bool isLan);
	void parseQuery(server_t::Ptr svr, CBytestream *bs);
	std::string parseUdpServerlist(CBytestream *bs, int UdpMasterserverIndex);
	void HTTPParseList(CHttp &http);
public:
	ServerList();
	~ServerList();

	void save();
	bool process();

	server_t::Ptr findServer(const NetworkAddr& addr, const std::string & name = "");
	void removeServer(const std::string& szAddress);
	void clear(SvrListFilterType filterType);

	void clearAuto();
	void shutdown();
	void pingLAN();
	server_t::Ptr addServer(const std::string& address, bool bManual, const std::string & name = "Untitled", int udpMasterserverIndex = -1);
	void addFavourite(const std::string& szName, const std::string& szAddress);
	server_t::Ptr findServerStr(const std::string& szAddress, const std::string & name = "");
	void pingServer(server_t::Ptr svr);
	void wantsToJoin(const std::string& Nick, server_t::Ptr svr);
	void queryServer(server_t::Ptr svr);
	void getServerInfo(server_t::Ptr svr);
	UdpMasterserverInfo	getUdpMasterserverForServer(const std::string& szAddress);

	void updateList();
	void updateUDPList();
	void refreshList();
	void refreshServer(server_t::Ptr s, bool updategui = true);
	bool isProcessing();

	void fillList(DeprecatedGUI::CListview *lv, SvrListFilterType filterType, SvrListSettingsFilter::Ptr settingsFilter = SvrListSettingsFilter::Ptr((SvrListSettingsFilter*)NULL));

	void each(Action& act);
	void eachConst(Action& act) const;

	static Ptr get();
};

#endif
