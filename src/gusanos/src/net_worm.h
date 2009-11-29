#ifndef NET_WORM_H
#define NET_WORM_H

//#include "vec.h"
//#include "base_object.h"
#include "base_worm.h"
//#include "sprite.h"

#include <zoidcom.h>

class NetWormInterceptor;

class NetWorm : public BaseWorm
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
		
	static ZCom_ClassID  classID;
	static const float MAX_ERROR_RADIUS = 10;
		
	NetWorm(bool isAuthority);
	~NetWorm();

	void think();
	void correctOwnerPosition();

	void assignOwner( BasePlayer* owner);
	void setOwnerId( ZCom_ConnID _id );
	void sendSyncMessage( ZCom_ConnID id );
	void sendWeaponMessage( int index, ZCom_BitStream* data, zU8 repRules = ZCOM_REPRULE_AUTH_2_ALL );
	
	eZCom_NodeRole getRole()
	{
		if ( m_node )
		{
			return m_node->getRole();
		}else
			return eZCom_RoleUndefined;
	}
	
	virtual void sendLuaEvent(LuaEventDef* event, eZCom_SendMode mode, zU8 rules, ZCom_BitStream* userdata, ZCom_ConnID connID);
	
	ZCom_NodeID getNodeID();
	
	void respawn();
	void dig();
	void die();
	void changeWeaponTo( unsigned int weapIndex );
	void damage( float amount, BasePlayer* damager );
	void setWeapon(size_t index, WeaponType* type );
	void clearWeapons();
	
	//virtual void deleteThis();
	virtual void finalize();
	
	Vec lastPosUpdate;
	int timeSinceLastUpdate;
	
private:
	
	void addEvent(ZCom_BitStream* data, NetEvents event);
		
	bool m_isAuthority;
	ZCom_Node *m_node;
	NetWormInterceptor* m_interceptor;
	ZCom_NodeID m_playerID; // The id of the owner player node to replicate to all proxys
};

class NetWormInterceptor : public ZCom_NodeReplicationInterceptor
{
public:
	NetWormInterceptor( NetWorm* parent );

	bool inPreUpdateItem (ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent);

	// Not used virtual stuff
	void outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {}
	void outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {}
	bool outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) { return true; }
	bool outPreUpdateItem (ZCom_Node* node, ZCom_ConnID from, eZCom_NodeRole remote_role, ZCom_Replicator* replicator);
	void outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {}
	bool inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role) { return true; }
	void inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {};

	virtual ~NetWormInterceptor()
	{}
private:
	NetWorm* m_parent;
};

#endif  // _WORM_H_
