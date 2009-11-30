/*
 *  netstream.h
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

// This is going to be a replacement for ZoidCom

#ifndef __GUS_NETSTREAM_H__
#define __GUS_NETSTREAM_H__

#include <string>
#include <stdint.h>

typedef uint8_t Net_U8;
typedef uint32_t Net_U32;
typedef int32_t Net_S32;

typedef uint32_t Net_ClassID;
typedef uint32_t Net_ConnID;
typedef uint32_t Net_NodeID;
typedef uint32_t Net_InterceptID;

enum eNet_NodeRole {
	eNet_RoleUndefined,
	eNet_RoleOwner,
	
	eNet_RoleAuthority,
	eNet_RoleProxy
};
enum eNet_SendMode {
	eNet_ReliableOrdered,
	eNet_Reliable
};

typedef uint32_t eNet_Event;

enum {
	Net_REPLICATOR_INITIALIZED
};

typedef Net_U8 Net_RepRules;
enum {
	Net_REPRULE_AUTH_2_ALL,
	Net_REPRULE_OWNER_2_AUTH,
	Net_REPRULE_AUTH_2_PROXY
};

typedef Net_U8 Net_RepFlags;
enum {
	Net_REPFLAG_MOSTRECENT,
	Net_REPFLAG_INTERCEPT,
	Net_REPFLAG_RARELYCHANGED
};

enum {
	eNet_EventUser,
	eNet_EventInit,
	eNet_EventRemoved
};

static const Net_ClassID Net_Invalid_ID = Net_ClassID(-1);

enum eNet_ConnectResult {
	eNet_ConnAccepted
};

enum eNet_CloseReason {
	eNet_ClosedDisconnect,
	eNet_ClosedTimeout,
	eNet_ClosedReconnect
};

enum eNet_NetResult {
	eNet_NetEnabled
};


struct Net_BitStream {
	void addBool(bool);
	void addInt(int n, int bits);
	void addSignedInt(int n, int bits);
	void addBitStream(Net_BitStream* str);
	void addString(const std::string&);
	
	int getInt(int bits);
	int getSignedInt(int bits);
	const char* getStringStatic();
	
};

struct Net_NodeReplicationInterceptor;
struct Net_Control;
struct Net_ReplicatorSetup;

struct Net_Node {
	eNet_NodeRole getRole();
	
	bool checkEventWaiting();
	Net_BitStream* getNextEvent(eNet_Event*, eNet_NodeRole*, Net_ConnID*);
	
	void sendEvent(eNet_SendMode, Net_RepRules rules, Net_BitStream*);
	void sendEventDirect(eNet_SendMode, Net_BitStream*, Net_ConnID);

	void beginReplicationSetup(int something = 0);
	void setInterceptID(Net_InterceptID);
	void addReplicationInt(Net_S32*, int bits, bool, Net_RepFlags, Net_RepRules, Net_NodeID);
	void endReplicationSetup();
	void setReplicationInterceptor(Net_NodeReplicationInterceptor*);
	
	void setEventNotification(bool,bool); // TODO: true,false -> enables eEvent_Init
	
	bool registerNodeUnique(Net_ClassID, eNet_NodeRole, Net_Control*);
	bool registerNodeDynamic(Net_ClassID, Net_Control*);
	bool registerRequestedNode(Net_ClassID, Net_Control*);
	
	void applyForNetLevel(int something);
	
	void setOwner(Net_ConnID, bool something);
	
	Net_NodeID getNetworkID();
};

struct Net_Control {
	Net_BitStream* Net_createBitStream();

	void Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode);

};

struct Net_ReplicatorBasic {
	uint8_t m_flags;
	Net_BitStream* getPeekStream();
	
	void* peekDataRetrieve();
};

struct Net_Replicator : Net_ReplicatorBasic {
	void* peekData();

	Net_ReplicatorSetup* getSetup();
};

struct Net_ReplicatorSetup {
	Net_InterceptID getInterceptID();
};

struct Net_NodeReplicationInterceptor {};

struct Net_Address {};



void Net_simulateLag(int,int);
void Net_simulateLoss(int,int);

bool Net_initSockets(bool, int port, int, int);

void Net_setControlID(int);
void Net_setDebugName(const std::string&);
void Net_setUpstreamLimit(int,int);

void Net_requestDownstreamLimit(Net_ConnID, int,int);
void Net_requestNetMode(Net_ConnID, int);

void Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode);

#endif

