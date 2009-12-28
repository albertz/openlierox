#include "network.h"

#include "server.h"
#include "client.h"
#include "gusgame.h"
#include "updater.h"
#include "glua.h"
#include "gconsole.h"
#include "net_worm.h"
#include "game/WormInputHandler.h"
#include "particle.h"
#include "util/macros.h"
#include "util/log.h"
#include "util/text.h"
#include "lua/bindings-network.h"

#include <string>
#include <iostream>
#include <memory>
#include <list>
#include <set>
#include <utility>
#include "netstream.h"
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

using namespace std;

int const Network::protocolVersion = 2;

LuaReference LuaEventDef::metaTable;

namespace
{

	mq_define_message(Connect, 0, ()) {}
	mq_end_define_message()

	Network::State state = Network::StateDisconnected;
	int stateTimeOut = 0;

	void setLuaState(Network::State s)
	{
		EACH_CALLBACK(i, networkStateChange) {
			(lua.call(*i), s)();
		}
	}

	void setState(Network::State s)
	{
		state = s;
	}

#define SET_STATE(s_) DLOG("Network state: " #s_); setState(State##s_)

	MessageQueue msg;

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
			if(!r.second) {
				delete event;
				event = r.first->second;
			}
			events.push_back(event);
			return event;
		}

		LuaEventDef* assign(std::string const& name, LuaEventDef* event)
		{
			std::pair<MapT::iterator, bool> r = stringToEvent.insert(std::make_pair(name, event));
			if(!r.second) {
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
			if(i != stringToEvent.end()) {
				i->second->idx = idx;
				events.push_back(i->second);
			} else
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

		void encode(Net_BitStream* data)
		{
			data->addInt(events.size(), 8);
			foreach(i, events) {
				DLOG("Encoding lua event: " << (*i)->name);
				data->addString((*i)->name.c_str());
			}
		}
	};

	std::string serverName;
	std::string serverDesc;
	bool m_host = false;
	bool m_client = false; //? This wasn't initialized before
	int logNetstream = 0; //TODO: Netstream
	std::set<Net_U32> bannedIPs;

	int m_serverPort; // Neither was this

	std::string m_lastServerAddr;
	int reconnectTimer = 0;
	int connCount = 0;

	NetStream* m_netstream = 0;
	Net_Control* m_control = 0;
	Net_ConnID m_serverID = Net_Invalid_ID;
	LuaEventList luaEvents[Network::LuaEventGroup::Max];

	void registerClasses() // Factorization of class registering in client and server
	{
		NetWorm::classID = m_control->Net_registerClass("worm",0);
		CWormInputHandler::classID = m_control->Net_registerClass("player",0);
		GusGame::classID = m_control->Net_registerClass("gusGame",0);
		Updater::classID = m_control->Net_registerClass("updater",0);
		Particle::classID = m_control->Net_registerClass("particle",Net_CLASSFLAG_ANNOUNCEDATA);
	}

	std::string disconnectCmd(std::list<std::string> const& args)
	{
		return "Gusanos disconnect command not available";
	}
}

Network network;

void LuaEventDef::call(Net_BitStream* s)
{
	Net_BitStream* n = s->Duplicate();
	(lua.call(callb), luaReference, lua.fullReference(*n, LuaBindings::Net_BitStreamMetaTable))();
}

void LuaEventDef::call(LuaReference obj, Net_BitStream* s)
{
	Net_BitStream* n = s->Duplicate();
	(lua.call(callb), luaReference, obj, lua.fullReference(*n, LuaBindings::Net_BitStreamMetaTable))();
}

LuaEventDef::~LuaEventDef()
{
	lua.destroyReference(luaReference);
	luaReference.reset();
	lua.destroyReference(callb);
	callb.reset();
}

Network::Network()
{
}

Network::~Network()
{
}

void Network::log(char const* msg)
{
	cerr << "Gusanos Network: " << msg << endl;
}

void Network::init()
{
	if(logNetstream) { //TODO: Netstream
		m_netstream = new NetStream(log);
	} else
		m_netstream = new NetStream();

	if ( !m_netstream->Init() ) {
		console.addLogMsg("* ERROR: UNABLE TO INITIALIZE NETSTREAM NETWORK LIB");
	} else {
		console.addLogMsg("* NETSTREAM NETWORK LIB INITIALIZED");
	}
}

void Network::shutDown()
{
	delete m_control;
	m_control = 0;
	delete m_netstream;
	m_netstream = 0;
}

void Network::registerInConsole()
{
	console.registerVariables()
	("NET_CHECK_CRC", &network.checkCRC, 1)
	;

}

Net_ConnID Network::getServerID()
{
	return m_serverID;
}

void Network::update()
{
	if ( m_control ) {
		m_control->Net_processOutput();
		m_control->Net_processInput();
	}

	switch(state) {
			case StateConnecting:
			case StateHosting:
			break;

			case StateDisconnected:
			case StateIdle: {
				mq_process_messages(msg)
				mq_case(Connect)
				if(!isDisconnected()) {
					//if(!isDisconnecting())
					disconnect();

					mq_delay(); // Wait until network is disconnected
				}

				m_control = new Client( 0 );
				registerClasses();
				m_control->Net_Connect();
				//m_client = true; // We wait with setting this until we've connected
				setLuaState(StateConnecting);
				SET_STATE(Idle);
				mq_end_case()

				mq_end_process_messages()
			}
			break;

			case StateDisconnecting: {
				if(connCount == 0 || stateTimeOut <= 0) {
					if(connCount != 0)
						WLOG(connCount << " connection(s) might not have disconnected properly.");
					setLuaState(StateDisconnected);
					SET_STATE(Disconnected);

					if(m_control) {
						m_control->Shutdown();
						delete m_control;
						m_control = 0;
					}

					connCount = 0;
					m_client = false;
					m_host = false;
					m_serverID = Net_Invalid_ID;

					gusGame.removeNode();
					updater.removeNode();
				} else
					--stateTimeOut;
			}
			break;
	}

	if( reconnectTimer > 0 ) {
		//disconnect();
		if(--reconnectTimer == 0) {
			DLOG("Reconnecting Gusanos");
			olxConnect();
		}
	}
}

void Network::olxHost()
{
	//disconnect();
	assert(state == StateDisconnected); // We assume that we're disconnected

	//mq_queue(msg, Host);
	m_control = new Server();
	registerClasses();
	m_host = true;
	gusGame.assignNetworkRole( true ); // Gives the gusGame class node authority role
	updater.assignNetworkRole(true);
	setLuaState(StateHosting);
	SET_STATE(Idle);
}

void Network::olxConnect()
{
	//disconnect(); // Done is message handler

	mq_queue(msg, Connect);
}

void Network::disconnect( DConnEvents event )
{
	if(state == StateIdle && m_control) {
		setLuaState(StateDisconnecting);
		SET_STATE(Disconnecting);
		stateTimeOut = 1000;

		Net_BitStream *eventData = new Net_BitStream;
		eventData->addInt( static_cast<int>( event ), 8 );

		LOG("Disconnecting...");
		m_control->Net_disconnectAll(eventData);
	}
}

void Network::disconnect( Net_ConnID id, DConnEvents event )
{
	if(!m_control)
		return;

	std::auto_ptr<Net_BitStream> eventData(new Net_BitStream);
	eventData->addInt( static_cast<int>( event ), 8 );
	m_control->Net_Disconnect( id, eventData.release());
}

void Network::clear()
{
	// The lua event tables contain pointers to objects allocated by lua
	for(int t = LuaEventGroup::GusGame; t < LuaEventGroup::Max; ++t) {
		luaEvents[t].clear();
	}
}

void Network::olxReconnect(int delay)
{
	reconnectTimer = delay;
}

void Network::setServerID(Net_ConnID serverID)
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

Net_Control* Network::getNetControl()
{
	return m_control;
}

LuaEventDef* Network::addLuaEvent(LuaEventGroup::type type, char const* name, LuaEventDef* event)
{
	if(gusGame.options.host)
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

void Network::encodeLuaEvents(Net_BitStream* data)
{
	for(int t = LuaEventGroup::GusGame; t < LuaEventGroup::Max; ++t) {
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






