/////////////////////////////////////////
//
//             OpenLieroX
//
//   work by JasonB
//   code under LGPL
//   enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Network Channel class
// Created 16/6/01
// Jason Boettcher


#include <map>

#include "LieroX.h"
#include "Debug.h"
#include "CChannel.h"
#include "StringUtils.h"
#include "Timer.h"
#include "MathLib.h"
#include "CServer.h"



// default max size for UDP packets for windows is 1280
// only a size of 512 is guaranteed
enum { 
	MAX_PACKET_SIZE = 512,
	RELIABLE_HEADER_LEN = 8 // Only for CChannel_056b
};

void CChannel::Clear()
{
	Socket = NULL;
	iPacketsDropped = 0;
	iPacketsGood = 0;
	cIncomingRate.clear();
	cOutgoingRate.clear();
	iOutgoingBytes = 0;
	iIncomingBytes = 0;
	iPing = 0;
	fLastSent = fLastPckRecvd = fLastPingSent = AbsTime();
	iCurrentIncomingBytes = 0;
	iCurrentOutgoingBytes = 0;
	Messages.clear();
	
	ReliableStreamBandwidthCounter = 0.0f;
	ReliableStreamLastSentTime = tLX->currentTime;
	ReliableStreamBandwidthCounterLastUpdate = tLX->currentTime;
	LimitReliableStreamBandwidth( -1.0f, 5.0f, 1024.0f );
}

///////////////////
// Setup the channel
void CChannel::Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock)
{
	Clear();
	RemoteAddr = _adr;
	fLastPckRecvd = tLX->currentTime;
	Socket = _sock;
	fLastSent = AbsTime();
	fLastPingSent = fLastSent;
	iPing = 0;
}

////////////////////
// Adds a packet to reliable queue
void CChannel::AddReliablePacketToSend(CBytestream& bs)
{
	if (bs.GetLength() > MAX_PACKET_SIZE - RELIABLE_HEADER_LEN)  {
		warnings
			<< "trying to send a reliable packet of size " << bs.GetLength()
			<< " which is bigger than allowed size (" << (MAX_PACKET_SIZE - RELIABLE_HEADER_LEN)
			<< "), packet might not be sent at all!" << endl;
		Messages.push_back(bs); // Try to send it anyway, perhaps we're lucky...
		return;
	}

	if(bs.GetLength() == 0)
		return;

	Messages.push_back(bs);
	// The messages are joined in Transmit() in one bigger packet, until it will hit bandwidth limit
}

size_t CChannel::currentReliableOutSize() {
	size_t s = 0;
	for(std::list<CBytestream>::iterator i = Messages.begin(); i != Messages.end(); ++i)
		s += i->GetLength();
	return s;
}

void CChannel::UpdateTransmitStatistics( int sentDataSize )
{
	// Update statistics
	iOutgoingBytes += sentDataSize;
	iCurrentOutgoingBytes += sentDataSize;
	fLastSent = tLX->currentTime;

	// Calculate the bytes per second
	cOutgoingRate.addData( tLX->currentTime, sentDataSize );
}

void CChannel::UpdateReceiveStatistics( int receivedDataSize )
{
	// Got a packet (good or bad), update the received time
	fLastPckRecvd = tLX->currentTime;

	// Update statistics - calculate the bytes per second
	// TODO: it was Bytestream->GetRestLen() before for iIncomingBytes and iCurrentIncomingBytes,
	// so skipped packet header, I think it's not that important, check this Albert and remove this TODO
	iIncomingBytes += receivedDataSize;
	iCurrentIncomingBytes += receivedDataSize;
	cIncomingRate.addData( tLX->currentTime, receivedDataSize );
}

void CChannel::LimitReliableStreamBandwidth( float BandwidthLimit, float MaxPacketRate, float BandwidthCounterMaxValue )
{
	ReliableStreamBandwidthLimit = BandwidthLimit;
	ReliableStreamMaxPacketRate = MaxPacketRate;
	ReliableStreamBandwidthCounterMaxValue = BandwidthCounterMaxValue;
	// That's all, we won't reset ReliableStreamBandwidthCounter here
}

void CChannel::UpdateReliableStreamBandwidthCounter()
{
	ReliableStreamBandwidthCounter += 
		( tLX->currentTime - ReliableStreamBandwidthCounterLastUpdate ).seconds() *
		ReliableStreamBandwidthLimit;

	ReliableStreamBandwidthCounterLastUpdate = tLX->currentTime;
	
	if( ReliableStreamBandwidthCounter > ReliableStreamBandwidthCounterMaxValue )
		ReliableStreamBandwidthCounter = ReliableStreamBandwidthCounterMaxValue;
}

bool CChannel::CheckReliableStreamBandwidthLimit( float dataSizeToSend, bool doUpdate )
{
	if( ReliableStreamBandwidthLimit <= 0 ) // No bandwidth limit
		return true;

	if( ReliableStreamBandwidthCounter >= dataSizeToSend ||
		// Allow sending packets that exceed MaxValue, if Counter == MaxValue, then Counter will become negative
		ReliableStreamBandwidthCounter >= ReliableStreamBandwidthCounterMaxValue ) 
	{
		if(doUpdate) {
			ReliableStreamBandwidthCounter -= dataSizeToSend;
			ReliableStreamLastSentTime = tLX->currentTime;
		}
		return true;
	}

	return false;
}

float CChannel::MaxDataPossibleToSendInstantly() {
	if( ReliableStreamBandwidthLimit <= 0 ) // No bandwidth limit
		return GameServer::getMaxUploadBandwidth();
	
	float tmpReliableStreamBandwidthCounter = ReliableStreamBandwidthCounter +
		( tLX->currentTime - ReliableStreamBandwidthCounterLastUpdate ).seconds() *
		ReliableStreamBandwidthLimit;

	tmpReliableStreamBandwidthCounter = MAX(tmpReliableStreamBandwidthCounter, ReliableStreamBandwidthCounterMaxValue);	
	tmpReliableStreamBandwidthCounter -= currentReliableOutSize();
	
	tmpReliableStreamBandwidthCounter -= 4; // all kind of header stuff
	
	return MAX(0.0f, tmpReliableStreamBandwidthCounter);
}


bool CChannel::ReliableStreamBandwidthLimitHit()
{
	if( ReliableStreamBandwidthLimit <= 0 ||
		ReliableStreamBandwidthCounter >= ReliableStreamBandwidthCounterMaxValue / 2.0f ||
		ReliableStreamLastSentTime + 1.0f / ReliableStreamMaxPacketRate <= tLX->currentTime )
		return false;
	return true;
}

///////////////////
// CChannel for LX 0.56b implementation - LOSES PACKETS, and that cannot be fixed.

void CChannel_056b::Clear()
{
	CChannel::Clear();
	iOutgoingSequence = 0;
	iReliableSequence = 0;
	iLast_ReliableSequence = 0;
	iIncoming_ReliableSequence = 0;
	iIncomingSequence = 0;
	iIncomingAcknowledged = 0;
	iOutgoingBytes = 0;
	iIncomingBytes = 0;
	fLastSent = AbsTime();
	bNewReliablePacket = false;
	iPongSequence = -1;
	Reliable.Clear();
}

void CChannel_056b::Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock)
{
	Clear();
	CChannel::Create( _adr, _sock );
}

///////////////////
// Transmitt data, as well as handling reliable packets
void CChannel_056b::Transmit( CBytestream *bs )
{
	UpdateReliableStreamBandwidthCounter();
	
	CBytestream outpack;
	Uint32 SendReliable = 0;
	ulong r1,r2;

	// If the remote side dropped the last reliable packet, re-send it
	if( iIncomingAcknowledged > iLast_ReliableSequence && 
		iIncoming_ReliableAcknowledged != iReliableSequence &&
		CheckReliableStreamBandwidthLimit( (float)Reliable.GetLength() ) )
	{
		//hints << "Remote side dropped a reliable packet, resending..." << endl;
		SendReliable = 1;
	}


	// We send reliable message in these cases:
	// 1. The reliable buffer is empty, we copy the reliable message into it and send it
	// 2. We need to refresh ping
	if( Reliable.GetLength() == 0 && 
		! ReliableStreamBandwidthLimitHit() &&
		( !Messages.empty() || (tLX->currentTime >= fLastPingSent + 1.0f && iPongSequence == -1)))
	{
		while( ! Messages.empty() && 
				Reliable.GetLength() + Messages.front().GetLength() <= MAX_PACKET_SIZE - RELIABLE_HEADER_LEN &&
				CheckReliableStreamBandwidthLimit( (float)Messages.front().GetLength() ) )
		{
				Reliable.Append( & Messages.front() );
				Messages.pop_front();
		}

		// XOR the reliable sequence
		iReliableSequence ^= 1;

		// We got a reliable packet to send
		SendReliable = 1;
	}

	// Create the reliable packet header
	r1 = iOutgoingSequence | (SendReliable << 31);
	r2 = iIncomingSequence | (iIncoming_ReliableSequence << 31);

	iOutgoingSequence++;

	outpack.writeInt(r1,4);
	outpack.writeInt(r2,4);


	// If were sending a reliable message, send it first
	if(SendReliable) {
		outpack.Append(&Reliable);
		iLast_ReliableSequence = iOutgoingSequence - 1;

		// If we are sending a reliable message, remember this time and use it for ping calculations
		if (iPongSequence == -1)  {
			iPongSequence = iOutgoingSequence - 1;
			fLastPingSent = tLX->currentTime; //GetTime();
		}

	}

	// And add on the un reliable data if room in the packet struct
	if(bs) {
		if(outpack.GetLength() + bs->GetLength() < 4096) // Backward compatibility, the old bytestream has a fixed buffer of 4096 bytes
			outpack.Append(bs);
		else
			hints << "Not adding unrealiable data to avoid too big packets" << endl;
	}


	// Send the packet
	Socket->setRemoteAddress(RemoteAddr);
	outpack.Send(Socket.get());

	UpdateTransmitStatistics( outpack.GetLength() );
}


///////////////////
// Process channel (after receiving data)
bool CChannel_056b::Process(CBytestream *bs)
{
	Uint32 Sequence, SequenceAck;
	Uint32 ReliableAck, ReliableMessage;
	int drop;

	// Start from the beginning of the packet
	bs->ResetPosToBegin();
	if( bs->GetLength() == 0 )
		return false;

	UpdateReceiveStatistics( bs->GetLength() );

	// Read the reliable packet header
	Sequence = bs->readInt(4);
	SequenceAck = bs->readInt(4);

	// Get the reliable bits
	ReliableMessage = Sequence >> 31;
	ReliableAck = SequenceAck >> 31;

	// Get rid of the reliable bits
	Sequence &= ~(1<<31);
	SequenceAck &= ~(1<<31);

	// Get rid of the old packets
	// Small hack: there's a bug in old clients causing the first packet being ignored and resent later
	// It caused a delay when joining (especially on high-ping servers), this hack improves it
	if((Sequence <= iIncomingSequence) && (Sequence != 0 && iIncomingSequence != 0)) {
//		warnings << "Packet dropped" << endl;
//		bs->Dump();
		/*
		If we didn't ignore it here, we would become it
		again (the remote side will resend it because it thinks we've dropped
		the packet) and then parse it again => doubled text etc */
		// see GameServer::ReadPackets and CClient::ReadPackets
		// or perhaps it's ok to return true here but we should change the behaviour in *::ReadPackets
		return false;
	}


	// Check for dropped packets
	drop = Sequence - (iIncomingSequence+1);
	if(drop>0)
		// Update statistics
		iPacketsDropped++;


	// If the outgoing reliable message has been acknowledged, clear it for more reliable messages
	if(ReliableAck == iReliableSequence)
		Reliable.Clear();

	// Check if pong has been acknowledged
	if(SequenceAck >= (size_t)iPongSequence)  {
		iPongSequence = -1;  // Ready for new pinging
		iPing = (int)((tLX->currentTime - fLastPingSent).milliseconds());
	}


	// If this packet contained a reliable message, update the sequences
	iIncomingSequence = Sequence;
	iIncomingAcknowledged = SequenceAck;
	iIncoming_ReliableAcknowledged = ReliableAck;
	if(ReliableMessage)  {
		iIncoming_ReliableSequence ^= 1;
		bNewReliablePacket = true;
	} else
		bNewReliablePacket = false;


	// Update the statistics
	iPacketsGood++;


	return true;
}


void CChannel_056b::recheckSeqs() {
	// Ensure the incoming sequence matchs the outgoing sequence
	if (this->getInSeq() >= this->getOutSeq())  {
		//if (chan->getInSeq() != chan->getOutSeq())
		//	warnings << cl->getWorm(0)->getName() << ": sequences not same (IN: " << chan->getInSeq() << ", OUT: " << chan->getOutSeq() << ")" << endl;
		//else
		//	hints << cl->getWorm(0)->getName() << ": sequences match!! (" << chan->getInSeq() << ")" << endl;*/
		this->setOutSeq(this->getInSeq());
	} else {
		// Sequences have slipped
		// Karel said: it's bullshit from JasonB, so we can ignore this warning :)
		//warnings << cl->getWorm(0)->getName() << ": sequences have slipped (IN: " << chan->getInSeq() << ", OUT: " << chan->getOutSeq() << ")" << endl;
		// TODO: Set the player's send_data property to false
	}
}


///////////////////
// Reliable CChannel implementation by pelya ( I hope it's less messy, though it has more code ).

/*
There are logical packets, each has it's own sequence number,
multiple logical packets can be transmitted in single net packet.
CChannel2::Process(CBytestream *) will return the logical packets one by one,
without merging them in one packet, to increase robustness.
It will modify CBytestream * argument for that.
The net packet format is:
	Acknowledged Packet Index 1 - 2 bytes, highest bit = 1
	...
	Acknowledged Packet Index N - 2 bytes, highest bit = 1
	Last Acknowledged Packet Index - 2 bytes, highest bit = 0 (marks end of acknowledged packets list) -
		 should be the lowest acknowledged sequence, all packets with lower
		 sequences are considered acknowledged.
	Packet Index 1 - 2 bytes, highest bit = 1
	Packet Size 1 - 2 bytes
	...
	Packet Index N - 2 bytes, highest bit = 1
	Packet Size N - 2 bytes
	Last Packet Index - 2 bytes, highest bit = 0 (marks end of packets list)
	Last Packet Size - 2 bytes, if we don't have reliable packet then Last Packet Size == 0, Last Packet Index ignored.
	Packets data
	Non-reliable packet data

Only Last Acknowledged Packet Index and Last Packet Index are mandatory -
then CChannel will act like old CChannel_056b, when only one packet is allowed
to be flying through network at any time.
*/

enum { 

// SEQUENCE_WRAPAROUND is where sequence wraps to zero - sequence range is from 0 to SEQUENCE_WRAPAROUND-1.
// All sequences (or packet indexes) used are wrapping around at this number
// We cannot allow four leadinf 0xFF bytes in reliable packet, so SEQUENCE_WRAPAROUND
// is slightly less than 0x7FFF.
	SEQUENCE_WRAPAROUND = 32766,

// SEQUENCE_SAFE_DIST is the max distance between two sequences when packets will get ignored as erroneous ones.
	SEQUENCE_SAFE_DIST = 100,

// MAX_NON_ACKNOWLEDGED_PACKETS is max amount of packets that can be flying through the net at the same time.
	MAX_NON_ACKNOWLEDGED_PACKETS = 3,	// 1 is minimum - it behaves like old CChannel then.

// SEQUENCE_HIGHEST_BIT is highest bit in a 2-byte int, for convenience.
	SEQUENCE_HIGHEST_BIT = 0x8000
};

// How much to wait before sending another empty keep-alive packet, sec.
const float KEEP_ALIVE_PACKET_TIMEOUT = 1.0f;

// How much to wait before sending data packet again, sec - 
// if packets rarely get lost over net it will decrease bandwidth dramatically, for little lag tradeoff.
// Set to 0 to flood net with packets instantly as in CChannel_056b.
// If any new data available to send, or unreliable data present, packet is sent anyway.
// This is only inital value, it will get changed with time according to ping.
const float DATA_PACKET_TIMEOUT = 0.2f;

// DataPacketTimeout = ping / DATA_PACKET_TIMEOUT_PING_COEFF, 
// the bigger that value is, the more often channel will re-send data delayed in network.
const float DATA_PACKET_TIMEOUT_PING_COEFF = 1.5f;

#ifdef DEBUG
const float DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY = 0.0f; // Self-explanatory
#endif

// Do not check "if( sequence1 < sequence2 )", use this function instead, it will handle wraparound issues.
// SequenceDiff( 32765, 32764 ) equals to SequenceDiff( 0, 32765 ) equals to SequenceDiff( 1, 0 ) equals to 1.
int SequenceDiff( int s1, int s2 )
{
	int diff = s1 - s2;
	while( diff > SEQUENCE_WRAPAROUND / 2 - 1 )
		diff -= SEQUENCE_WRAPAROUND;
	while( diff < - (SEQUENCE_WRAPAROUND / 2) )
		diff += SEQUENCE_WRAPAROUND;
	return diff;
}

void CChannel2::Clear()
{
	CChannel::Clear();
	Messages.clear();
	ReliableOut.clear();
	ReliableIn.clear();
	LastReliableOut = 0;
	LastAddedToOut = 0;
	LastReliableIn = 0;
	PongSequence = -1;
	LastReliablePacketSent = SEQUENCE_WRAPAROUND - 1;
	NextReliablePacketToSend = 0;
	LastReliableIn_SentWithLastPacket = SEQUENCE_WRAPAROUND - 1;
	
	KeepAlivePacketTimeout = KEEP_ALIVE_PACKET_TIMEOUT;
	DataPacketTimeout = DATA_PACKET_TIMEOUT;
	MaxNonAcknowledgedPackets = MAX_NON_ACKNOWLEDGED_PACKETS;

	#ifdef DEBUG
	DebugSimulateLaggyConnectionSendDelay = tLX->currentTime;
	#endif
};

void CChannel2::Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock)
{
	Clear();
	CChannel::Create( _adr, _sock );
}

// Get reliable packet from local buffer
bool CChannel2::GetPacketFromBuffer(CBytestream *bs)
{
	if( ReliableIn.size() == 0 )
		return false;

	int seqMin = LastReliableIn;
	PacketList_t::iterator itMin = ReliableIn.end();
	for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end(); it++ )
	{
		if( SequenceDiff( seqMin, it->second ) >= 0 )
		{
			seqMin = it->second;
			itMin = it;
		};
	};
	if( itMin != ReliableIn.end() )
	{
		*bs = itMin->first;
		ReliableIn.erase(itMin);
		return true;
	};
	return false;
}

// This function will first return non-reliable data,
// and then one or many reliable packets - it will modify bs for that,
// so you should call it in a loop, clearing bs after each call.
bool CChannel2::Process(CBytestream *bs)
{
	bs->ResetPosToBegin();
	if( bs->GetLength() == 0 )
		return GetPacketFromBuffer(bs);

	UpdateReceiveStatistics( bs->GetLength() );

	// Acknowledged packets info processing

	// Read acknowledged packets indexes
	unsigned seqAck = bs->readInt(2);
	std::vector< int > seqAckList;
	while( seqAck & SEQUENCE_HIGHEST_BIT )
	{
		seqAckList.push_back( seqAck & ~ SEQUENCE_HIGHEST_BIT );
		seqAck = bs->readInt(2);
	};
	if( SequenceDiff( seqAck, LastReliableOut ) < 0 || SequenceDiff( seqAck, LastReliableOut ) > SEQUENCE_SAFE_DIST )
	{
		iPacketsDropped++;	// Update statistics
		return GetPacketFromBuffer(bs);	// Packet from the past or from too distant future - ignore it.
	};

	LastReliableOut = seqAck;

	iPacketsGood++;	// Update statistics

	// Delete acknowledged packets from buffer
	for( PacketList_t::iterator it = ReliableOut.begin(); it != ReliableOut.end(); )
	{
		bool erase = false;
		if( SequenceDiff( LastReliableOut, it->second ) >= 0 )
			erase = true;
		for( unsigned f=0; f<seqAckList.size(); f++ )
			if( seqAckList[f] == it->second )
				erase = true;
		if(erase)
			it = ReliableOut.erase(it);
		else
			it++;
	}

	// Calculate ping ( with LastReliableOut, not with last packet - should be fair enough )
	if( PongSequence != -1 && SequenceDiff( LastReliableOut, PongSequence ) >= 0 )
	{
		iPing = (int) ((tLX->currentTime - fLastPingSent).milliseconds());
		PongSequence = -1;
		// Traffic shaping occurs here - change DataPacketTimeout according to received ping
		// Change the value slowly, to avoid peaks
		// TODO: I haven't really tested it this thing does any good, but it seems to work okay.
		DataPacketTimeout = ( iPing/1000.0f/DATA_PACKET_TIMEOUT_PING_COEFF + DataPacketTimeout*9.0f ) / 10.0f; 
	};

	// Processing of arrived data packets

	// Read packets info
	std::vector< int > seqList;
	std::vector< int > seqSizeList;
	unsigned seq = bs->readInt(2);
	while( seq & SEQUENCE_HIGHEST_BIT )
	{
		seqList.push_back( seq & ~ SEQUENCE_HIGHEST_BIT );
		seqSizeList.push_back( bs->readInt(2) );
		seq = bs->readInt(2);
	};
	seqList.push_back( seq );
	seqSizeList.push_back( bs->readInt(2) );

	// Put packets in buffer
	for( unsigned f=0; f<seqList.size(); f++ )
	{
		if( seqSizeList[f] == 0 ) // Last reliable packet may have size 0, if we're received non-reliable-only net packet
			continue;	// Just skip it, it's fake packet

		bool addPacket = true;
		for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end() && addPacket; it++ )
			if( it->second == seqList[f] )
				addPacket = false;
		if( addPacket && SequenceDiff( seqList[f], LastReliableIn ) > 0 ) // Do not add packets from the past
		{	// Packet not in buffer yet - add it
			CBytestream bs1;
			bs1.writeData( bs->readData(seqSizeList[f]) );
			ReliableIn.push_back( std::make_pair( bs1, seqList[f] ) );
		}
		else	// Packet is in buffer already
		{
			// We may check here if arrived packet is the same as packet in buffer, and print errors.
			bs->Skip( seqSizeList[f] );
		};
	}

	// Increase LastReliableIn until the first packet that is missing from sequence
	while( true )	// I just love such constructs :P don't worry, I've put "break" inside the loop.
	{
		bool nextPacketFound = false;
		int LastReliableInInc = LastReliableIn + 1;	// Next value of LastReliableIn
		if( LastReliableInInc >= SEQUENCE_WRAPAROUND )
			LastReliableInInc = 0;
		for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end() && !nextPacketFound; it++ )
			if( it->second == LastReliableInInc )
				nextPacketFound = true;
		if( nextPacketFound )
			LastReliableIn = LastReliableInInc;
		else
			break;
	};

	if( bs->GetRestLen() > 0 )	// Non-reliable data left in this packet
		return true;	// Do not modify bs, allow user to read non-reliable data at the end of bs

	if( GetPacketFromBuffer(bs) )	// We can return some reliable packet
		return true;

	// We've got valid empty packet, or packet from future, return empty packet - bs->GetRestLen() == 0 here.
	// It is required to update server statistics, so clients that don't send packets won't timeout.
	return true;
};

void CChannel2::Transmit(CBytestream *unreliableData)
{
	UpdateReliableStreamBandwidthCounter();

	#ifdef DEBUG
	// Very simple laggy connection emulation - send next packet once per DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY
	if( DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY > 0.0f )
	{
		if( DebugSimulateLaggyConnectionSendDelay > tLX->currentTime )
			return;
		DebugSimulateLaggyConnectionSendDelay = tLX->currentTime + DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY;
	}
	#endif

	CBytestream bs;
	// Add acknowledged packets indexes

	for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end(); it++ )
		if( SequenceDiff( it->second, LastReliableIn ) > 0 ) // Packets out of sequence
			bs.writeInt( it->second | SEQUENCE_HIGHEST_BIT, 2 );

	bs.writeInt( LastReliableIn, 2 );

	// Add reliable packet to ReliableOut buffer
	while( (int)ReliableOut.size() < MaxNonAcknowledgedPackets && ! Messages.empty() && ! ReliableStreamBandwidthLimitHit() )
	{
		LastAddedToOut ++ ;
		if( LastAddedToOut >= SEQUENCE_WRAPAROUND )
			LastAddedToOut = 0;

		ReliableOut.push_back( std::make_pair( Messages.front(), LastAddedToOut ) );
		Messages.pop_front();

		while( ! Messages.empty() && 
				ReliableOut.back().first.GetLength() + Messages.front().GetLength() <= MAX_PACKET_SIZE - RELIABLE_HEADER_LEN )
		{
			ReliableOut.back().first.Append( & Messages.front() );
			Messages.pop_front();
		}
	};

	// Check if other side acknowledged packets with indexes bigger than NextReliablePacketToSend,
	// and roll NextReliablePacketToSend back to LastReliableOut.
	if( ! ReliableOut.empty() )
	{
		for( PacketList_t::iterator it = ReliableOut.begin(), it1 = it++; it != ReliableOut.end(); it1 = it++ )
		{
			if( SequenceDiff( it->second, it1->second ) != 1 )
				NextReliablePacketToSend = LastReliableOut;
		};
		if( ReliableOut.back().second != LastAddedToOut )
			NextReliablePacketToSend = LastReliableOut;
	};

	// Timeout occured - other side didn't acknowledge our packets in time - re-send all of them from the first one.
	if( LastReliablePacketSent == LastAddedToOut &&
		SequenceDiff( LastReliablePacketSent, LastReliableOut ) >= MaxNonAcknowledgedPackets &&
		tLX->currentTime - fLastSent >= DataPacketTimeout )
	{
		NextReliablePacketToSend = LastReliableOut;
	}
	
	// Add packet headers and data - send all packets with indexes from NextReliablePacketToSend and up.
	// Add older packets to the output first.
	// NextReliablePacketToSend points to the last packet.
	CBytestream packetData;
	bool unreliableOnly = true;
	bool firstPacket = true;	// Always send first packet, even if it bigger than MAX_PACKET_SIZE
	int packetIndex = LastReliableOut;
	int packetSize = 0;
	
	for( PacketList_t::iterator it = ReliableOut.begin(); it != ReliableOut.end(); it++ )
	{
		if( SequenceDiff( it->second, NextReliablePacketToSend ) >= 0 )
		{
			if( ! CheckReliableStreamBandwidthLimit( (float)(it->first.GetLength() + 4) ) ||
				( bs.GetLength() + 4 + packetData.GetLength() + it->first.GetLength() > MAX_PACKET_SIZE && ! firstPacket ) )
				break;

			if( !firstPacket )
			{
				bs.writeInt( packetIndex | SEQUENCE_HIGHEST_BIT, 2 );
				bs.writeInt( packetSize, 2 );
			};
			packetIndex = it->second;
			packetSize = it->first.GetLength();

			firstPacket = false;
			unreliableOnly = false;
			NextReliablePacketToSend = it->second;

			packetData.Append( &it->first );
		};
	};

	bs.writeInt( packetIndex, 2 );
	bs.writeInt( packetSize, 2 );
	
	bs.Append( &packetData );

	if( unreliableOnly )
		bs.Append(unreliableData);
	else
	{
		if( bs.GetLength() + unreliableData->GetLength() <= MAX_PACKET_SIZE )
			bs.Append(unreliableData);

		// If we are sending a reliable message, remember this time and use it for ping calculations
		if (PongSequence == -1)
		{
			PongSequence = NextReliablePacketToSend;
			fLastPingSent = tLX->currentTime;
		};
	};

	if( unreliableData->GetLength() == 0 &&
		LastReliablePacketSent == LastAddedToOut &&
		tLX->currentTime - fLastSent < DataPacketTimeout )
	{
		// No unreliable data to send, and we've just sent the same packet -
		// send it again after some timeout, don't flood net.
		return;
	};
	
	if( unreliableData->GetLength() == 0 && packetData.GetLength() == 0 && 
		LastReliableIn == LastReliableIn_SentWithLastPacket &&
		tLX->currentTime - fLastSent < KeepAlivePacketTimeout )
	{
		// Nothing to send really, send one empty packet per halfsecond so we won't timeout,
		// but always send first packet with acknowledges, or other side will flood
		// non-acknowledged packets for halfsecond.
		// CChannel_056b will always send packet on each frame, so we're conserving bandwidth compared to it, hehe.

		cOutgoingRate.addData( tLX->currentTime, 0 );
		return;
	}

	// Send the packet
	Socket->setRemoteAddress(RemoteAddr);
	bs.Send(Socket.get());

	LastReliableIn_SentWithLastPacket = LastReliableIn;
	LastReliablePacketSent = NextReliablePacketToSend;

	UpdateTransmitStatistics( bs.GetLength() );
}


///////////////////
// Robustness test for CChannel

std::string printBinary(const std::string & s)
{
	std::string r;
	char buf[10];
	for(size_t f=0; f<s.size(); f++)
	{
		sprintf( buf, "%02X ", (unsigned)( (unsigned char)s[f] ) );
		r += buf;
	}
	return r;
}

void TestCChannelRobustness()
{
	notes << "Testing CBytestream" << endl;
	CBytestream bsTest;
	bsTest.Test();
	notes << "\n\n\n\nTesting CChannel robustness" << endl;
	int lagMin = 50;
	int lagMax = 400;
	int packetLoss = 15; // In percents
	float packetsPerSecond1 = 10.0f; // One channel sends faster than another
	float packetsPerSecond2 = 0.2f;
	int packetExtraData = 8192; // Extra data in bytes to add to packet to check buffer overflows

	CChannel3 c1, c2;	//CChannel_056b c1, c2;
	SmartPointer<NetworkSocket> s1 = new NetworkSocket(); s1->OpenUnreliable(0);
	SmartPointer<NetworkSocket> s2 = new NetworkSocket(); s2->OpenUnreliable(0);
	SmartPointer<NetworkSocket> s1lag = new NetworkSocket(); s1lag->OpenUnreliable(0);
	SmartPointer<NetworkSocket> s2lag = new NetworkSocket(); s2lag->OpenUnreliable(0);
	NetworkAddr a1, a2, a1lag, a2lag;
	a1 = s1->localAddress();
	a2 = s2->localAddress();
	a1lag = s1lag->localAddress();
	a2lag = s2lag->localAddress();
	c1.Create( a1lag, s1 );
	c2.Create( a2lag, s2 );
	s1lag->setRemoteAddress( a2 );
	s2lag->setRemoteAddress( a1 );

	std::multimap< int, CBytestream > s1buf, s2buf;

	int i1=0, i2=0, i1r=0, i2r=0;
	float packetDelay1 = 10000000;
	if( packetsPerSecond1 > 0 )
		packetDelay1 = 1000.0f / packetsPerSecond1;
	float packetDelay2 = 10000000;
	if( packetsPerSecond2 > 0 )
		packetDelay2 = 1000.0f / packetsPerSecond2;
	float nextPacket1 = 0;
	float nextPacket2 = 0;
	for( int testtime=0; testtime < 100000; testtime+= 10, nextPacket1 += 10, nextPacket2 += 10 )
	{
		tLX->currentTime = AbsTime(testtime);

		// Transmit number sequence and some unreliable info
		CBytestream b1, b2, b1u, b2u;

		if( nextPacket1 >= packetDelay1 )
		{
			nextPacket1 = 0;
			i1++;
			b1.writeInt(i1, 4);
			for( int f=0; f<packetExtraData; f++ )
				b1.writeByte(0xff);
			c1.AddReliablePacketToSend(b1);
		}

		if( nextPacket2 >= packetDelay2 )
		{
			nextPacket2 = 0;
			i2++;
			b2.writeInt(i2, 4);
			for( int f=0; f<packetExtraData; f++ )
				b2.writeByte(0xff);
			c2.AddReliablePacketToSend(b2);
		}


		c1.Transmit( &b1u );
		c2.Transmit( &b2u );

		b1.Clear();
		b2.Clear();

		b1.Read(s1lag.get());
		b2.Read(s2lag.get());
		b1.ResetPosToBegin();
		b2.ResetPosToBegin();

		// Add the lag
		if( b1.GetLength() != 0 )
		{
			if( GetRandomInt(100) + 1 < packetLoss )
				notes << testtime << ": c1 sent packet - lost (" << c1.Messages.size() << 
						" in buf): " << printBinary(b1.readData()) << endl;
			else
			{
				int lag = ((testtime + lagMin + GetRandomInt(lagMax-lagMin)) / 10)*10; // Round to 10
				s1buf.insert( std::make_pair( lag, b1 ) );
				notes<< testtime << ": c1 sent packet - lag " << lag << " size " << b1.GetLength() << " (" << c1.Messages.size() << 
						" in buf): " << printBinary(b1.readData()) << endl;
			}
		}

		for( std::multimap< int, CBytestream > :: iterator it = s1buf.lower_bound(testtime);
				it != s1buf.upper_bound(testtime); it++ )
		{
			it->second.ResetPosToBegin();
			it->second.ResetPosToBegin();
			it->second.Send(s1lag.get());
		}

		if( b2.GetLength() != 0 )
		{
			if( GetRandomInt(100) + 1 < packetLoss )
				notes << testtime << ": c2 sent packet - lost (" << c2.Messages.size() <<
						" in buf): " << printBinary(b2.readData()) << endl;
			else
			{
				int lag = ((testtime + lagMin + GetRandomInt(lagMax-lagMin)) / 10)*10; // Round to 10
				s2buf.insert( std::make_pair( lag, b2 ) );
				notes << testtime << ": c2 sent packet - lag " << lag << " size " << b2.GetLength() << " (" << c2.Messages.size() <<
						" in buf): " << printBinary(b2.readData()) << endl;
			}
		}

		for( std::multimap< int, CBytestream > :: iterator it = s2buf.lower_bound(testtime);
				it != s2buf.upper_bound(testtime); it++ )
		{
			it->second.ResetPosToBegin();
			it->second.ResetPosToBegin();
			it->second.Send(s2lag.get());
		}

		// Receive and check number sequence and unreliable info
		b1.Clear();
		b2.Clear();

		b1.Read(s1.get());
		b2.Read(s2.get());
		b1.ResetPosToBegin();
		b2.ResetPosToBegin();

		if( b1.GetLength() != 0 )
		{
			notes << testtime << ": c1 recv packet (ping " << c1.getPing() << "): " << printBinary(b1.readData()) << endl;
			b1.ResetPosToBegin();
			while( c1.Process( &b1 ) )
			{
				while( b1.GetRestLen() != 0 )
				{
					int i1rr = b1.readInt(4);
					notes << testtime << ": c1 reliable packet, data " << hex(i1rr) << 
							" expected " << i1r+1 << " - " << (i1rr == i1r+1 ? "good" : "ERROR!") << endl;
					i1r = i1rr;
					for( int f=0; f<packetExtraData; f++ )
						b1.readByte();
				}
				b1.Clear();
			}
		}

		if( b2.GetLength() != 0 )
		{
			notes << testtime << ": c2 recv packet (ping " << c2.getPing() << "): " << printBinary(b2.readData()) << endl;
			b2.ResetPosToBegin();
			while( c2.Process( &b2 ) )
			{
				while( b2.GetRestLen() != 0 )
				{
					int i2rr = b2.readInt(4);
					notes << testtime << ": c2 reliable packet, data " << hex(i2rr) << 
							" expected " << i2r+1 << " - " << (i2rr == i2r+1 ? "good" : "ERROR!") << endl;
					i2r = i2rr;
					for( int f=0; f<packetExtraData; f++ )
						b2.readByte();
				}
				b2.Clear();
			}
		}

	}
}

/*
The format for packet is the same as with CChannel2, but with CRC16 added at the beginning,
and with indicator that packet is split into several smaller packets.
Packet won't contain four leading 0xFF because of CRC16, because two other bytes are acknowledged packet index.

In case Packet Size highest bit = 1 ( SEQUENCE_HIGHEST_BIT ) that means we're transmitting big packet
split into several smaller packets. Sequence of packets with Packet Size highest bit = 1 and
one packet with Packet Size highest bit = 0 following it is assembled into one big logical packet for user.
*/

enum {
	MAX_FRAGMENTED_PACKET_SIZE = MAX_PACKET_SIZE - 24 // Actually 12 bytes are enough, but I want to have safety bound.
};

Uint16 crc16(const char * buffer, size_t len, Uint16 crc = 0xffff); // Default non-zero value

bool CChannel3::Packet_t::operator < ( const Packet_t & p ) const
{ 
	return SequenceDiff( idx, p.idx ) < 0; 
};

void CChannel3::Clear()
{
	CChannel::Clear();
	Messages.clear();
	ReliableOut.clear();
	ReliableIn.clear();
	LastReliableOut = 0;
	LastAddedToOut = 0;
	LastReliableIn = 0;
	PongSequence = -1;
	LastReliablePacketSent = SEQUENCE_WRAPAROUND - 1;
	NextReliablePacketToSend = 0;
	LastReliableIn_SentWithLastPacket = SEQUENCE_WRAPAROUND - 1;
	
	KeepAlivePacketTimeout = KEEP_ALIVE_PACKET_TIMEOUT;
	DataPacketTimeout = DATA_PACKET_TIMEOUT;
	MaxNonAcknowledgedPackets = MAX_NON_ACKNOWLEDGED_PACKETS;

	#ifdef DEBUG
	DebugSimulateLaggyConnectionSendDelay = tLX->currentTime;
	#endif
}

void CChannel3::Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock)
{
	Clear();
	CChannel::Create( _adr, _sock );
}

// Get reliable packet from local buffer (merge fragmented packet)
bool CChannel3::GetPacketFromBuffer(CBytestream *bs)
{
	if( ReliableIn.size() == 0 )
		return false;

	bs->Clear();
	PacketList_t::iterator it = ReliableIn.begin();
	while( it != ReliableIn.end() && it->fragmented )
	{
		bs->Append( & it->data );
		it++;
	};
	if( it != ReliableIn.end() && it->idx <= LastReliableIn )
	{
		bs->Append( & it->data );
		it++;
		ReliableIn.erase( ReliableIn.begin(), it );
		return true;
	}
	bs->Clear();
	return false;
}

// This function will first return non-reliable data,
// and then one or many reliable packets - it will modify bs for that,
// so you should call it in a loop, clearing bs after each call.
bool CChannel3::Process(CBytestream *bs)
{
	bs->ResetPosToBegin();
	if( bs->GetLength() == 0 )
		return GetPacketFromBuffer(bs);

	UpdateReceiveStatistics( bs->GetLength() );
	
	// CRC16 check
	
	unsigned crc = bs->readInt(2);
	if( crc != crc16( bs->peekData( bs->GetRestLen() ).c_str(), bs->GetRestLen() ) )
	{
		iPacketsDropped++;	// Update statistics
		return GetPacketFromBuffer(bs);	// Packet from the past or from too distant future - ignore it.
	}

	// Acknowledged packets info processing

	// Read acknowledged packets indexes
	unsigned seqAck = bs->readInt(2);
	std::vector< int > seqAckList;
	while( seqAck & SEQUENCE_HIGHEST_BIT )
	{
		seqAckList.push_back( seqAck & ~ SEQUENCE_HIGHEST_BIT );
		seqAck = bs->readInt(2);
	}
	if( SequenceDiff( seqAck, LastReliableOut ) < 0 || SequenceDiff( seqAck, LastReliableOut ) > SEQUENCE_SAFE_DIST )
	{
		iPacketsDropped++;	// Update statistics
		return GetPacketFromBuffer(bs);	// Packet from the past or from too distant future - ignore it.
	}

	LastReliableOut = seqAck;

	iPacketsGood++;	// Update statistics

	// Delete acknowledged packets from buffer
	for( PacketList_t::iterator it = ReliableOut.begin(); it != ReliableOut.end(); )
	{
		bool erase = false;
		if( SequenceDiff( LastReliableOut, it->idx ) >= 0 )
			erase = true;
		for( unsigned f=0; f<seqAckList.size(); f++ )
			if( seqAckList[f] == it->idx )
				erase = true;
		if(erase)
			it = ReliableOut.erase(it);
		else
			it++;
	}

	// Calculate ping ( with LastReliableOut, not with last packet - should be fair enough )
	if( PongSequence != -1 && SequenceDiff( LastReliableOut, PongSequence ) >= 0 )
	{
		iPing = (int) ((tLX->currentTime - fLastPingSent).milliseconds());
		PongSequence = -1;
		// Traffic shaping occurs here - change DataPacketTimeout according to received ping
		// Change the value slowly, to avoid peaks
		DataPacketTimeout = ( iPing/1000.0f/DATA_PACKET_TIMEOUT_PING_COEFF + DataPacketTimeout*9.0f ) / 10.0f; 
	};

	// Processing of arrived data packets

	// Read packets info
	std::vector< int > seqList;
	std::vector< int > seqSizeList;
	unsigned seq = bs->readInt(2);
	while( seq & SEQUENCE_HIGHEST_BIT )
	{
		seqList.push_back( seq & ~ SEQUENCE_HIGHEST_BIT );
		seqSizeList.push_back( bs->readInt(2) );
		seq = bs->readInt(2);
	}
	seqList.push_back( seq );
	seqSizeList.push_back( bs->readInt(2) );

	// Put packets in buffer
	for( unsigned f=0; f<seqList.size(); f++ )
	{
		if( seqSizeList[f] == 0 ) // Last reliable packet may have size 0, if we're received non-reliable-only net packet
			continue;	// Just skip it, it's fake packet

		bool addPacket = true;
		for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end() && addPacket; it++ )
			if( it->idx == seqList[f] )
				addPacket = false;
		if( addPacket && SequenceDiff( seqList[f], LastReliableIn ) > 0 ) // Do not add packets from the past
		{	// Packet not in buffer yet - add it
			CBytestream bs1;
			bs1.writeData( bs->readData( seqSizeList[f] & ~ SEQUENCE_HIGHEST_BIT ) );
			ReliableIn.push_back( Packet_t( bs1, seqList[f], (seqSizeList[f] & SEQUENCE_HIGHEST_BIT) != 0 ) );
		}
		else	// Packet is in buffer already
		{
			// We may check here if arrived packet is the same as packet in buffer, and print errors.
			bs->Skip( seqSizeList[f] & ~ SEQUENCE_HIGHEST_BIT );
		};
	}

	// Increase LastReliableIn until the first packet that is missing from sequence
	while( true )	// I just love such constructs :P don't worry, I've put "break" inside the loop.
	{
		bool nextPacketFound = false;
		int LastReliableInInc = LastReliableIn + 1;	// Next value of LastReliableIn
		if( LastReliableInInc >= SEQUENCE_WRAPAROUND )
			LastReliableInInc = 0;
		for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end() && !nextPacketFound; it++ )
			if( it->idx == LastReliableInInc )
				nextPacketFound = true;
		if( nextPacketFound )
			LastReliableIn = LastReliableInInc;
		else
			break;
	}

	// The ReliableIn list updated - sort it
	ReliableIn.sort();
	
	if( bs->GetRestLen() > 0 )	// Non-reliable data left in this packet
		return true;	// Do not modify bs, allow user to read non-reliable data at the end of bs

	if( GetPacketFromBuffer(bs) )	// We can return some reliable packet
		return true;

	// We've got valid empty packet, or packet from future, return empty packet - bs->GetRestLen() == 0 here.
	// It is required to update server statistics, so clients that don't send packets won't timeout.
	return true;
}

void CChannel3::Transmit(CBytestream *unreliableData)
{
	UpdateReliableStreamBandwidthCounter();

	#ifdef DEBUG
	// Very simple laggy connection emulation - send next packet once per DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY
	if( DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY > 0.0f )
	{
		if( DebugSimulateLaggyConnectionSendDelay > tLX->currentTime )
			return;
		DebugSimulateLaggyConnectionSendDelay = tLX->currentTime + DEBUG_SIMULATE_LAGGY_CONNECTION_SEND_DELAY;
	}
	#endif

	CBytestream bs;
	// Add acknowledged packets indexes

	for( PacketList_t::iterator it = ReliableIn.begin(); it != ReliableIn.end(); it++ )
		if( SequenceDiff( it->idx, LastReliableIn ) > 0 ) // Packets out of sequence
			bs.writeInt( it->idx | SEQUENCE_HIGHEST_BIT, 2 );

	bs.writeInt( LastReliableIn, 2 );

	// Add reliable packet to ReliableOut buffer
	while( (int)ReliableOut.size() < MaxNonAcknowledgedPackets && !Messages.empty() && ! ReliableStreamBandwidthLimitHit() )
	{
		LastAddedToOut ++ ;
		if( LastAddedToOut >= SEQUENCE_WRAPAROUND )
			LastAddedToOut = 0;

		if( Messages.front().GetLength() > MAX_FRAGMENTED_PACKET_SIZE )
		{
			// Fragment the packet
			Messages.front().ResetPosToBegin();
			CBytestream bs;
			bs.writeData( Messages.front().readData( MAX_FRAGMENTED_PACKET_SIZE ) );
			ReliableOut.push_back( Packet_t( bs, LastAddedToOut, true ) );
			bs.Clear();
			bs.writeData( Messages.front().readData() );
			Messages.front() = bs;
		}
		else
		{
			ReliableOut.push_back( Packet_t( Messages.front(), LastAddedToOut, false ) );
			Messages.pop_front();
			while( ! Messages.empty() && 
					ReliableOut.back().data.GetLength() + Messages.front().GetLength() <= MAX_FRAGMENTED_PACKET_SIZE )
			{
				ReliableOut.back().data.Append( & Messages.front() );
				Messages.pop_front();
			}
		}
	}

	// Check if other side acknowledged packets with indexes bigger than NextReliablePacketToSend,
	// and roll NextReliablePacketToSend back to LastReliableOut.
	if( ! ReliableOut.empty() )
	{
		for( PacketList_t::iterator it = ReliableOut.begin(), it1 = it++; it != ReliableOut.end(); it1 = it++ )
		{
			if( SequenceDiff( it->idx, it1->idx ) != 1 )
				NextReliablePacketToSend = LastReliableOut;
		}
		if( ReliableOut.back().idx != LastAddedToOut )
			NextReliablePacketToSend = LastReliableOut;
	}

	// Timeout occured - other side didn't acknowledge our packets in time - re-send all of them from the first one.
	if( LastReliablePacketSent == LastAddedToOut &&
		SequenceDiff( LastReliablePacketSent, LastReliableOut ) >= MaxNonAcknowledgedPackets &&
		tLX->currentTime - fLastSent >= DataPacketTimeout )
	{
		NextReliablePacketToSend = LastReliableOut;	
	}
	
	// Add packet headers and data - send all packets with indexes from NextReliablePacketToSend and up.
	// Add older packets to the output first.
	// NextReliablePacketToSend points to the last packet.
	CBytestream packetData;
	bool unreliableOnly = true;
	bool firstPacket = true;	// Always send first packet, even if it bigger than MAX_PACKET_SIZE
								// This should not occur when packets are fragmented
	int packetIndex = LastReliableOut;
	int packetSize = 0;
	
	for( PacketList_t::iterator it = ReliableOut.begin(); it != ReliableOut.end(); it++ )
	{
		if( SequenceDiff( it->idx, NextReliablePacketToSend ) >= 0 )
		{
			if( ! CheckReliableStreamBandwidthLimit( (float)(it->data.GetLength() + 4) ) ||
				( bs.GetLength() + 4 + packetData.GetLength() + it->data.GetLength() > MAX_PACKET_SIZE-2 && !firstPacket ) )  // Substract CRC16 size
				break;

			if( !firstPacket )
			{
				bs.writeInt( packetIndex | SEQUENCE_HIGHEST_BIT, 2 );
				bs.writeInt( packetSize, 2 );
			};
			packetIndex = it->idx;
			packetSize = it->data.GetLength();
			if( it->fragmented )
				packetSize |= SEQUENCE_HIGHEST_BIT;

			firstPacket = false;
			unreliableOnly = false;
			NextReliablePacketToSend = it->idx;

			packetData.Append( &it->data );
		}
	}

	bs.writeInt( packetIndex, 2 );
	bs.writeInt( packetSize, 2 );
	
	bs.Append( &packetData );

	if( unreliableOnly )
		bs.Append(unreliableData);
	else
	{
		if( bs.GetLength() + unreliableData->GetLength() <= MAX_PACKET_SIZE-2 ) // Substract CRC16 size
			bs.Append(unreliableData);

		// If we are sending a reliable message, remember this time and use it for ping calculations
		if (PongSequence == -1)
		{
			PongSequence = NextReliablePacketToSend;
			fLastPingSent = tLX->currentTime;
		}
	}

	if( unreliableData->GetLength() == 0 &&
		LastReliablePacketSent == LastAddedToOut &&
		tLX->currentTime - fLastSent < DataPacketTimeout )
	{
		// No unreliable data to send, and we've just sent the same packet -
		// send it again after some timeout, don't flood net.
		return;
	}
	
	if( unreliableData->GetLength() == 0 && packetData.GetLength() == 0 && 
		LastReliableIn == LastReliableIn_SentWithLastPacket &&
		tLX->currentTime - fLastSent < KeepAlivePacketTimeout )
	{
		// Nothing to send really, send one empty packet per halfsecond so we won't timeout,
		// but always send first packet with acknowledges, or other side will flood
		// non-acknowledged packets for halfsecond.
		// CChannel_056b will always send packet on each frame, so we're conserving bandwidth compared to it, hehe.
		
		cOutgoingRate.addData( tLX->currentTime, 0 );
		return;
	}

	// Add CRC16 
	
	CBytestream bs1;
	bs1.writeInt( crc16( bs.peekData( bs.GetLength() ).c_str(), bs.GetLength() ), 2);
	bs1.Append(&bs);
	
	// Send the packet
	Socket->setRemoteAddress(RemoteAddr);
	bs1.Send(Socket.get());

	LastReliableIn_SentWithLastPacket = LastReliableIn;
	LastReliablePacketSent = NextReliablePacketToSend;

	UpdateTransmitStatistics( bs1.GetLength() );
}

void CChannel3::AddReliablePacketToSend(CBytestream& bs) // The same as in CChannel but without error msg
{
	if(bs.GetLength() == 0)
		return;

	Messages.push_back(bs);
	// The messages are joined in Transmit() in one bigger packet, until it will hit bandwidth limit
}

size_t CChannel3::currentReliableOutSize() {
	size_t s = 0;
	for(std::list<CBytestream>::iterator i = Messages.begin(); i != Messages.end(); ++i)
		s += i->GetLength() + 4;
	return s;
}




// CRC16 stolen from Linux kernel sources
/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
Uint16 const crc16_table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

inline Uint16 crc16_byte(Uint16 crc, const Uint8 data)
{
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}
   
/**
 * Compute the CRC-16 for the data buffer
 *
 * @param crc     previous CRC value
 * @param buffer  data pointer
 * @param len     number of bytes in the buffer
 * @return        the updated CRC value
 */
Uint16 crc16(const char * buffer, size_t len, Uint16 crc )
{
        while (len--)
		{
                crc = crc16_byte(crc, (Uint8)*buffer);
				++buffer;
		}
        return crc;
}

