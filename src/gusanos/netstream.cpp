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


struct Net_Node::NetNodeIntern {
	
};

Net_Node::Net_Node() {
	intern = new NetNodeIntern();
}

Net_Node::~Net_Node() {
	delete intern;
	intern = NULL;
}


eNet_NodeRole Net_Node::getRole() { return eNet_RoleUndefined; }
void Net_Node::setOwner(Net_ConnID, bool something) {}
void Net_Node::setAnnounceData(Net_BitStream*) {}
Net_NodeID Net_Node::getNetworkID() { return 0; }

bool Net_Node::registerNodeUnique(Net_ClassID, eNet_NodeRole, Net_Control*) { return false; }
bool Net_Node::registerNodeDynamic(Net_ClassID, Net_Control*) { return false; }
bool Net_Node::registerRequestedNode(Net_ClassID, Net_Control*) { return false; }

void Net_Node::applyForNetLevel(int something) {}
void Net_Node::removeFromNetLevel(int something) {}


void Net_Node::setEventNotification(bool,bool) {} // TODO: true,false -> enables eEvent_Init
void Net_Node::sendEvent(eNet_SendMode, Net_RepRules rules, Net_BitStream*) {}
void Net_Node::sendEventDirect(eNet_SendMode, Net_BitStream*, Net_ConnID) {}
bool Net_Node::checkEventWaiting() { return false; }
Net_BitStream* Net_Node::getNextEvent(eNet_Event*, eNet_NodeRole*, Net_ConnID*) { return NULL; }


void Net_Node::addReplicator(Net_ReplicatorBasic*, bool) {}	
void Net_Node::beginReplicationSetup(int something) {}
void Net_Node::setInterceptID(Net_InterceptID) {}
void Net_Node::addReplicationInt(Net_S32*, int bits, bool, Net_RepFlags, Net_RepRules, int p1, int p2, int p3) {}
void Net_Node::addReplicationFloat(Net_Float*, int bits, Net_RepFlags, Net_RepRules, int p1, int p2, int p3) {}
void Net_Node::endReplicationSetup() {}
void Net_Node::setReplicationInterceptor(Net_NodeReplicationInterceptor*) {}


struct Net_Control::NetControlIntern {
	bool isServer;
	int controlId;
	std::string debugName;
	
	struct DataPackage {
		enum Type {
			GPT_Global = 0,
			GPT_NodeNew = 1,
			GPT_NodeUpdate = 2
		};

		Type type;
		Net_ConnID connID; // target or source
		Net_BitStream data;
		eNet_SendMode sendMode;
		
		DataPackage() : sendMode(eNet_ReliableOrdered) {}
		void send(CBytestream& bs);
		void read(CBytestream& bs);
	};
	
	typedef std::list<DataPackage> Packages;
	Packages packetsToSend;
	Packages packetsReceived;
		
	NetControlIntern() {
		isServer = false;
		controlId = 0;
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

Net_BitStream* Net_Control::Net_createBitStream() { return NULL; }

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
	writeEliasGammaNr(bs, data.data().size());
	bs.writeData(data.data());
}

void Net_Control::NetControlIntern::DataPackage::read(CBytestream& bs) {
	type = (NetControlIntern::DataPackage::Type) bs.readByte();
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

void Net_Control::olxParse(CBytestream& bs) {
	size_t len = readEliasGammaNr(bs);

	intern->packetsReceived.clear();
	for(size_t i = 0; i < len; ++i) {
		intern->packetsReceived.push_back(NetControlIntern::DataPackage());
		NetControlIntern::DataPackage& p = intern->packetsReceived.back();
		p.read(bs);
	}
}

void Net_Control::Net_processOutput() {
}

void Net_Control::Net_processInput() {
}

void Net_Control::Net_sendData(Net_ConnID, Net_BitStream*, eNet_SendMode) {}
Net_ClassID Net_Control::Net_registerClass(const std::string& classname, Net_ClassFlags) { return 0; }


void Net_Control::Net_setControlID(int id) { intern->controlId = id; }
void Net_Control::Net_setDebugName(const std::string& n) { intern->debugName = n; }


void Net_Control::Net_requestNetMode(Net_ConnID, int) {}




Net_BitStream* Net_Replicator::getPeekStream() { return NULL; }
void* Net_Replicator::peekDataRetrieve() { return NULL; }

void* Net_Replicator::peekData() { return NULL; }
void Net_Replicator::peekDataStore(void*) {}



Net_ReplicatorSetup::Net_ReplicatorSetup(Net_RepFlags, Net_RepRules, Net_InterceptID p1, int p2, int p3) {
	interceptId = p1;
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
		if(&cServer->getClients()[i] == cl) return i;
	}
	
	errors << "NetConnID_conn: connection invalid" << endl;
	return 0;
}

CServerConnection* serverConnFromNetConnID(Net_ConnID id) {
	if(!cServer->getClients() || !cServer->isServerRunning()) {
		errors << "serverConnFromNetConnID: server is not running" << endl;
		return NULL;
	}
	
	if(id >= 0 && id < MAX_CLIENTS) return &cServer->getClients()[id];
	
	errors << "serverConnFromNetConnID: id " << id << " is invalid" << endl;
	return NULL;
}

bool isServerNetConnID(Net_ConnID id) {
	return id == NetConnID_server();
}


