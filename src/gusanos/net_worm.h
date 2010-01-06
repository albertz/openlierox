#ifndef NET_WORM_H
#define NET_WORM_H

//#include "vec.h"
//#include "CGameObject.h"
#include "CWorm.h"
//#include "sprite.h"

#include "netstream.h"

class NetWormInterceptor : public Net_NodeReplicationInterceptor
{
public:
	NetWormInterceptor( CWorm* parent );

	bool inPreUpdateItem (Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator);

	// Not used virtual stuff
	void outPreReplicateNode(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role) {}
	bool outPreUpdate(Net_Node *_node, Net_ConnID _to, eNet_NodeRole _remote_role) { return true; }
	bool outPreUpdateItem (Net_Node* node, Net_ConnID from, eNet_NodeRole remote_role, Net_Replicator* replicator);

	virtual ~NetWormInterceptor()
	{}
private:
	CWorm* m_parent;
};

#endif  // _WORM_H_
