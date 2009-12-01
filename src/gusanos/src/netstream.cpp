/*
 *  netstream.cpp
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

#include "netstream.h"
#include "Networking.h"



void Net_BitStream::addBool(bool) {}
void Net_BitStream::addInt(int n, int bits) {}
void Net_BitStream::addSignedInt(int n, int bits) {}
void Net_BitStream::addFloat(float f, int bits) {}
void Net_BitStream::addBitStream(Net_BitStream* str) {}
void Net_BitStream::addString(const std::string&) {}

bool Net_BitStream::getBool() { return false; }
int Net_BitStream::getInt(int bits) { return 0; }
int Net_BitStream::getSignedInt(int bits) { return 0; }
float Net_BitStream::getFloat(int bits) { return 0.0f; }
const char* Net_BitStream::getStringStatic() { return ""; }

Net_BitStream* Net_BitStream::Duplicate() { return NULL; }



eNet_NodeRole Net_Node::getRole() {}
void Net_Node::setOwner(Net_ConnID, bool something) {}
void Net_Node::setAnnounceData(Net_BitStream*) {}
Net_NodeID Net_Node::getNetworkID() {}

bool Net_Node::registerNodeUnique(Net_ClassID, eNet_NodeRole, Net_Control*) {}
bool Net_Node::registerNodeDynamic(Net_ClassID, Net_Control*) {}
bool Net_Node::registerRequestedNode(Net_ClassID, Net_Control*) {}

void Net_Node::applyForNetLevel(int something) {}
void Net_Node::removeFromNetLevel(int something) {}


void Net_Node::setEventNotification(bool,bool) {} // TODO: true,false -> enables eEvent_Init
void Net_Node::sendEvent(eNet_SendMode, Net_RepRules rules, Net_BitStream*) {}
void Net_Node::sendEventDirect(eNet_SendMode, Net_BitStream*, Net_ConnID) {}
bool Net_Node::checkEventWaiting() {}
Net_BitStream* Net_Node::getNextEvent(eNet_Event*, eNet_NodeRole*, Net_ConnID*) {}


void Net_Node::addReplicator(Net_ReplicatorBasic*, bool) {}	
void Net_Node::beginReplicationSetup(int something) {}
void Net_Node::setInterceptID(Net_InterceptID) {}
void Net_Node::addReplicationInt(Net_S32*, int bits, bool, Net_RepFlags, Net_RepRules, int p1, int p2, int p3) {}
void Net_Node::addReplicationFloat(Net_Float*, int bits, Net_RepFlags, Net_RepRules, int p1, int p2, int p3) {}
void Net_Node::endReplicationSetup() {}
void Net_Node::setReplicationInterceptor(Net_NodeReplicationInterceptor*) {}


void Net_Node::acceptFile(Net_ConnID, Net_FileTransID, int, bool accept) {}
Net_FileTransID Net_Node::sendFile(const char* filename, int, Net_ConnID, int, float) {}
Net_FileTransInfo& Net_Node::getFileInfo(Net_ConnID, Net_FileTransID) {}


void Net_Control::Net_Connect(const Net_Address&, void*) {}
void Net_Control::Shutdown() {}
void Net_Control::Net_disconnectAll(Net_BitStream*) {}
void Net_Control::Net_Disconnect(Net_ConnID id, Net_BitStream*) {}

Net_Address* Net_Control::Net_getPeer(Net_ConnID) {}

Net_BitStream* Net_Control::Net_createBitStream() {}

void Net_Control::Net_processOutput() {}
void Net_Control::Net_processInput(Net_ProcessType) {}

void Net_Control::Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode) {}
Net_ClassID Net_Control::Net_registerClass(const std::string& classname, Net_ClassFlags) {}

Net_ConnectionStats Net_Control::Net_getConnectionStats(Net_ConnID) {}

Net_ReplicatorBasic::Net_ReplicatorBasic(Net_ReplicatorSetup*) {}

Net_BitStream* Net_ReplicatorBasic::getPeekStream() {}
void* Net_ReplicatorBasic::peekDataRetrieve() {}

Net_ReplicatorSetup* Net_ReplicatorBasic::getSetup() {}	
void* Net_ReplicatorBasic::peekData() {}
void Net_ReplicatorBasic::peekDataStore(void*) {}



Net_ReplicatorSetup::Net_ReplicatorSetup(Net_RepFlags, Net_RepRules, int p1, int p2, int p3) {}
Net_InterceptID Net_ReplicatorSetup::getInterceptID() {}


void Net_Address::setAddress(eNet_AddressType, int, const char*) {}
Net_U32 Net_Address::getIP() const {}


NetStream::NetStream() {}
NetStream::NetStream( void (*)( const char* ) ) {}
void NetStream::setLogLevel(int) {}
bool NetStream::Init() {}




void Net_simulateLag(int,int) {}
void Net_simulateLoss(int,int) {}

bool Net_initSockets(bool, int port, int, int) {}

void Net_setControlID(int) {}
void Net_setDebugName(const std::string&) {}
void Net_setUpstreamLimit(int,int) {}

void Net_requestDownstreamLimit(Net_ConnID, int,int) {}
void Net_requestNetMode(Net_ConnID, int) {}

void Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode) {}

