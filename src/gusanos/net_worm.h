#ifndef NET_WORM_H
#define NET_WORM_H

//#include "vec.h"
//#include "CGameObject.h"
#include "CWorm.h"
//#include "sprite.h"

#include "netstream.h"

class NetWormInterceptor;

class NetWorm : public CWorm
{	
public:
	friend class NetWormInterceptor;
		
	enum NetEvents
	{
		PosCorrection = 0,
		Respawn,
		Dig,
		Die,
		ChangeWeapon,
		WeaponMessage,
		SetWeapon,
		ClearWeapons,
		SYNC,
		LuaEvent,
		EVENT_COUNT,
	};
		
	enum ReplicationItems
	{
		PlayerID = 0,
		Position,
		AIM
	};
		
	static Net_ClassID  classID;
	static const float MAX_ERROR_RADIUS = 10;
		
	NetWorm(bool isAuthority);
	~NetWorm();

	void think();
	void correctOwnerPosition();

	void assignOwner( CWormInputHandler* owner);
	void setOwnerId( Net_ConnID _id );
	void sendSyncMessage( Net_ConnID id );
	void sendWeaponMessage( int index, Net_BitStream* data, Net_U8 repRules = Net_REPRULE_AUTH_2_ALL );
	
	eNet_NodeRole getRole()
	{
		if ( m_node )
		{
			return m_node->getRole();
		}else
			return eNet_RoleUndefined;
	}
	
	virtual void sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, Net_BitStream* userdata, Net_ConnID connID);
	
	Net_NodeID getNodeID();
	
	void respawn();
	void dig();
	void die();
	void changeWeaponTo( unsigned int weapIndex );
	void damage( float amount, CWormInputHandler* damager );
	void setWeapon(size_t index, WeaponType* type );
	void clearWeapons();
	
	//virtual void deleteThis();
	virtual void finalize();
	
	Vec lastPosUpdate;
	int timeSinceLastUpdate;
	
private:
	
	void addEvent(Net_BitStream* data, NetEvents event);
		
	bool m_isAuthority;
	Net_Node *m_node;
	NetWormInterceptor* m_interceptor;
	Net_NodeID m_playerID; // The id of the owner player node to replicate to all proxys
};

class NetWormInterceptor : public Net_NodeReplicationInterceptor
{
public:
	NetWormInterceptor( NetWorm* parent );

	bool inPreUpdateItem (Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator, Net_U32 _estimated_time_sent);

	// Not used virtual stuff
	void outPreReplicateNode(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role) {}
	void outPreDereplicateNode(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role) {}
	bool outPreUpdate(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role) { return true; }
	bool outPreUpdateItem (Net_Node* node, Net_ConnID from, eNet_NodeRole remote_role, Net_Replicator* replicator);
	void outPostUpdate(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role, Net_U32 _rep_bits, Net_U32 _event_bits, Net_U32 _meta_bits) {}
	bool inPreUpdate(Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role) { return true; }
	void inPostUpdate(Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_U32 _rep_bits, Net_U32 _event_bits, Net_U32 _meta_bits) {};

	virtual ~NetWormInterceptor()
	{}
private:
	NetWorm* m_parent;
};

#endif  // _WORM_H_
