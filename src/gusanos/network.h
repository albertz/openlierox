#ifndef NETWORK_H
#define NETWORK_H


#include "netstream.h"
#include <string>
#include <boost/function.hpp>
#include "lua51/luaapi/types.h"

#include "message_queue.h"

const unsigned int INVALID_NODE_ID = 0;

struct LuaEventDef
{
	static LuaReference metaTable;
	
	LuaEventDef(std::string name_, LuaReference callb_)
	: name(name_), idx(0), callb(callb_)
	{
	}
	
	~LuaEventDef();
	
	void call(Net_BitStream*);
	
	void call(LuaReference, Net_BitStream*);
	
	void* operator new(size_t count);
	
	void operator delete(void* block)
	{
		// Lua frees the memory
	}
	
	void* operator new(size_t count, void* space)
	{
		return space;
	}
	
	std::string  name;
	size_t       idx;
	LuaReference callb;
	LuaReference luaReference;
};

class Network
{
public:
	friend class Client;
	friend class Server;
	
	static int const protocolVersion;
		
	enum NetEvents
	{
		PLAYER_REQUEST,
		RConMsg,
		ConsistencyInfo,
	};
	
	enum DConnEvents
	{
		Kick,
		ServerMapChange,
		Quit,
		IncompatibleData,
		IncompatibleProtocol
	};
	
	enum State
	{
		StateIdle,	// Not doing anything
		StateDisconnecting, // Starting disconnection sequence
		StateDisconnected, // Disconnected
		
		StateConnecting, // Pseudo-state
		StateHosting,  // Pseudo-state
	};
	
	struct ClientEvents
	{
		enum type
		{
			LuaEvents,
			Max
		};
	};
	
	struct ConnectionReply
	{
		enum type
		{
			Refused = 0,
			Retry,
			Banned,
		};
	};
	
	struct LuaEventGroup
	{
		enum type
		{
			GusGame,
			CWormHumanInputHandler,
			Worm,
			Particle,
			Max
		};
	};
	
	Network();
	~Network();
	
	static void log(char const* msg);
	
	static void init();
	static void shutDown();
	static void registerInConsole();
	static void update();
	
	static void olxHost();
	static void olxConnect();
	static void disconnect( DConnEvents event = Quit );
	static void disconnect( Net_ConnID id, DConnEvents event );
	static void olxReconnect(int delay = 1);
	static void clear();
	
	static void setServerID( Net_ConnID serverID );
	static Net_ConnID getServerID();
	
	static bool isHost();
	static bool isClient();
		
	static LuaEventDef* addLuaEvent(LuaEventGroup::type, char const* name, LuaEventDef* event);
	static void indexLuaEvent(LuaEventGroup::type, char const* name);
	static LuaEventDef* indexToLuaEvent(LuaEventGroup::type type, int idx);
	static void encodeLuaEvents(Net_BitStream* data);
	
	static Net_Control* getNetControl();

	static void incConnCount();
	static void decConnCount();
	
	static bool isDisconnected();
	static bool isDisconnecting();
		
	int checkCRC;
	
private:
	static void setClient(bool v);
};

extern Network network;

#endif // _NETWORK_H_
