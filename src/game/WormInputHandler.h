/*
 *  WormInputHandler.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#ifndef __WORMINPUTHANDLER_H__
#define __WORMINPUTHANDLER_H__

#include <SDL.h>
#include <string>

#include "util/BaseObject.h"
#include "gusanos/netstream.h"
#include "gusanos/luaapi/types.h"
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <vector>
using boost::shared_ptr;

struct PlayerOptions;
class CWorm;
class BasePlayerInterceptor;
class WeaponType;
struct LuaEventDef;

// Note: None of the BaseActions should assume a combination of keys.
// For example: Activating JUMP and CHANGE does nothing here ( instead 
// of shooting the Ninja Rope ) So key combinations should be created
// on the Player class instead. Because of that, all actions in the 
// CWormInputHandler class are direct ( they do nothing more and nothing less
// than what the name tells )

// Note2: All access to the worm class from a derivation of CWormInputHandler
// should pass by the CWormInputHandler class ( This is because the CWormInputHandler
// class will be responsible of the network part )

#define COMPACT_EVENTS
#define COMPACT_ACTIONS


class CWorm;
class CViewport;
struct lua_State;

class CWormInputHandler : public BaseObject {
protected:
	CWorm* m_worm;
public:
	Uint32 uniqueID;
	CWormInputHandler(CWorm* w) : m_worm(w), uniqueID(0) { gusInit(w); }
	
	CWorm* worm() const { return m_worm; }
	virtual bool weOwnThis() const;
	virtual CServerConnection* ownerClient() const;

	virtual std::string name() = 0;

	virtual void initWeaponSelection() = 0; // should reset at least bWeaponsReady
	virtual void doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v) = 0;

	// simulation
	virtual void startGame() {}
	virtual void getInput() = 0;
    virtual void clearInput() {}
	
	virtual void onRespawn() {}
	
	
	// ------------------------------------------------------
	// ----------------- Gusanos ----------------------------
	// ----------------- CWormInputHandler -------------------------
	
public:
	
	enum BaseActions
	{
		LEFT = 0,
		RIGHT,
		//UP,
		//DOWN,
		FIRE,
		JUMP,
		//CHANGE, // Probably useless Action
		NINJAROPE,
		DIG,
		RESPAWN,
		//
		ACTION_COUNT,
	};
	
	enum NetEvents
	{
		SYNC = 0,
		ACTION_STOP,
		ACTION_START,
		SELECT_WEAPONS,
		LuaEvent,
		//
		EVENT_COUNT,
	};
	
	enum ReplicationItems
	{
		WormID,
		Other
	};
	
	struct Stats
	{
		Stats()
		: deaths(0), kills(0)
		{
		}
		
		~Stats();
		
		int deaths;
		int kills;
		LuaReference luaData;
	};
		
	// ClassID is Used by zoidcom to identify the class over the network,
	// do not confuse with the node ID which identifies instances of the class.
	static Net_ClassID  classID;

protected:
	virtual void OlxInputToGusEvents();
	virtual ~CWormInputHandler();
	
private:
	void gusInit(CWorm* w);
	
public:
	void think();
	// subThink() gets called inside think() and its used to give the derivations
	// the ability to think without replacing the main CWormInputHandler::think().
	virtual void subThink() = 0;
#ifndef DEDICATED_ONLY
	virtual void render() {}
#endif
	
	void assignNetworkRole();
	
	void assignWorm(CWorm* worm);
	
	void sendSyncMessage( Net_ConnID id ); // Its the initializing message that is sent to new clients that recieve the node.
	
	void nameChangePetition(); // Asks the server to change the name to the one in the player options.
	
	void baseActionStart( BaseActions action );
	void baseActionStop( BaseActions action );
	
	void addKill();
	void addDeath();
	
	Net_NodeID getNodeID();
	Net_ConnID getConnectionID();
	void sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, BitStream* userdata, Net_ConnID connID);
	CWorm* getWorm() { return m_worm; }
	
	static LuaReference metaTable;
	virtual LuaReference getMetaTable() const { return metaTable; }
	virtual void deleteThis();
	
	shared_ptr<Stats> stats;
	
	bool deleteMe;
	
/*	std::string m_name;
	int colour;
	int team;*/
	bool local;
	
	LuaReference luaData;
	
	void selectWeapons( std::vector< WeaponType* > const& weaps );
	
	Net_Node* getNode() { return m_node; }
	
protected:
	void addEvent(BitStream* data, NetEvents event);
	void addActionStart(BitStream* data, BaseActions action);
	void addActionStop(BitStream* data, BaseActions action);
	
	Net_Node *m_node;
	BasePlayerInterceptor* m_interceptor;
		
};


class BasePlayerInterceptor : public Net_NodeReplicationInterceptor
{
public:
	BasePlayerInterceptor( CWormInputHandler* parent );

	bool inPreUpdateItem (Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator);

	// Not used virtual stuff
	void outPreReplicateNode(Net_Node *_node, eNet_NodeRole _remote_role) {}
	bool outPreUpdateItem (Net_Node *_node, eNet_NodeRole _remote_role, Net_Replicator *_replicator) { return true; }
	bool outPreUpdate (Net_Node* node, eNet_NodeRole remote_role) { return true; }

	virtual ~BasePlayerInterceptor()
	{}
private:
	CWormInputHandler* m_parent;
};


#endif

