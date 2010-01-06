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
#include "EndianSwap.h"
#include "gusanos/encoding.h"
#include "CBytestream.h"

#include "Protocol.h"
#include "CServer.h"
#include "CServerConnection.h"
#include "CClient.h"
#include "CServerNetEngine.h"
#include "CChannel.h"


// Grows the bit stream if the number of bits that are going to be added exceeds the buffer size
void Net_BitStream::growIfNeeded(size_t addBits)
{
	if (m_size + addBits >= m_data.size() * 8)  {
		size_t growsize = (addBits + 7) / 8;
		while (growsize--)
			m_data += '\0';
	}
}

static const unsigned char bitMasks[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

void Net_BitStream::writeBits(const char *bits, size_t bitCount)
{
	if(bitCount == 0) return;
	
	growIfNeeded(bitCount);

	size_t byteCount = (bitCount + 7) / 8;  // Number of bytes that will be needed
	unsigned char shift = m_size % 8;  // Bit index in m_data (result) array
	unsigned char leftIndex = (7 - (bitCount - 1) % 8);  // Bit index in the source array (bits)
	for (size_t i = 0; i < byteCount - 1; i++)  {
		// In case the byte in the source array is split in two real bytes, we have to read & merge them
		unsigned char byte = (bits[i] << leftIndex) | (bits[i + 1] >> (7 - leftIndex));
		m_data[m_size / 8]		|= byte >> shift;
		m_data[m_size / 8 + 1]	|= byte << (8 - shift);
		m_size += 8;
	}

	// The last few bits
	// bitRest is always < 8
	// Does the same thing as the above loop, only has to check if the
	// bits are split in more bytes
	size_t bitRest = bitCount - (byteCount - 1) * 8;
	unsigned char byte = bits[byteCount - 1];
	byte = byte << leftIndex;
	m_data[m_size / 8] |= byte >> shift;
	if (shift + bitRest > 8)
		m_data[m_size / 8 + 1] |= byte << (8 - shift);
	m_size += bitRest;
}

std::string Net_BitStream::readBits(size_t bitCount)
{
	if(bitCount == 0) return "";
	
	// Check
	if (m_readPos + bitCount >= m_size)
		return "";

	// Allocate space for the result
	std::string res;
	const size_t byteCount = (bitCount + 7) / 8;
	res.reserve(byteCount);
	for (size_t i = 0; i < byteCount; i++)
		res += '\0';

	const unsigned char leftIndex = (7 - (bitCount - 1) % 8);  // Bit index in the result array
	const unsigned char shift = m_readPos % 8;  // Bit index in the source (m_data) array

	for (size_t i = 0; i < byteCount - 1; i++)  {
		// In case the one resulting byte is split in two bytes in the array, we have to read two bytes
		// and shift & merge them accordingly
		unsigned char byte1 = m_data[m_readPos / 8];
		byte1 <<= shift;
		unsigned char byte2 = m_data[m_readPos / 8 + 1];
		byte2 >>= shift;
		unsigned char b = byte1 | byte2;
		res[i] |= b >> leftIndex;
		m_readPos += 8;
	}

	// Last few bits
	// The bit rest is the number of bits that is still left to be read
	// It is always less than 8
	// If the last few bits are split in two bytes, the next byte is read and both read
	// bytes are shifted & merged accordingly
	size_t bitRest = bitCount - (byteCount - 1) * 8;
	unsigned char byte = m_data[m_readPos / 8];
	byte <<= shift;
	if (shift + bitRest + 1 > 8)  {
		unsigned char b2 = m_data[m_readPos / 8 + 1];
		b2 >>= shift;
		res[byteCount - 1] = (byte | b2) >> leftIndex;
	} else
		res[byteCount - 1] = (byte >> leftIndex);
	m_readPos += bitRest;
	

	return res;
}

void Net_BitStream::addBool(bool b) {
	const char val = (char)b;
	writeBits(&val, 1);
}

void Net_BitStream::addInt(int n, int bits) {
	assert(bits >= 0 && bits < 33);
	union  {
		char bytes[4];
		int n;
	} data;
	data.n = n;
	BEndianSwap(data.n);
	writeBits(&data.bytes[(32 - bits) / 8], (size_t)bits);
}

void Net_BitStream::addSignedInt(int n, int bits) {
	addInt(n, bits);
}

void Net_BitStream::addFloat(float f, int bits) {
	assert(bits >= 0 && bits < 33);
	union  {
		char bytes[4];
		float f;
	} data;
	data.f = f;
	BEndianSwap(data.f);
	writeBits(data.bytes, (size_t)bits);
}

void Net_BitStream::addBitStream(Net_BitStream* str) {
	writeBits(str->m_data.data(), str->m_size);
}

void Net_BitStream::addString(const std::string& str) {
	while(m_readPos % 8 != 0) m_readPos++;
	writeBits(str.c_str(), (str.size() + 1) * 8);
}

bool Net_BitStream::getBool() { 
	std::string b = readBits(1);
	if (b.size())  {
		bool res = b[0] != 0;
		return res;
	}
	// TODO: throw an exception?
	return false;
}

int Net_BitStream::getInt(int bits) { 
	assert(bits >= 0 && bits < 33);
	std::string b = readBits(bits);
	if (b.size() * 8 < (size_t)bits)
		return 0; // TODO: throw an exception?
	union  {
		char bytes[4];
		int n;
	} data;
	memset(data.bytes, 0, sizeof(data.bytes));
	memcpy(&data.bytes[(32 - bits) / 8], b.data(), (bits + 7) / 8);
	BEndianSwap(data.n);
	return data.n;
}
int Net_BitStream::getSignedInt(int bits) { return getInt(bits); }
float Net_BitStream::getFloat(int bits) { 
	assert(bits >= 0 && bits < 33);
	std::string b = readBits(bits);
	if (b.size() * 8 < (size_t)bits)
		return 0; // TODO: throw an exception?

	union  {
		char bytes[4];
		float f;
	} data;
	memset(data.bytes, 0, sizeof(data.bytes));
	memcpy(data.bytes, b.data(), (bits + 7) / 8);
	BEndianSwap(data.f);
	return data.f;
}

const char* Net_BitStream::getStringStatic() { 
	while(m_readPos % 8 != 0) m_readPos++;
	const char* ret = &m_data[m_readPos / 8];
	while(true) {
		if(m_readPos >= m_size) return NULL;
		if(m_data[m_readPos / 8] == 0) break;
		m_readPos += 8;
	}
	m_readPos += 8;	
	return ret;
}

Net_BitStream* Net_BitStream::Duplicate() { 
	Net_BitStream *r = new Net_BitStream();
	r->m_data = m_data;
	r->m_size = m_size;
	r->m_readPos = m_readPos;
	return r;
}

void Net_BitStream::reset()
{
	m_readPos = 0;
	m_size = 0;
	m_data.clear();
}

//
// Net_BitStream Tests
//
bool Net_BitStream::testBool()
{
	reset();
	addBool(true);
	addBool(false);
	bool r1 = getBool();
	bool r2 = getBool();
	return r1 && !r2;
}

bool Net_BitStream::testInt()
{
	reset();
	for (int i = 0; i < 32; i++)
		addInt(1 << i, i + 1);
	for (int i = 0; i < 32; i++)
		if (getInt(i + 1) != (1 << i))
			return false;
	return true;
}

bool Net_BitStream::testFloat()
{
	reset();
	union T  {
		int n;
		float f;
	};

	for (int i = 0; i < 32; i++)  {
		T t; t.n = 1 << i;
		addFloat(t.f, i + 1);
	}
	for (int i = 0; i < 32; i++)  {
		T t; t.f = getFloat(i + 1);
		if (t.n != (1 << i))
			return false;
	}
	return true;
}

bool Net_BitStream::testStream()
{
	reset();
	Net_BitStream s2;
	s2.addBool(true);
	s2.addBool(false);
	s2.addInt(50, 32);
	s2.addInt(60, 16);
	s2.addInt(30, 5);
	addInt(14, 4);
	addBool(true);
	addBitStream(&s2);
	if (getInt(4) != 14)
		return false;
	if (getBool() != true)
		return false;
	if (getBool() != true)
		return false;
	if (getBool() != false)
		return false;
	if (getInt(32) != 50)
		return false;
	if (getInt(16) != 60)
		return false;
	if (getInt(5) != 30)
		return false;
	return true;
}

bool Net_BitStream::testSafety()
{
	try {
		reset();
		getBool();
		addInt(5, 5);
		getInt(6);
		addFloat(0.0f, 32);
		getFloat(31);
		getInt(5);
	} catch (...)  {
		return false;
	}
	return true;
}

bool Net_BitStream::testString()
{
	addBool(true);
	addString("some short text to test bitstream");
	if (getBool() != true)
		return false;
	if (std::string(getStringStatic()) != "some short text to test bitstream")
		return false;
	return true;
}

bool Net_BitStream::runTests()
{
	bool res = true;
	if (!testBool())  {
		printf("Boolean test failed!\n");
		res = false;
	}
	if (!testInt())  {
		printf("Integer test failed\n");
		res = false;
	}
	if (!testFloat())  {
		printf("Float test failed\n");
		res = false;
	}
	if (!testStream())  {
		printf("Stream test failed\n");
		res = false;
	}
	if (!testString())  {
		printf("String test failed\n");
		res = false;
	}
	if (!testSafety())  {
		printf("Safety test failed\n");
		res = false;
	}
	return res;
}









static const Net_NodeID UNIQUE_NODE_ID = 0; // unique nodes are local-only nodes
static const Net_NodeID INVALID_NODE_ID = Net_NodeID(-1);

struct Net_Control::NetControlIntern {
	bool isServer;
	int controlId;
	std::string debugName;
	bool cbNodeRequest_Dynamic;
	Net_NodeID cbNodeRequest_nodeId;
	
	struct DataPackage {
		enum Type {
			GPT_Direct = 0,
			GPT_NodeInit = 1,
			GPT_NodeRemove = 2,
			GPT_NodeUpdate = 3,
			GPT_NodeEvent = 4, /* eNet_EventUser */
		};

		Type type;
		Net_ConnID connID; // target or source
		Net_NodeID nodeID; // node this is about; invalid if type==direct
		Net_BitStream data;
		eNet_SendMode sendMode;
		
		DataPackage() : sendMode(eNet_ReliableOrdered) {}
		void send(CBytestream& bs);
		void read(CBytestream& bs);
	};
		
	typedef std::list<DataPackage> Packages;
	Packages packetsToSend;
	Packages packetsReceived;
	
	struct Class {
		std::string name;
		Net_ClassID id;
		Net_ClassFlags flags;

		Class() : id(Net_ClassID(-1)), flags(0) {}
		Class(const std::string& n, Net_ClassID i, Net_ClassFlags f) : name(n), id(i), flags(f) {}
	};

	typedef std::map<Net_ClassID, Class> Classes;
	Classes classes;
	
	typedef std::map<Net_NodeID, Net_Node*> Nodes;
	Nodes nodes;
	typedef std::set<Net_Node*> LocalNodes;
	LocalNodes localNodes;
	
	NetControlIntern() {
		isServer = false;
		controlId = 0;
		cbNodeRequest_Dynamic = false;
		cbNodeRequest_nodeId = 0;
	}
	
	DataPackage& pushPackageToSend() { packetsToSend.push_back(DataPackage()); return packetsToSend.back(); }
	
	Net_NodeID getUnusedNodeId() {
		if(nodes.size() == 0) return 1;
		return nodes.rbegin()->first + 1;
	}
	
	Net_Node* getNode(Net_NodeID id) {
		Nodes::iterator i = nodes.find(id);
		if(i != nodes.end()) return i->second;
		return NULL;
	}
	
	Class* getClass(Net_ClassID cid) {
		Classes::iterator i = classes.find(cid);
		if(i != classes.end()) return &i->second;
		return NULL;
	}
	
	std::string debugClassName(Net_ClassID cid) {
		Classes::iterator i = classes.find(cid);
		if(i == classes.end()) return "INVALID-CLASS(" + itoa(cid) + ")";
		return "Class(" + itoa(cid) + ":" + i->second.name + ")";
	}
};

struct Net_Node::NetNodeIntern {
	NetNodeIntern() :
	control(NULL), classId(Net_ClassID(-1)), nodeId(INVALID_NODE_ID), role(eNet_RoleUndefined),
	eventForInit(false), eventForRemove(false),
	forthcomingReplicatorInterceptID(0), interceptor(NULL) {}
	~NetNodeIntern() { clearReplicationSetup(); }

	Net_Control* control;	
	Net_ClassID classId;
	Net_NodeID nodeId;
	eNet_NodeRole role;
	bool eventForInit, eventForRemove;
	std::auto_ptr<Net_BitStream> announceData;
	
	typedef std::list< std::pair<Net_Replicator*,bool> > ReplicationSetup;
	ReplicationSetup replicationSetup;
	Net_InterceptID forthcomingReplicatorInterceptID;
	Net_NodeReplicationInterceptor* interceptor;
		
	void clearReplicationSetup() {
		for(ReplicationSetup::iterator i = replicationSetup.begin(); i != replicationSetup.end(); ++i)
			if(/* autodelete */ i->second)
				delete i->first;
		replicationSetup.clear();
	}
	
	struct Event {
		eNet_Event ev;
		eNet_NodeRole role;
		Net_ConnID cid;
		Net_BitStream stream;
		Event() : ev(eNet_Event(-1)), role(eNet_RoleUndefined), cid(0) {}
		Event(eNet_Event _ev, eNet_NodeRole _r, Net_ConnID _cid, const Net_BitStream& _s) : ev(_ev), role(_r), cid(_cid), stream(_s) {}
		static Event NodeInit(Net_ConnID c) { return Event(eNet_EventInit, eNet_RoleAuthority /* TODO? */, c, Net_BitStream()); }
		static Event NodeRemoved(Net_ConnID c) { return Event(eNet_EventRemoved, eNet_RoleAuthority /* TODO? */, c, Net_BitStream()); }
		static Event User(const Net_BitStream& s, Net_ConnID c) { return Event(eNet_EventUser, eNet_RoleAuthority /* TODO? */, c, s); }
	};
	
	typedef std::list<Event> Events;
	Events incomingEvents;
	Event curIncomingEvent;

	std::string debugStr() {
		if(!control) return "Node-no-control";
		if(nodeId == INVALID_NODE_ID) return "Node-invalid";
		if(classId == Net_ClassID(-1)) return "Node-no-ClassID";
		return "Node(" + itoa(nodeId) + "," + control->intern->debugClassName(classId) + ")";
	}
	
};


Net_Control::Net_Control(bool isServer) : intern(NULL) {
	intern = new NetControlIntern();
	intern->isServer = isServer;
}

Net_Control::~Net_Control() {
	delete intern;
	intern = NULL;
}

void Net_Control::Shutdown() {}
void Net_Control::Net_disconnectAll(Net_BitStream*) {}
void Net_Control::Net_Disconnect(Net_ConnID id, Net_BitStream*) {}


static void writeEliasGammaNr(CBytestream& bs, size_t n) {
	Net_BitStream bits;
	Encoding::encodeEliasGamma(bits, n + 1);
	bs.writeData(bits.data());
}

static size_t readEliasGammaNr(CBytestream& bs) {
	Net_BitStream bits(bs.data());
	bits.getInt(bs.GetPos() * 8); // skip to bs pos
	size_t len = Encoding::decodeEliasGamma(bits) - 1;
	bs.Skip( (bits.bitPos() + 7) / 8 - bs.GetPos() );
	return len;
}

void Net_Control::NetControlIntern::DataPackage::send(CBytestream& bs) {
	bs.writeByte(type);
	if(type != GPT_Direct) writeEliasGammaNr(bs, nodeID);
	writeEliasGammaNr(bs, data.data().size());
	bs.writeData(data.data());
}

void Net_Control::NetControlIntern::DataPackage::read(CBytestream& bs) {
	type = (NetControlIntern::DataPackage::Type) bs.readByte();
	nodeID = (type == GPT_Direct) ? INVALID_NODE_ID : readEliasGammaNr(bs);
	size_t len = readEliasGammaNr(bs);
	data = Net_BitStream( bs.getRawData( bs.GetPos(), len ) );
	bs.Skip(len);
}

void Net_Control::olxSend(bool sendPendingOnly) {
	if(intern->packetsToSend.size() == 0) return;
	
	CBytestream bs;
	bs.writeByte(intern->isServer ? (uchar)S2C_GUSANOS : (uchar)C2S_GUSANOS);

	writeEliasGammaNr(bs, intern->packetsToSend.size());
	for(NetControlIntern::Packages::iterator i = intern->packetsToSend.begin(); i != intern->packetsToSend.end(); ++i)
		i->send(bs);
	intern->packetsToSend.clear();
	
	if(tLX->iGameType == GME_JOIN)
		cClient->getChannel()->AddReliablePacketToSend(bs);
	else {
		// send to all except local client
		CServerConnection *cl = cServer->getClients();
		for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
			if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE) continue;
			if(cl->getNetEngine() == NULL) continue;
			
			if(!cl->isLocalClient())
				cl->getNetEngine()->SendPacket(&bs);
		}
	}
}

void Net_Control::olxParse(Net_ConnID src, CBytestream& bs) {
	size_t len = readEliasGammaNr(bs);

	for(size_t i = 0; i < len; ++i) {
		intern->packetsReceived.push_back(NetControlIntern::DataPackage());
		NetControlIntern::DataPackage& p = intern->packetsReceived.back();
		p.read(bs);
		p.connID = src;
	}
}

void Net_Control::Net_processOutput() {
	// goes through the nodes and push Node-updates as needed
	for(NetControlIntern::Nodes::iterator i = intern->nodes.begin(); i != intern->nodes.end(); ++i) {
		Net_Node::NetNodeIntern* node = i->second->intern;
		if(node->role == eNet_RoleAuthority || node->role == eNet_RoleOwner) {
			Net_ConnID cid = 0;
			eNet_NodeRole remoterole = eNet_RoleProxy;
			
			if(node->interceptor) {
				if(!node->interceptor->outPreUpdate(i->second, cid, remoterole))
					continue;
			}
			
			NetControlIntern::DataPackage p;
			p.connID = cid;
			p.sendMode = eNet_ReliableOrdered; // TODO ?
			p.type = NetControlIntern::DataPackage::GPT_NodeUpdate;
			
			size_t count = 0;
			
			for(Net_Node::NetNodeIntern::ReplicationSetup::iterator j = node->replicationSetup.begin(); j != node->replicationSetup.end(); ++j) {
				Net_ReplicatorBasic* replicator = dynamic_cast<Net_ReplicatorBasic*>(j->first);
				if(replicator == NULL) {
					errors << "Replicator is not a basic replicator" << endl;
					goto skipUpdateItem;
				}
				
				if(!replicator->checkState())
					goto skipUpdateItem;
				
				if(node->interceptor) {
					if(!node->interceptor->outPreUpdateItem(i->second, cid, remoterole, replicator))
						goto skipUpdateItem;
				}
				
				p.data.addBool(true); // mark that update item follows
				replicator->packData(&p.data);
				count++;
				continue;
				
			skipUpdateItem:
				p.data.addBool(false); // mark that update item is skipped
			}
			
			if(count > 0)
				intern->pushPackageToSend() = p;
		}
	}
}

static void doNodeUpdate(Net_Node* node, Net_BitStream& bs, Net_ConnID cid) {
	size_t i = 0;
	for(Net_Node::NetNodeIntern::ReplicationSetup::iterator j = node->intern->replicationSetup.begin(); j != node->intern->replicationSetup.end(); ++j, ++i) {
		if( /* skip mark */ !bs.getBool()) continue;
		
		Net_ReplicatorBasic* replicator = dynamic_cast<Net_ReplicatorBasic*>(j->first);
		if(replicator == NULL) {
			errors << "Replicator " << i << " in update of node " << node->intern->debugStr() << " is not a basic replicator" << endl;
			break; // nothing else we can do
		}
		
		bool store = true;
		if(node->intern->interceptor)
			store = node->intern->interceptor->inPreUpdateItem(node, cid, eNet_RoleAuthority, replicator);
		
		replicator->unpackData(&bs, store);
	}
}

static bool unregisterNode(Net_Node* node);

void Net_Control::Net_processInput() {
	for(NetControlIntern::Packages::iterator i = intern->packetsReceived.begin(); i != intern->packetsReceived.end(); ++i) {
		switch(i->type) {
			case NetControlIntern::DataPackage::GPT_Direct:
				Net_cbDataReceived(i->connID, i->data);
				break;
				
			case NetControlIntern::DataPackage::GPT_NodeInit: {
				Net_ClassID classId = i->data.getInt(32);
				NetControlIntern::Class* nodeClass = intern->getClass(classId);
				if(nodeClass == NULL) {
					warnings << "NodeInit for node " << i->nodeID << " with class " << classId << " failed because that class is unknown" << endl;
					break;
				}
				
				intern->cbNodeRequest_Dynamic = true;
				intern->cbNodeRequest_nodeId = i->nodeID;
				bool announce = nodeClass->flags & Net_CLASSFLAG_ANNOUNCEDATA;
				
				Net_cbNodeRequest_Dynamic(i->connID, classId, announce ? &i->data : NULL, eNet_RoleProxy, i->nodeID);
				if(intern->cbNodeRequest_Dynamic) { // we didnt created the node - otherwise this would be false
					errors << "Net_cbNodeRequest_Dynamic callback didnt created the node " << i->nodeID << " with class " << classId << endl;
					intern->cbNodeRequest_Dynamic = false; // reset anyway
					break;
				}

				Net_Node* node = intern->getNode(i->nodeID);
				if(node == NULL) {
					errors << "NodeInit: node " << i->nodeID << " not found after dynamic creation" << endl;
					break;
				}

				if(node->intern->eventForInit)
					node->intern->incomingEvents.push_back( Net_Node::NetNodeIntern::Event::NodeInit(i->connID) );
				
				break;
			}
				
			case NetControlIntern::DataPackage::GPT_NodeRemove: {
				Net_Node* node = intern->getNode(i->nodeID);
				if(node == NULL) {
					warnings << "NodeRemove: node " << i->nodeID << " not found" << endl;
					break;
				}
				
				if(node->intern->eventForRemove)
					node->intern->incomingEvents.push_back( Net_Node::NetNodeIntern::Event::NodeRemoved(i->connID) );
				
				unregisterNode(node);
				break;
			}
				
			case NetControlIntern::DataPackage::GPT_NodeUpdate: {
				Net_Node* node = intern->getNode(i->nodeID);
				if(node == NULL) {
					warnings << "NodeUpdate: node " << i->nodeID << " not found" << endl;
					break;
				}

				doNodeUpdate(node, i->data, i->connID);
				break;
			}
				
			case NetControlIntern::DataPackage::GPT_NodeEvent: {
				Net_Node* node = intern->getNode(i->nodeID);
				if(node == NULL) {
					warnings << "NodeUpdate: node " << i->nodeID << " not found" << endl;
					break;
				}
				
				node->intern->incomingEvents.push_back( Net_Node::NetNodeIntern::Event::User(i->data, i->connID) );
				break;
			}
				
			default:
				warnings << "invalid Gusanos data package type" << endl;
		}
	}
	
	intern->packetsReceived.clear();
}

void Net_Control::Net_sendData(Net_ConnID id, Net_BitStream* s, eNet_SendMode m) {
	NetControlIntern::DataPackage& p = intern->pushPackageToSend();
	p.connID = id;
	p.sendMode = m;
	p.type = NetControlIntern::DataPackage::GPT_Direct;
	p.data = *s;
	delete s;
}

Net_ClassID Net_Control::Net_registerClass(const std::string& classname, Net_ClassFlags flags) {
	Net_ClassID id = 0;
	if(intern->classes.size() > 0)
		id = intern->classes.rbegin()->first + 1;

	intern->classes[id] = NetControlIntern::Class(classname, id, flags);
	return id;
}


void Net_Control::Net_setControlID(int id) { intern->controlId = id; }
void Net_Control::Net_setDebugName(const std::string& n) { intern->debugName = n; }


void Net_Control::Net_requestNetMode(Net_ConnID, int) {
	// we silently ignore that because we dont have different netstream levels
}



static bool __unregisterNode(Net_Node* node) {
	if(node->intern->nodeId != INVALID_NODE_ID) {
		if(node->intern->control == NULL) {
			errors << "Net_Node::unregisterNode: node was a valid id but no reference to Net_Control" << endl;
			return false;
		}
		
		if(node->intern->nodeId != UNIQUE_NODE_ID) { // dynamic node
			Net_Control::NetControlIntern::Nodes& nodes = node->intern->control->intern->nodes;
			Net_Control::NetControlIntern::Nodes::iterator i = nodes.find(node->intern->nodeId);
			if(i == nodes.end()) {
				errors << "Net_Node::unregisterNode: dynamic node not found in node-list" << endl;
				return false;
			}
			
			nodes.erase(i);

			if(node->intern->role == eNet_RoleAuthority) {
				Net_Control::NetControlIntern::DataPackage& p = node->intern->control->intern->pushPackageToSend();
				p.connID = 0;
				p.sendMode = eNet_ReliableOrdered;
				p.type = Net_Control::NetControlIntern::DataPackage::GPT_NodeRemove;
				p.nodeID = node->intern->nodeId;
			}
		}
		
		else { // unique node
			Net_Control::NetControlIntern::LocalNodes& nodes = node->intern->control->intern->localNodes;
			Net_Control::NetControlIntern::LocalNodes::iterator i = nodes.find(node);
			if(i == nodes.end()) {
				errors << "Net_Node::unregisterNode: unique node not found in node-list" << endl;
				return false;
			}
			
			nodes.erase(i);
		}
		
		return true;
	}
	return false;
}

static bool unregisterNode(Net_Node* node) {
	bool ret = __unregisterNode(node);
	node->intern->nodeId = INVALID_NODE_ID;
	node->intern->classId = Net_ClassID(-1);
	node->intern->control = NULL;
	node->intern->role = eNet_RoleUndefined;
	return ret;
}

Net_Node::Net_Node() {
	intern = new NetNodeIntern();
}

Net_Node::~Net_Node() {
	unregisterNode(this);
	delete intern;
	intern = NULL;
}


eNet_NodeRole Net_Node::getRole() { return intern->role; }
void Net_Node::setOwner(Net_ConnID, bool something) {}
void Net_Node::setAnnounceData(Net_BitStream* s) { intern->announceData = std::auto_ptr<Net_BitStream>(s); }
Net_NodeID Net_Node::getNetworkID() { return intern->nodeId; }

static bool registerNode(Net_ClassID cid, Net_Node* node, Net_NodeID nid, eNet_NodeRole role, Net_Control* con) {
	if(node->intern->nodeId != INVALID_NODE_ID) {
		errors << "Net_Node::registerNode " << node->intern->debugStr() << " trying to register node twice" << endl;
		return false;
	}

	if(con == NULL) {
		errors << "Net_Node::registerNode without Net_Control" << endl;
		return false;
	}
	
	if(nid != UNIQUE_NODE_ID && con->intern->getNode(nid)) {
		errors << "Net_Node::registerNode " << node->intern->debugStr() << " node id " << nid << " was already taken by " << con->intern->getNode(nid)->intern->debugStr() << endl;
		return false;
	}
	
	node->intern->control = con;
	node->intern->classId = cid;
	node->intern->nodeId = nid;
	node->intern->role = role;
	
	if(nid == UNIQUE_NODE_ID)
		con->intern->localNodes.insert(node);
	else {
		con->intern->nodes[nid] = node;
	
		if(role == eNet_RoleAuthority) {
			Net_ConnID cid = 0;
			
			if(node->intern->interceptor)
				node->intern->interceptor->outPreReplicateNode(node, cid, role);

			Net_Control::NetControlIntern::DataPackage& p = con->intern->pushPackageToSend();
			p.connID = cid;
			p.sendMode = eNet_ReliableOrdered;
			p.type = Net_Control::NetControlIntern::DataPackage::GPT_NodeInit;
			p.nodeID = nid;
			p.data.addInt(cid, 32);
			
			bool announce = con->intern->classes[cid].flags & Net_CLASSFLAG_ANNOUNCEDATA;
			if(node->intern->announceData.get() && !announce)
				warnings << "node " << node->intern->debugStr() << " has announce data but class doesnt have flag set" << endl;
			else if(!node->intern->announceData.get() && announce)
				warnings << "node " << node->intern->debugStr() << " has no announce data but class requests it" << endl;
			else if(node->intern->announceData.get() && announce)
				p.data.addBitStream(node->intern->announceData.get());			
		}
	}
	
	notes << "Node " << nid << " registers with role " << role << " and class " << con->intern->classes[cid].name << endl;
	return true;
}

bool Net_Node::registerNodeUnique(Net_ClassID cid, eNet_NodeRole role, Net_Control* con) {
	return registerNode(cid, this, UNIQUE_NODE_ID, role, con);
}

bool Net_Node::registerNodeDynamic(Net_ClassID cid, Net_Control* con) {
	eNet_NodeRole role = con->intern->cbNodeRequest_Dynamic ? eNet_RoleProxy : eNet_RoleAuthority;
	Net_NodeID nid = con->intern->cbNodeRequest_Dynamic ? con->intern->cbNodeRequest_nodeId : con->intern->getUnusedNodeId();
	con->intern->cbNodeRequest_Dynamic = false; // only for first node
	return registerNode(cid, this, nid, role, con);
}

bool Net_Node::registerRequestedNode(Net_ClassID cid, Net_Control* con) { return registerNodeDynamic(cid, con); }

void Net_Node::applyForNetLevel(int something) {}
void Net_Node::removeFromNetLevel(int something) {}


void Net_Node::setEventNotification(bool eventForInit /* eNet_EventInit */, bool eventForRemove /* eNet_EventRemoved */) {
	intern->eventForInit = eventForInit;
	intern->eventForRemove = eventForRemove;
}

void Net_Node::sendEvent(eNet_SendMode m, Net_RepRules rules, Net_BitStream* s) {
	Net_Control::NetControlIntern::DataPackage& p = intern->control->intern->pushPackageToSend();
	p.connID = 0;
	p.sendMode = m;
	p.type = Net_Control::NetControlIntern::DataPackage::GPT_NodeEvent;
	p.nodeID = intern->nodeId;
	p.data = *s;
	delete s;
}

void Net_Node::sendEventDirect(eNet_SendMode m, Net_BitStream* s, Net_ConnID cid) {
	Net_Control::NetControlIntern::DataPackage& p = intern->control->intern->pushPackageToSend();
	p.connID = cid;
	p.sendMode = m;
	p.type = Net_Control::NetControlIntern::DataPackage::GPT_NodeEvent;
	p.nodeID = intern->nodeId;
	p.data = *s;
	delete s;
}

bool Net_Node::checkEventWaiting() { return intern->incomingEvents.size() > 0; }

Net_BitStream* Net_Node::getNextEvent(eNet_Event* e, eNet_NodeRole* r, Net_ConnID* cid) {
	if(intern->incomingEvents.size() == 0) return NULL;
	
	NetNodeIntern::Event& ev = intern->curIncomingEvent = intern->incomingEvents.front();
	intern->incomingEvents.pop_front();

	if(e) *e = ev.ev;
	if(r) *r = ev.role;
	if(cid) *cid = ev.cid;
	return &ev.stream;
}


void Net_Node::beginReplicationSetup() {
	intern->clearReplicationSetup();
}

void Net_Node::addReplicator(Net_Replicator* rep, bool autodelete) {
	intern->replicationSetup.push_back( std::make_pair(rep, autodelete) );
}

void Net_Node::addReplicationInt(Net_S32* n, int bits, bool, Net_RepFlags f, Net_RepRules r, Net_InterceptID id, int p2, int p3) {

	if((f & Net_REPFLAG_INTERCEPT) && id == 0)
		id = intern->forthcomingReplicatorInterceptID;

	struct IntReplicator : Net_ReplicatorBasic {
		Net_ReplicatorSetup setup;
		Net_S32* n;
		Net_S32 old;
		int bits;
		
		IntReplicator(const Net_ReplicatorSetup& s) : Net_ReplicatorBasic(&setup), setup(s) {}
		Net_Replicator* Duplicate(Net_Replicator*) { return new IntReplicator(*this); }		
		
		void* peekData() { return NULL; }
		void clearPeekData() {}
		
		bool checkState() { return *n != old; }
		void packData(Net_BitStream *_stream) { _stream->addInt(*n, bits); }
		void unpackData(Net_BitStream *_stream, bool _store) {
			Net_S32 i = _stream->getInt(bits);
			if(_store) *n = i;
		}
	};
	
	IntReplicator* rep = new IntReplicator(Net_ReplicatorSetup(f, r, id, p2, p3));
	rep->n = n;
	rep->old = *n;
	rep->bits = bits;
	
	addReplicator(rep, true);
}

void Net_Node::addReplicationFloat(Net_Float* n, int bits, Net_RepFlags f, Net_RepRules r, Net_InterceptID id, int p2, int p3) {

	if((f & Net_REPFLAG_INTERCEPT) && id == 0)
		id = intern->forthcomingReplicatorInterceptID;

	struct FloatReplicator : Net_ReplicatorBasic {
		Net_ReplicatorSetup setup;
		Net_Float* n;
		Net_Float old;
		int bits;
		
		FloatReplicator(const Net_ReplicatorSetup& s) : Net_ReplicatorBasic(&setup), setup(s) {}
		Net_Replicator* Duplicate(Net_Replicator*) { return new FloatReplicator(*this); }		
		
		void* peekData() { return NULL; }
		void clearPeekData() {}
		
		bool checkState() { return *n != old; }
		void packData(Net_BitStream *_stream) { _stream->addFloat(*n, bits); }
		void unpackData(Net_BitStream *_stream, bool _store) {
			Net_Float i = _stream->getFloat(bits);
			if(_store) *n = i;
		}
	};
	
	FloatReplicator* rep = new FloatReplicator(Net_ReplicatorSetup(f, r, id, p2, p3));
	rep->n = n;
	rep->old = *n;
	rep->bits = bits;
		
	addReplicator(rep, true);	
}

void Net_Node::endReplicationSetup() {}

void Net_Node::setInterceptID(Net_InterceptID id) { intern->forthcomingReplicatorInterceptID = id; }

void Net_Node::setReplicationInterceptor(Net_NodeReplicationInterceptor* inter) { intern->interceptor = inter; }








Net_ReplicatorSetup::Net_ReplicatorSetup(Net_RepFlags flags, Net_RepRules rules, Net_InterceptID id, int p2, int p3) {
	repFlags = flags;
	repRules = rules;
	interceptId = id;
}



struct NetStream::NetStreamIntern {
	void (*logFct)( const char* );
	
	NetStreamIntern() {
		logFct = NULL;
	}
	
	void log(const std::string& msg) {
		if(logFct) (*logFct) (msg.c_str());
	}
};

NetStream::NetStream() {
	intern = new NetStreamIntern();
}

NetStream::NetStream( void (*logFct)( const char* ) ) {
	intern = new NetStreamIntern();
	intern->logFct = logFct;
}

NetStream::~NetStream() {
	delete intern;
	intern = NULL;
}

bool NetStream::Init() {
	intern->log("NetStream::Init()");
	return true;
}


Net_ConnID NetConnID_server() {
	return Net_ConnID(-1);
}

Net_ConnID NetConnID_conn(CServerConnection* cl) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(&cServer->getClients()[i] == cl) return i + 1;
	}
	
	errors << "NetConnID_conn: connection invalid" << endl;
	return 0;
}

CServerConnection* serverConnFromNetConnID(Net_ConnID id) {
	if(!cServer->getClients() || !cServer->isServerRunning()) {
		errors << "serverConnFromNetConnID: server is not running" << endl;
		return NULL;
	}
	
	if(id >= 1 && id <= MAX_CLIENTS) return &cServer->getClients()[id - 1];
	
	errors << "serverConnFromNetConnID: id " << id << " is invalid" << endl;
	return NULL;
}

bool isServerNetConnID(Net_ConnID id) {
	return id == NetConnID_server();
}


