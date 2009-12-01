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

typedef float Net_Float;
typedef uint8_t Net_U8;
typedef uint32_t Net_U32;
typedef int32_t Net_S32;

typedef uint32_t Net_ClassID;
typedef uint32_t Net_ConnID;
typedef uint32_t Net_NodeID;
typedef uint32_t Net_InterceptID;
typedef uint32_t Net_FileTransID;
	
enum eNet_NodeRole {
	eNet_RoleUndefined,
	eNet_RoleOwner,
	
	eNet_RoleAuthority,
	eNet_RoleProxy
};
enum eNet_SendMode {
	eNet_ReliableOrdered,
	eNet_Reliable,
	eNet_ReliableUnordered,
	eNet_Unreliable
};

enum {
	Net_REPLICATOR_INITIALIZED
};

typedef Net_U8 Net_RepRules;
enum Net_RepRule {
	Net_REPRULE_NONE = 0,
	Net_REPRULE_AUTH_2_ALL = 1,
	Net_REPRULE_OWNER_2_AUTH = 2,
	Net_REPRULE_AUTH_2_PROXY = 4,
	Net_REPRULE_AUTH_2_OWNER = 8,
};

typedef Net_U8 Net_RepFlags;
enum Net_RepFlag {
	Net_REPFLAG_MOSTRECENT = 1,
	Net_REPFLAG_INTERCEPT = 2,
	Net_REPFLAG_RARELYCHANGED = 4
};

typedef Net_U8 Net_ClassFlags;
enum {
	Net_CLASSFLAG_ANNOUNCEDATA = 1
};

enum {
	Net_FTRANS_ID_BITS = sizeof(Net_FileTransID) * 8
};

enum eNet_Event {
	eNet_EventUser,
	eNet_EventInit,
	eNet_EventRemoved,
	
	eNet_EventFile_Incoming,
	eNet_EventFile_Complete,
	eNet_EventFile_Data
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
	void addFloat(float f, int bits);
	void addBitStream(Net_BitStream* str);
	void addString(const std::string&);
	
	bool getBool();
	int getInt(int bits);
	int getSignedInt(int bits);
	float getFloat(int bits);
	const char* getStringStatic();
	
	Net_BitStream* Duplicate();
};

struct Net_FileTransInfo {
	std::string path;
	int bps;
	size_t transferred;
	size_t size;
};

struct Net_NodeReplicationInterceptor;
struct Net_Control;
struct Net_ReplicatorSetup;
struct Net_ReplicatorBasic;

struct Net_Node {
	eNet_NodeRole getRole();
	void setOwner(Net_ConnID, bool something);
	void setAnnounceData(Net_BitStream*);	
	Net_NodeID getNetworkID();
	
	bool registerNodeUnique(Net_ClassID, eNet_NodeRole, Net_Control*);
	bool registerNodeDynamic(Net_ClassID, Net_Control*);
	bool registerRequestedNode(Net_ClassID, Net_Control*);
	
	void applyForNetLevel(int something);
	void removeFromNetLevel(int something);
	
	
	void setEventNotification(bool,bool); // TODO: true,false -> enables eEvent_Init
	void sendEvent(eNet_SendMode, Net_RepRules rules, Net_BitStream*);
	void sendEventDirect(eNet_SendMode, Net_BitStream*, Net_ConnID);
	bool checkEventWaiting();
	Net_BitStream* getNextEvent(eNet_Event*, eNet_NodeRole*, Net_ConnID*);
	

	void addReplicator(Net_ReplicatorBasic*, bool);	
	void beginReplicationSetup(int something = 0);
	void setInterceptID(Net_InterceptID);
	void addReplicationInt(Net_S32*, int bits, bool, Net_RepFlags, Net_RepRules, int p1 = 0, int p2 = 0, int p3 = 0);
	void addReplicationFloat(Net_Float*, int bits, Net_RepFlags, Net_RepRules, int p1 = 0, int p2 = 0, int p3 = 0);
	void endReplicationSetup();
	void setReplicationInterceptor(Net_NodeReplicationInterceptor*);
	
	
	void acceptFile(Net_ConnID, Net_FileTransID, int, bool accept);
	Net_FileTransID sendFile(const char* filename, int, Net_ConnID, int, float);
	Net_FileTransInfo& getFileInfo(Net_ConnID, Net_FileTransID);
};

struct Net_ConnectionStats {
	int avg_ping;
};

enum Net_ProcessType { eNet_NoBlock };
struct Net_Address;

struct Net_Control {
	void Net_Connect(const Net_Address&, void*);
	void Shutdown();
	void Net_disconnectAll(Net_BitStream*);
	void Net_Disconnect(Net_ConnID id, Net_BitStream*);
	
	Net_Address* Net_getPeer(Net_ConnID);
		
	Net_BitStream* Net_createBitStream();

	void Net_processOutput();
	void Net_processInput(Net_ProcessType);
	
	void Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode);
	Net_ClassID Net_registerClass(const std::string& classname, Net_ClassFlags);
	
	Net_ConnectionStats Net_getConnectionStats(Net_ConnID);
};

struct Net_ReplicatorBasic {
	uint8_t m_flags;

	Net_ReplicatorBasic(Net_ReplicatorSetup*);
	
	Net_BitStream* getPeekStream();
	void* peekDataRetrieve();

	// replicator
	Net_ReplicatorSetup* getSetup();	
	void* peekData();
	void peekDataStore(void*);
};

#define Net_Replicator Net_ReplicatorBasic


struct Net_ReplicatorSetup {
	Net_ReplicatorSetup(Net_RepFlags, Net_RepRules, int p1 = 0, int p2 = 0, int p3 = 0);
	Net_InterceptID getInterceptID();
};

struct Net_NodeReplicationInterceptor {};

enum eNet_AddressType {
	eNet_AddressUDP
};

struct Net_Address {
	void setAddress(eNet_AddressType, int, const char*);
	Net_U32 getIP() const;
};

struct NetStream {
	NetStream();
	NetStream( void (*)( const char* ) );
	void setLogLevel(int);
	bool Init();
};


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

