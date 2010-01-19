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
#include <SDL.h> // for uint8_t etc.
#include <vector>
#include "Utils.h"
#include "SmartPointer.h"

typedef float Net_Float;
typedef uint8_t Net_U8;
typedef uint32_t Net_U32;
typedef int32_t Net_S32;

typedef uint32_t Net_ClassID;
typedef uint32_t Net_ConnID;
typedef uint32_t Net_NodeID;
typedef uint8_t Net_InterceptID;
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
	Net_REPRULE_AUTH_2_PROXY = 1,
	Net_REPRULE_AUTH_2_OWNER = 2,
	Net_REPRULE_AUTH_2_ALL = Net_REPRULE_AUTH_2_PROXY | Net_REPRULE_AUTH_2_OWNER,
	Net_REPRULE_OWNER_2_AUTH = 4,
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

enum eNet_Event {
	eNet_EventUser,
	eNet_EventInit,
	eNet_EventRemoved
};


static const Net_NodeID UNIQUE_NODE_ID = 1; // unique nodes are local-only nodes
static const Net_NodeID INVALID_NODE_ID = 0;
static const Net_ClassID INVALID_CLASS_ID = 0;
static const Net_ConnID INVALID_CONN_ID = 0;


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


class Net_BitStream {
private:
	std::vector<bool> m_data;
	size_t m_readPos;  // in bits

	void growIfNeeded(size_t addBits);
	void writeBits(const std::vector<bool>& bits);
	std::vector<bool> readBits(size_t bitCount);
	void reset();

	bool testBool();
	bool testInt();
	bool testFloat();
	bool testStream();
	bool testString();
	bool testSafety();
public:
	Net_BitStream() : m_readPos(0) {}
	Net_BitStream(const std::string& rawdata);

	void addBool(bool);
	void addInt(int n, int bits);
	void addSignedInt(int n, int bits);
	void addFloat(float f, int bits);
	void addBitStream(const Net_BitStream& str);
	void addString(const std::string&);
	
	bool getBool();
	int getInt(int bits);
	int getSignedInt(int bits);
	float getFloat(int bits);
	std::string getString();
	
	Net_BitStream* Duplicate();
	bool runTests();
	
	const std::vector<bool>& data() const { return m_data; }
	void resetPos() { m_readPos = 0; }
	void setBitPos(size_t p) { m_readPos = p; }
	void skipBits(size_t b) { m_readPos += b; }
	size_t bitPos() const { return m_readPos; }
	size_t bitSize() const { return m_data.size(); }
	size_t restBitSize() const { return m_data.size() - m_readPos; }
};

struct Net_FileTransInfo {
	std::string path;
	int bps;
	size_t transferred;
	size_t size;
};

class CServerConnection;
Net_ConnID NetConnID_server();
Net_ConnID NetConnID_conn(CServerConnection* cl);
CServerConnection* serverConnFromNetConnID(Net_ConnID id);
bool isServerNetConnID(Net_ConnID id);

struct Net_NodeReplicationInterceptor;
struct Net_Control;
struct Net_ReplicatorSetup;
struct Net_Replicator;
struct NetNodeIntern;
template <> void SmartPointer_ObjectDeinit<NetNodeIntern> ( NetNodeIntern * obj );

struct Net_Node : DontCopyTag {
	// we have that as smartptr because we may still refer to it (in packets queue) after Net_Node was deleted
	SmartPointer<NetNodeIntern> intern;
	Net_Node(); ~Net_Node();
	
	eNet_NodeRole getRole();
	void setOwner(Net_ConnID);
	void setAnnounceData(Net_BitStream*);	
	Net_NodeID getNetworkID();
	
	bool isNodeRegistered();
	bool areWeOwner();
	Net_ConnID getOwner();
	
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
	

	void beginReplicationSetup();
	void addReplicator(Net_Replicator*, bool autodelete);	
	void addReplicationInt(Net_S32*, int bits, bool, Net_RepFlags, Net_RepRules, Net_InterceptID id = 0, int p2 = 0, int p3 = 0);
	void addReplicationFloat(Net_Float*, int bits, Net_RepFlags, Net_RepRules, Net_InterceptID id = 0, int p2 = 0, int p3 = 0);
	void endReplicationSetup();

	void setInterceptID(Net_InterceptID);
	void setReplicationInterceptor(Net_NodeReplicationInterceptor*);

};

class CBytestream;
struct NetControlIntern;
template <> void SmartPointer_ObjectDeinit<NetControlIntern> ( NetControlIntern * obj );

struct Net_Control : DontCopyTag {
	// we have that as smartptr because we may still refer to it after Net_Control was deleted
	SmartPointer<NetControlIntern> intern;
	
	Net_Control(bool isServer);
	virtual ~Net_Control();

	void Shutdown();
	void Net_ConnectToServer();
	void Net_disconnectAll(Net_BitStream*);
	void Net_Disconnect(Net_ConnID id, Net_BitStream*);
			
	void Net_setControlID(int);
	void Net_setDebugName(const std::string&);
	void Net_requestNetMode(Net_ConnID, int);

	void olxSend(bool sendPendingOnly);
	void olxParse(Net_ConnID src, CBytestream& bs);
	void olxHandleClientDisconnect(Net_ConnID cl);
	
	void Net_processOutput();
	void Net_processInput();
	
	void Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode);
	Net_ClassID Net_registerClass(const std::string& classname, Net_ClassFlags);
		
	// ------- virtual callbacks -----------
		
	virtual void Net_cbDataReceived( Net_ConnID id, Net_BitStream &data) = 0;
	
	
	// server wants to tell us about new node
	virtual void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id ) = 0;
	
	// called when incoming connection has been established
	virtual void Net_cbConnectionSpawned( Net_ConnID _id ) = 0;
	// called when a connection closed
	virtual void Net_cbConnectionClosed( Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata ) = 0;
	// called when we got connected to server
	virtual void Net_cbConnectResult( eNet_ConnectResult res ) = 0;
	
};

struct Net_Replicator {
	uint8_t m_flags;
	Net_ReplicatorSetup* setup;
	void* ptr;
	Net_BitStream* peekStream;
	
	virtual ~Net_Replicator() {}
	virtual Net_Replicator* Duplicate(Net_Replicator *_dest) = 0;

	Net_Replicator(Net_ReplicatorSetup* s) : m_flags(0), setup(s), ptr(NULL), peekStream(NULL) {}
	Net_ReplicatorSetup* getSetup() const { return setup; }
	
	Net_BitStream* getPeekStream() { return peekStream; }
	void* peekDataRetrieve() { return ptr; }
	void peekDataStore(void* p) { if(ptr) clearPeekData(); ptr = p; }
	
	virtual void* peekData() = 0;
	virtual void clearPeekData() = 0;
};

struct Net_ReplicatorBasic : Net_Replicator {
	Net_ReplicatorBasic(Net_ReplicatorSetup* s) : Net_Replicator(s) {}
	
	virtual bool checkState() = 0; // true if update is needed
	virtual void packData(Net_BitStream *_stream) = 0;
	virtual void unpackData(Net_BitStream *_stream, bool _store) = 0;	
};



struct Net_ReplicatorSetup {
	Net_RepFlags repFlags;
	Net_RepRules repRules;
	Net_InterceptID interceptId;
	Net_ReplicatorSetup(Net_RepFlags, Net_RepRules, Net_InterceptID id = 0, int p2 = 0, int p3 = 0);
	Net_InterceptID getInterceptID() { return interceptId; }
};

struct Net_NodeReplicationInterceptor {
	virtual ~Net_NodeReplicationInterceptor() {}
	
	virtual void outPreReplicateNode(Net_Node *_node, eNet_NodeRole _remote_role) = 0;
	virtual bool outPreUpdateItem (Net_Node* node, eNet_NodeRole remote_role, Net_Replicator* replicator) = 0;
	virtual bool inPreUpdateItem (Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator) = 0;
};

#endif

