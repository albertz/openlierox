#include "network.h"

#include "server.h"
#include "client.h"
#include "game.h"
#include "updater.h"
#include "glua.h"
#include "gconsole.h"
//#include "text.h"
#include "net_worm.h"
#include "base_player.h"
#include "particle.h"
#include "http.h"
#include "util/macros.h"
#include "util/log.h"
#include "util/text.h"
#include "lua/bindings-network.h"

#ifndef DISABLE_ZOIDCOM

#include <string>
#include <iostream>
#include <memory>
#include <list>
#include <set>
#include <utility>
#include <zoidcom.h>
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

using namespace std;

int const Network::protocolVersion = 2;

LuaReference LuaEventDef::metaTable;

namespace
{
	
	mq_define_message(Connect, 0, (std::string addr_))
		: addr(addr_)
		{
			
		}
		
		std::string addr;
	mq_end_define_message()
	
	/*
	mq_define_message(Host, 0)
		{
			
		}
	mq_end_define_message()
	*/
	
	/*
	enum State
	{
		StateIdle,	// Not doing anything
		StateDisconnecting, // Starting disconnection sequence
		StateDisconnected, // Disconnected
	};*/
	
	Network::State state = Network::StateDisconnected;
	int stateTimeOut = 0;
	
	void setLuaState(Network::State s)
	{
		EACH_CALLBACK(i, networkStateChange)
		{
			(lua.call(*i), s)();
		}
	}
	
	void setState(Network::State s)
	{
		state = s;
	}
	
	#define SET_STATE(s_) DLOG("Network state: " #s_); setState(State##s_)

	MessageQueue msg;
	
	struct HttpRequest
	{
		HttpRequest(HTTP::Request* req_, HttpRequestCallback handler_)
		: req(req_), handler(handler_)
		{
		}
		
		HTTP::Request* req;
		HttpRequestCallback handler;
	};
	
	struct LuaEventList
	{
		typedef std::map<std::string, LuaEventDef*> MapT;
		MapT stringToEvent;
		std::vector<LuaEventDef*> events;
		
		LuaEventDef* add(std::string const& name, LuaEventDef* event)
		{
			size_t idx = events.size() + 1;
			
			event->idx = idx;
			std::pair<MapT::iterator, bool> r = stringToEvent.insert(std::make_pair(name, event));
			if(!r.second)
			{
				delete event;
				event = r.first->second;
			}
			events.push_back(event);
			return event;
		}
		
		LuaEventDef* assign(std::string const& name, LuaEventDef* event)
		{
			std::pair<MapT::iterator, bool> r = stringToEvent.insert(std::make_pair(name, event));
			if(!r.second)
			{
				r.first->second->callb = event->callb;
				delete event;
				event = r.first->second; 
			}
			return event;
		}
		
		void index(std::string const& name)
		{
			int idx = events.size() + 1;
			MapT::iterator i = stringToEvent.find(name);
			if(i != stringToEvent.end())
			{
				i->second->idx = idx;
				events.push_back(i->second);
			}
			else
				events.push_back(0);
		}
		
		LuaEventDef* fromIndex(size_t idx)
		{
			if(idx > 0 && idx <= events.size())
				return events[idx - 1];
			return 0;
		}
		
		void clear()
		{
			stringToEvent.clear();
			events.clear();
		}
		
		void encode(ZCom_BitStream* data)
		{
			data->addInt(events.size(), 8);
			foreach(i, events)
			{
				DLOG("Encoding lua event: " << (*i)->name);
				data->addString((*i)->name.c_str());
			}
		}
	};
	
	std::list<HttpRequest> requests;
	std::string serverName;
	std::string serverDesc;
	int registerGlobally = 1;
	HTTP::Host masterServer("comser.liero.org.pl");
	int updateTimer = 0;
	bool serverAdded = false;
	bool m_host = false;
	bool m_client = false; //? This wasn't initialized before
	int logZoidcom = 0;
	std::set<zU32> bannedIPs;
	
	int m_serverPort; // Neither was this
	
	std::string m_lastServerAddr;
	int reconnectTimer = 0;
	int connCount = 0;
	
	ZoidCom* m_zcom = 0;
	ZCom_Control* m_control = 0;
	ZCom_ConnID m_serverID = ZCom_Invalid_ID;
	LuaEventList luaEvents[Network::LuaEventGroup::Max];
	
	void processHttpRequests()
	{
		foreach_delete(i, requests)
		{
			if(!i->req || i->req->think())
			{
				if(i->handler)
					i->handler(i->req);
				else
					delete i->req;
				requests.erase(i);
			}
		}
	}
	
	void onServerRemoved(HTTP::Request* req)
	{
		if(req->success)
		{
			cout << "Unregistered from master server" << endl;
		}
		else
		{
			if(req->getError() == TCP::Socket::ErrorTimeout)
				cerr << "Unregistration from master server timed out" << endl;
			else
				cerr << "Failed to unregister from master server" << endl;
		}
		serverAdded = false;
		delete req;
	}
	
	void onServerAdded(HTTP::Request* req)
	{
		if(req->success)
		{
			serverAdded = true;
			cout << "Registered to master server" << endl;
		}
		else
		{
			serverAdded = false;
			if(req->getError() == TCP::Socket::ErrorTimeout)
				cerr << "Registration to master server timed out" << endl;
			else
				cerr << "Failed to register to master server" << endl;
		}
		delete req;
	}
	
	void onServerUpdate(HTTP::Request* req)
	{
		if(req->success)
		{
			cout << "Sent update to master server" << endl;
		}
		else
		{
			if(req->getError() == TCP::Socket::ErrorTimeout)
				cerr << "Master server update timed out" << endl;
			else
				cerr << "Failed to send update to master server" << endl;
			serverAdded = false; // Maybe the server was droped, try to reregister it
		}
		delete req;
	}
	
	void registerToMasterServer()
	{
		if(!registerGlobally) return;
		
		std::list<std::pair<std::string, std::string> > data;

		push_back(data)
			("action", "add")
			("title", serverName)
			("desc", serverDesc)
			("port", convert<std::string>::value(m_serverPort))
			("protocol", convert<std::string>::value(Network::protocolVersion))
			("mod", game.getModName())
			("map", game.level.getName())
		;
		network.addHttpRequest(masterServer.post("gusserv.php", data), onServerAdded);
	}
	
	void registerClasses() // Factorization of class registering in client and server
	{
		NetWorm::classID = m_control->ZCom_registerClass("worm",0);
		BasePlayer::classID = m_control->ZCom_registerClass("player",0);
		Game::classID = m_control->ZCom_registerClass("game",0);
		Updater::classID = m_control->ZCom_registerClass("updater",0);
		Particle::classID = m_control->ZCom_registerClass("particle",ZCOM_CLASSFLAG_ANNOUNCEDATA);
	}
	
	std::string setProxy(std::list<std::string> const& args) 
	{
		if(args.size() >= 2)
		{
			let_(i, args.begin());
			std::string const& addr = *i++;
			int port = cast<int>(*i++);
			
			masterServer.options.setProxy(addr, port);
			
			return "";
		}
		return "NET_SET_PROXY"; //TODO: help
	}
	
	std::string disconnectCmd(std::list<std::string> const& args) 
	{
		network.disconnect();
		return "";
	}
}

Network network;

void LuaEventDef::call(ZCom_BitStream* s)
{
	ZCom_BitStream* n = s->Duplicate();
	(lua.call(callb), luaReference, lua.fullReference(*n, LuaBindings::ZCom_BitStreamMetaTable))();
}

void LuaEventDef::call(LuaReference obj, ZCom_BitStream* s)
{
	ZCom_BitStream* n = s->Duplicate();
	(lua.call(callb), luaReference, obj, lua.fullReference(*n, LuaBindings::ZCom_BitStreamMetaTable))();
}

LuaEventDef::~LuaEventDef()
{
	lua.destroyReference(luaReference);
	luaReference.reset();
	lua.destroyReference(callb);
	callb.reset();
}

Network::Network()
: clientRetry(false)
{
/*
	m_host = false;
	m_zcom = NULL;
	m_control = NULL;
	connCount = 0;
	m_reconnect = false;*/
}

Network::~Network()
{
/* What's with this?
	m_host = false;
	m_client = false;
	m_serverID = ZCom_Invalid_ID;*/
}

void Network::log(char const* msg)
{
	cerr << "ZOIDCOM: " << msg << endl;
}

void Network::init()
{
	if(logZoidcom)
	{
		m_zcom = new ZoidCom(log);
		m_zcom->setLogLevel(2);
	}
	else
		m_zcom = new ZoidCom();
	
	if ( !m_zcom->Init() )
	{
		console.addLogMsg("* ERROR: UNABLE TO INITIALIZE ZOIDCOM NETWORK LIB");
	}else
	{
		console.addLogMsg("* ZOIDCOM NETWORK LIB INITIALIZED");
		console.addLogMsg("* FOR MORE INFO VISIT WWW.ZOIDCOM.COM");
	}
}

void Network::shutDown()
{
	delete m_control; m_control = 0;
	delete m_zcom; m_zcom = 0;
}

void Network::registerInConsole()
{
	console.registerVariables()
		("NET_SERVER_PORT", &m_serverPort, 9898)
		("NET_SERVER_NAME", &serverName, "Unnamed server")
		("NET_SERVER_DESC", &serverDesc, "")
		("NET_REGISTER", &registerGlobally, 1)
		("NET_SIM_LAG", &network.simLag, 0)
		("NET_SIM_LOSS", &network.simLoss, -1.f)
		("NET_UP_LIMIT", &network.upLimit, 10000)
		("NET_DOWN_BPP", &network.downBPP, 200)
		("NET_DOWN_PPS", &network.downPPS, 20)
		("NET_CHECK_CRC", &network.checkCRC, 1)
		("NET_LOG", &logZoidcom, 0)
		("NET_AUTODOWNLOADS", &network.autoDownloads, 1)
	;
	
	console.registerCommands()
		("NET_SET_PROXY", setProxy)
		("DISCONNECT", disconnectCmd)
	;
}

ZCom_ConnID Network::getServerID()
{
	return m_serverID;
}

void Network::update()
{
	if ( m_control )
	{
		m_control->ZCom_processOutput();
		m_control->ZCom_processInput(eZCom_NoBlock);
	}
	
	processHttpRequests();
	
	switch(state)
	{
		case StateDisconnected:
		case StateIdle:
		{
			mq_process_messages(msg)
				mq_case(Connect)
					if(!isDisconnected())
					{
						//if(!isDisconnecting())
						disconnect();
						
						mq_delay(); // Wait until network is disconnected
					}
					
					m_control = new Client( 0 );
					registerClasses();
					ZCom_Address address;
					address.setAddress( eZCom_AddressUDP, 0, ( data.addr + ":" + cast<string>(m_serverPort) ).c_str() );
					m_control->ZCom_Connect( address, NULL );
					//m_client = true; // We wait with setting this until we've connected
					m_lastServerAddr = data.addr;
					setLuaState(StateConnecting);
					SET_STATE(Idle);
				mq_end_case()
				
				/*
				mq_case(Host)
					if(!isDisconnected())
					{
						if(!isDisconnecting())
							disconnect();
						
						mq_delay(); // Wait until network is disconnected
					}
					
					m_control = new Server(m_serverPort);
					registerClasses();
					m_host = true;
					game.assignNetworkRole( true ); // Gives the game class node authority role
					updater.assignNetworkRole(true);
					registerToMasterServer();
					SET_STATE(Idle);
				mq_end_case()
				*/
			mq_end_process_messages()

			if(m_host)
			{
				if(registerGlobally && ++updateTimer > 6000*3)
				{
					updateTimer = 0;
					if(!serverAdded)
					{
						registerToMasterServer();
					}
					else
					{
						std::list<std::pair<std::string, std::string> > data;
						push_back(data)
							("action", "update")
							("port", convert<std::string>::value(m_serverPort))
						;
						network.addHttpRequest(masterServer.post("gusserv.php", data), onServerUpdate);
					}
				}
			}
		}
		break;
		
		case StateDisconnecting:
		{
			if(requests.size() == 0 && (connCount == 0 || stateTimeOut <= 0))
			{
				if(connCount != 0)
					WLOG(connCount << " connection(s) might not have disconnected properly.");
				setLuaState(StateDisconnected);
				SET_STATE(Disconnected);
				
				if(m_control)
				{
					m_control->Shutdown();
					network.clientRetry = false;
					
					delete m_control;
					m_control = 0;
				}
				
				connCount = 0;
				m_client = false;
				m_host = false;
				m_serverID = ZCom_Invalid_ID;
				
				game.removeNode();
				updater.removeNode();
			}
			else
				--stateTimeOut;
		}
		break;
	}
	
	if( reconnectTimer > 0 )
	{
		//disconnect();
		if(--reconnectTimer == 0)
		{
			DLOG("Reconnecting to " << m_lastServerAddr);  
			connect( m_lastServerAddr );
		}
	}
}

HTTP::Request* Network::fetchServerList()
{
	std::list<std::pair<std::string, std::string> > data;
	push_back(data)
		("action", "list")
		("protocol", convert<std::string>::value(protocolVersion))
	;
	return masterServer.post("gusserv.php", data);
}

void Network::host()
{
	//disconnect();
	assert(state == StateDisconnected); // We assume that we're disconnected

	//mq_queue(msg, Host);
	m_control = new Server(m_serverPort);
	registerClasses();
	m_host = true;
	game.assignNetworkRole( true ); // Gives the game class node authority role
	updater.assignNetworkRole(true);
	registerToMasterServer();
	setLuaState(StateHosting);
	SET_STATE(Idle);
}

void Network::connect( const std::string &_address )
{
	//disconnect(); // Done is message handler
	
	mq_queue(msg, Connect, _address);
}

void Network::disconnect( DConnEvents event )
{
	if(state == StateIdle && m_control)
	{
		setLuaState(StateDisconnecting);
		SET_STATE(Disconnecting);
		stateTimeOut = 1000;

		ZCom_BitStream *eventData = new ZCom_BitStream;
		eventData->addInt( static_cast<int>( event ), 8 );
		
		LOG("Disconnecting...");
		network.clientRetry = true;
		m_control->ZCom_disconnectAll(eventData);
	}
	
	if(serverAdded)
	{
		std::list<std::pair<std::string, std::string> > data;
		push_back(data)
			("action", "remove")
			("port", convert<std::string>::value(m_serverPort))
		;
		network.addHttpRequest(masterServer.post("gusserv.php", data), onServerRemoved);
	}
}

void Network::disconnect( ZCom_ConnID id, DConnEvents event )
{
	if(!m_control) return;
	
	std::auto_ptr<ZCom_BitStream> eventData(new ZCom_BitStream);
	eventData->addInt( static_cast<int>( event ), 8 );
	m_control->ZCom_Disconnect( id, eventData.release());
}

void Network::clear()
{
	// The lua event tables contain pointers to objects allocated by lua
	for(int t = LuaEventGroup::Game; t < LuaEventGroup::Max; ++t)
	{
		luaEvents[t].clear();
	}
}

void Network::reconnect(int delay)
{
	reconnectTimer = delay;
}

void Network::kick( ZCom_ConnID connID )
{
	if( m_control )
	{
		ZCom_BitStream *eventData = new ZCom_BitStream;
		eventData->addInt( static_cast<int>( Kick ), 8 );
		m_control->ZCom_Disconnect( connID, eventData );
	}
}

void Network::ban( ZCom_ConnID connID )
{
	if( m_control )
	{
		ZCom_Address const* addr = m_control->ZCom_getPeer(connID);
		bannedIPs.insert(addr->getIP());
	}
}

bool Network::isBanned(ZCom_ConnID connID)
{
	if( m_control )
	{
		ZCom_Address const* addr = m_control->ZCom_getPeer(connID);
		if(bannedIPs.find(addr->getIP()) != bannedIPs.end())
			return true;
	}
	return false;
}

void Network::setServerID(ZCom_ConnID serverID)
{
	m_serverID = serverID;
}

bool Network::isHost()
{
	return m_host;
}

bool Network::isClient()
{
	return m_client;
}

ZCom_Control* Network::getZControl()
{
	return m_control;
}

int Network::getServerPing()
{
	if( m_client )
	{
		if ( m_serverID ) 
			return m_control->ZCom_getConnectionStats(m_serverID).avg_ping;
	}
	return -1;
}

void Network::addHttpRequest(HTTP::Request* req, HttpRequestCallback handler)
{
	requests.push_back(HttpRequest(req, handler));
}

LuaEventDef* Network::addLuaEvent(LuaEventGroup::type type, char const* name, LuaEventDef* event)
{
	if(game.options.host)
		return luaEvents[type].add(name, event);
	else if(m_client)
		return luaEvents[type].assign(name, event);
	
	return event;
}

void Network::indexLuaEvent(LuaEventGroup::type type, char const* name)
{
	if(m_client)
		luaEvents[type].index(name);
}

void Network::encodeLuaEvents(ZCom_BitStream* data)
{
	for(int t = LuaEventGroup::Game; t < LuaEventGroup::Max; ++t)
	{
		luaEvents[t].encode(data);
	}
}

LuaEventDef* Network::indexToLuaEvent(LuaEventGroup::type type, int idx)
{
	return luaEvents[type].fromIndex(idx);
}

void Network::incConnCount()
{
	++connCount;
}

void Network::decConnCount()
{
	--connCount;
}

bool Network::isDisconnected()
{
	return state == StateDisconnected;
}

bool Network::isDisconnecting()
{
	return state == StateDisconnecting;
}

void Network::setClient(bool v)
{
	m_client = v;
}

#endif






