#include "bindings-network.h"

#include "luaapi/types.h"
#include "luaapi/macros.h"
#include "luaapi/classes.h"

#include "../network.h"
#include "../game.h"
#include "../base_player.h"
#include "../base_worm.h"
#include "../particle.h"
#include "../encoding.h"
#include "sockets.h"
#include "util/log.h"
#include "tcp.h"

#include <iostream>
#include <memory>
#include <utility>
#include <list>
#include <cstring>
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
using boost::lexical_cast;

namespace LuaBindings
{
	
LuaReference SocketMetaTable;
LuaReference ZCom_BitStreamMetaTable;
LuaReference LuaGameEventMetaTable, LuaPlayerEventMetaTable, LuaWormEventMetaTable, LuaParticleEventMetaTable;


class LuaSocket : public TCP::Socket
{
public:
	LuaSocket(int s_ /*LuaReference ref, int s_, LuaContext context_, LuaReference recvCallback_*/)
	: TCP::Socket(s_, 10*60) //, context(context_), recvCallback(recvCallback_)
	, dataSender(0) //, luaReference(ref)
	{
	}
	
	void send(char const* p, size_t len)
	{
		char* m = new char[len];
		memcpy(m, p, len);
		sendQueue.push_back(std::make_pair(m, m+len));
	}
	
	void think()
	{
		if(error)
		{
			cerr << "Error!" << endl;
			error = ErrorNone;
		}
		
		TCP::Socket::think();
		
		if(connected)
		{
			if(dataSender) // We're still sending data
			{
				if(dataSender->resume())
				{
					// Send done or errored
					delete dataSender; dataSender = 0;
					delete[] sendQueue.front().first;
					sendQueue.pop_front();
				}
			}
			else if(!sendQueue.empty())
			{
				std::pair<char*, char*>& t = sendQueue.front();
				
				if(!(dataSender = TCP::Socket::send(t.first, t.second)))
				{
					delete[] t.first;
					sendQueue.pop_front();
				}
			}
			
			if(readChunk())
				return;
		}
	}
	
	~LuaSocket()
	{
		delete dataSender;
		foreach(i, sendQueue)
		{
			delete[] i->first;
		}
	}
	
	char const* begin()
	{
		return dataBegin;
	}
	
	char const* end()
	{
		return dataEnd;
	}
	
private:
	TCP::Socket::ResumeSend* dataSender;
	std::list<std::pair<char*, char*> > sendQueue;
	//LuaContext context;
	//LuaReference recvCallback;
	LuaReference luaReference;
};

/*! tcp_connect(addr, port)
	
	Returns a TCPSocket object connecting to //addr// on port //port//.
*/
int l_tcp_connect(lua_State* L)
{
	LuaContext context(L);
	
	char const* addr = lua_tostring(context, 1);
	if(!addr) return 0;
	int port = lua_tointeger(context, 2);
	
	//lua_pushvalue(context, 3);
	//LuaReference recvCallback = context.createReference();
	
	sockaddr_in server;
	
	std::auto_ptr<TCP::Hostent> hp(TCP::resolveHost( addr ));
	
	if(!hp.get())
		return 0;
	
    TCP::createAddr(server, hp.get(), port);
    
    int s;
    if((s = TCP::socketNonBlock()) < 0)
    	return 0;
    	
    if(!TCP::connect(s, server))
    	return 0;

	void* space = lua_newuserdata(context, sizeof(LuaSocket));
	//lua_pushvalue(context, -1);
	//LuaSocket* sock = new (space) LuaSocket(context.createReference(), s, context, recvCallback);
	new (space) LuaSocket(s);
	context.push(SocketMetaTable);
	lua_setmetatable(context, -2);
	
	return 1;
}

/*! TCPSocket:send(data)
	
	Sends //data// on the socket.
*/
LMETHOD(LuaSocket, tcp_send,
	size_t len;
	char const* s = lua_tolstring(context, 2, &len);
	if(!s) return 0;
	p->send(s, len);
	return 0;
)

/*! TCPSocket:think()
	
	Processes data sending and recieving.
	
	If there's data recieved, it's returned as a string.
	
	If there's no data recieved, nil is returned. 
	
	Must be called regularly for data sends and connection to finish.
*/
LMETHOD(LuaSocket, tcp_think,
	p->think();
	if(p->begin() == p->end())
		return 0;
	DLOG((void *)p->begin() << ", " << (void *)p->end() << ", size: " << (p->end() - p->begin()));
	lua_pushlstring(context, p->begin(), p->end() - p->begin());
	return 1;
)

LMETHOD(LuaSocket, tcp_destroy,
	p->~LuaSocket();
	return 0;
)

/*! Bitstream:dump(value)
	
	Adds a lua object to the bitstream.
	
	WARNING: This function is not very space efficient and can only encode tables
	, strings, numbers, booleans and nil (or tables with keys and values of those types)
*/
METHODC(ZCom_BitStream, bitStream_dump,
	context.serialize(*p, 2);
	context.pushvalue(1);
	return 1;
)

/*! Bitstream:undump()
	
	Extracts a lua object added to the bitstream using the //dump// method.
*/
METHODC(ZCom_BitStream, bitStream_undump,
	context.deserialize(*p);
	return 1;
)

/*! Bitstream:encode_elias_gamma(value)
	
	Adds an integer above or equal to 1 encoded using the elias gamma universal encoding.
*/
METHODC(ZCom_BitStream, bitStream_encodeEliasGamma,
	int v = lua_tointeger(context, 2);
	if(v < 1)
	{
		WLOG("Number " << v << " can't be encoded with elias gamma. Encoded as 1.");
		v = 1;
	}
	Encoding::encodeEliasGamma(*p, static_cast<unsigned int>(v));
	context.pushvalue(1);
	return 1;
)

/*! Bitstream:decode_elias_gamma()
	
	Extracts an integer above or equal to 1 encoded using the elias gamma universal encoding.
*/
METHODC(ZCom_BitStream, bitStream_decodeEliasGamma,
	context.push(Encoding::decodeEliasGamma(*p));
	return 1;
)

/*! Bitstream:add_bool()
	
	Extracts a boolean value from this bitstream (Returns either true or false).
*/
METHODC(ZCom_BitStream, bitStream_addBool,
	p->addBool(lua_toboolean(context, 2));
	context.pushvalue(1);
	return 1;
)

/*! Bitstream:get_bool()
	
	Extracts a boolean value from this bitstream (Returns either true or false).
*/
METHODC(ZCom_BitStream, bitStream_getBool,
	context.push(p->getBool());
	return 1;
)

/*! Bitstream:add_int(value[, bits])
	
	Adds an integer to this bitstream encoded in //bits// bits.
	
	If bits is left out, 32 bits is assumed.
*/
METHODC(ZCom_BitStream, bitStream_addInt,
	int bits = 32;
	if(lua_gettop(context) >= 3)
		bits = lua_tointeger(context, 3);
	p->addInt(lua_tointeger(context, 2), bits);
	context.pushvalue(1);
	return 1;
)

/*! Bitstream:get_int([bits])
	
	Extracts an integer from this bitstream encoded in //bits// bits.
	
	If bits is left out, 32 bits is assumed.
*/
METHODC(ZCom_BitStream, bitStream_getInt,
	int bits = 32;
	if(lua_gettop(context) >= 2)
		bits = lua_tointeger(context, 2);
	context.push(static_cast<int>(p->getInt(bits)));
	return 1;
)

/*! Bitstream:add_string(str)
	
	Adds the string //str// to this bitstream.
*/
METHODC(ZCom_BitStream, bitStream_addString,
	p->addString(lua_tostring(context, 2));
	context.pushvalue(1);
	return 1;
)

/*! Bitstream:get_string()
	
	Extracts a string from this bitstream.
*/
METHODC(ZCom_BitStream, bitStream_getString,
	context.push(p->getStringStatic());
	return 1;
)

METHODC(ZCom_BitStream, bitStream_destroy,
	delete p;
	return 0;
)

/*! new_bitstream()
	
	Returns a new Bitstream object.
*/
int l_new_bitstream(lua_State* L)
{
	LuaContext context(L);
	ZCom_BitStream* p = new ZCom_BitStream;
	context.pushFullReference(*p, ZCom_BitStreamMetaTable);
	return 1;
}

#define LUA_EVENT_SEND_METHOD(type_, params_, decl_, cases_, body_) \
LMETHOD(LuaEventDef, luaEvent_##type_##_send, \
	if(p->idx > 0)	{ \
		ZCom_BitStream* userdata = 0; \
		eZCom_SendMode mode = eZCom_ReliableOrdered; \
		ZCom_ConnID connID = 0; \
		zU8 rules = ZCOM_REPRULE_AUTH_2_ALL | ZCOM_REPRULE_OWNER_2_AUTH; \
		decl_ \
		switch(lua_gettop(context)) \
		{ \
			default: if(lua_gettop(context) < params_+3) break; \
			case params_+5: rules = lua_tointeger(context, params_+5); \
			case params_+4: mode = (eZCom_SendMode)lua_tointeger(context, params_+4); \
			case params_+3: connID = (ZCom_ConnID)lua_tointeger(context, params_+3); \
			case params_+2: userdata = ASSERT_OBJECT(ZCom_BitStream, params_+2); \
			cases_ \
		} \
		body_ \
	} \
	return 0; \
)

/*! NetworkGameEvent:send([data[, connection[, mode[, rules]]]])

	Sends a game event to one or more computers.
	
	//data// is a bitstream to send with the event.
	
	//connection// is a connection ID to send the event on. If it's 0, the //rules// parameter decides who to send to.
	
	//mode// is one of these values (default is SendMode.ReliableOrdered):
	* SendMode.ReliableUnordered : The events always arrive, but may come in a different order than sent.
	* SendMode.ReliableOrdered : The events always arrive and in the order sent.
	* SendMode.Unreliable : The events may or may not arrive and in any order.
	
	//rules// decides what computers to send the event to if //connection// is 0 or left out.
	It can be any (or sometimes a sum) of:
	* RepRule.Auth2All : Event is sent from the server to clients.
	* RepRule.Auth2Owner : Event is sent from the server to clients owning the node.
	* RepRule.Auth2Proxy : Event is sent from the server to clients not owning the node.
	* RepRule.None : Event is not sent.
	* RepRule.Owner2Auth : Event is sent from clients owning the node to the server.
	
	A valid combination is (RepRule.Owner2Auth + RepRule.Auth2All) which sends the event
	from owners to the server or from the server to all clients (depending on if we're the server or not).
*/
LUA_EVENT_SEND_METHOD(game, 0, /* no declarations */, /* no cases */,
	game.sendLuaEvent(p, mode, rules, userdata, connID);
)

/*! NetworkPlayerEvent:send(player, [data[, connection[, mode[, rules]]]])

	Sends a player event to one or more computers.
	
	//player// is the Player object to send the event on.
	
	See NetworkGameEvent:send for documentation of the other parameters.
*/
LUA_EVENT_SEND_METHOD(player, 1,
	BasePlayer* player = 0;
,
	case 2: player = ASSERT_OBJECT(BasePlayer, 2);
,
	if(player)
		player->sendLuaEvent(p, mode, rules, userdata, connID);
)

/*! NetworkWormEvent:send(worm, [data[, connection[, mode[, rules]]]])

	Sends a worm event to one or more computers.
	
	//worm// is the Worm object to send the event on.
	
	See NetworkGameEvent:send for documentation of the other parameters.
*/
LUA_EVENT_SEND_METHOD(worm, 1,
	BaseWorm* worm = 0;
,
	case 2: worm = ASSERT_OBJECT(BaseWorm, 2);
,
	if(worm)
		worm->sendLuaEvent(p, mode, rules, userdata, connID);
)

//! version 0.9c

/*! NetworkParticleEvent:send(particle, [data[, connection[, mode[, rules]]]])

	Sends a particle event to one or more computers.
	
	//particle// is the Particle object to send the event on.
	
	See NetworkGameEvent:send for documentation of the other parameters.
*/
LUA_EVENT_SEND_METHOD(particle, 1,
	Particle* particle = 0;
,
	case 2: particle = ASSERT_OBJECT(Particle, 2);
,
	if(particle)
		particle->sendLuaEvent(p, mode, rules, userdata, connID);
)

//! version any

/*! network_game_event(name, handler)
	
	Creates and returns a NetworkGameEvent object.
	//name// is any string that is unique to this NetworkGameEvent.
	//handler// is a function that is called when this event is recieved on this client/server.
	It has the the form:
	<code>
	function(event, data)
	</code>
	Where //event// is the NetworkGameEvent returned by //network_game_event// and //data//
	is a bitstream with data sent with the event.
*/
int l_network_game_event(lua_State* L)
{
	LuaContext context(L);
	char const* name = lua_tostring(context, 1);
	if(!name) return 0;
	lua_pushvalue(context, 2);
	LuaReference ref = context.createReference();
	LuaEventDef* event = lua_new_m_keep(LuaEventDef, (name, ref), context, LuaGameEventMetaTable);
	event = network.addLuaEvent(Network::LuaEventGroup::Game, name, event);

	return 1;
}

/*! network_player_event(name, handler)
	
	Creates and returns a NetworkPlayerEvent object.
	//name// is any string that is unique to this NetworkPlayerEvent.
	//handler// is a function that is called when this event is recieved on this client/server.
	It has the the form:
	<code>
	function(event, player, data)
	</code>
	Where //event// is the NetworkPlayerEvent returned by //network_player_event//, //player//
	is the Player object the event was sent on, and //data// is a bitstream with data sent
	with the event.
*/
int l_network_player_event(lua_State* L)
{
	LuaContext context(L);
	char const* name = lua_tostring(context, 1);
	if(!name) return 0;
	lua_pushvalue(context, 2);
	LuaReference ref = context.createReference();
	LuaEventDef* event = lua_new_m_keep(LuaEventDef, (name, ref), context, LuaPlayerEventMetaTable);
	event = network.addLuaEvent(Network::LuaEventGroup::Player, name, event);

	return 1;
}

/*! network_worm_event(name, handler)
	
	Creates and returns a NetworkWormEvent object.
	//name// is any string that is unique to this NetworkWormEvent.
	//handler// is a function that is called when this event is recieved on this client/server.
	It has the the form:
	<code>
	function(event, worm, data)
	</code>
	Where //event// is the NetworkWormEvent returned by //network_worm_event//, //worm//
	is the Worm object the event was sent on, and //data// is a bitstream with data sent
	with the event.
*/
int l_network_worm_event(lua_State* L)
{
	LuaContext context(L);
	char const* name = lua_tostring(context, 1);
	if(!name) return 0;
	lua_pushvalue(context, 2);
	LuaReference ref = context.createReference();
	LuaEventDef* event = lua_new_m_keep(LuaEventDef, (name, ref), context, LuaWormEventMetaTable);
	event = network.addLuaEvent(Network::LuaEventGroup::Worm, name, event);

	return 1;
}

//! version 0.9c

/*! network_particle_event(name, handler)
	
	Creates and returns a NetworkParticleEvent object.
	//name// is any string that is unique to this NetworkParticleEvent.
	//handler// is a function that is called when this event is recieved on this client/server.
	It has the the form:
	<code>
	function(event, particle, data)
	</code>
	Where //event// is the NetworkParticleEvent returned by //network_particle_event//, //particle//
	is the Particle object the event was sent on, and //data// is a bitstream with data sent
	with the event.
*/
int l_network_particle_event(lua_State* L)
{
	LuaContext context(L);
	char const* name = lua_tostring(context, 1);
	if(!name) return 0;
	lua_pushvalue(context, 2);
	LuaReference ref = context.createReference();
	LuaEventDef* event = lua_new_m_keep(LuaEventDef, (name, ref), context, LuaParticleEventMetaTable);
	event = network.addLuaEvent(Network::LuaEventGroup::Particle, name, event);

	return 1;
}

//! version any

void initNetwork(LuaContext& context)
{
	context.functions()
		("tcp_connect", l_tcp_connect)
		("network_game_event", l_network_game_event)
		("network_player_event", l_network_player_event)
		("network_worm_event", l_network_worm_event)
		("network_particle_event", l_network_particle_event)
		("new_bitstream", l_new_bitstream)
	;
	
	CLASSM(Socket,
		("__gc", l_tcp_destroy)
	,
		("send", l_tcp_send)
		("think", l_tcp_think)
	)
	
	CLASS(LuaGameEvent,
		("send", l_luaEvent_game_send)
	)
	
	CLASS(LuaPlayerEvent,
		("send", l_luaEvent_player_send)
	)
	
	CLASS(LuaWormEvent,
		("send", l_luaEvent_worm_send)
	)
	
	CLASS(LuaParticleEvent,
		("send", l_luaEvent_particle_send)
	)
	
	CLASSM(ZCom_BitStream,
		("__gc", l_bitStream_destroy)
	,
		("add_int", l_bitStream_addInt)
		("get_int", l_bitStream_getInt)
		("add_string", l_bitStream_addString)
		("get_string", l_bitStream_getString)
		("encode_elias_gamma", l_bitStream_encodeEliasGamma)
		("decode_elias_gamma", l_bitStream_decodeEliasGamma)
		("add_bool", l_bitStream_addBool)
		("get_bool", l_bitStream_getBool)
		("dump", l_bitStream_dump)
		("undump", l_bitStream_undump)
	)
	
	ENUM(SendMode,
		("ReliableUnordered", eZCom_ReliableUnordered)
		("ReliableOrdered", eZCom_ReliableOrdered)
		("Unreliable", eZCom_Unreliable)
	)
	
	ENUM(RepRule,
		("Auth2All", ZCOM_REPRULE_AUTH_2_ALL)
		("Auth2Owner", ZCOM_REPRULE_AUTH_2_OWNER)
		("Auth2Proxy", ZCOM_REPRULE_AUTH_2_PROXY)
		("None", ZCOM_REPRULE_NONE)
		("Owner2Auth", ZCOM_REPRULE_OWNER_2_AUTH)
	)
	
	
	ENUM(Network,
		("Connecting", Network::StateConnecting)
		("Disconnecting", Network::StateDisconnecting)
		("Disconnected", Network::StateDisconnected)
		("Hosting", Network::StateHosting)
	)

	
	lua_pushboolean(context, !network.isClient());
	lua_setfield(context, LUA_GLOBALSINDEX, "AUTH");
}

}
