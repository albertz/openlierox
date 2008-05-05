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

#include <iostream>
#include <map>

#include "LieroX.h"
#include "CChannel.h"
#include "StringUtils.h"
#include "Timer.h"
#include "MathLib.h"

using namespace std;

// default max size for UDP packets for windows is 1280
// only a size of 512 is guaranteed
#define MAX_PACKET_SIZE 512
#define RELIABLE_HEADER_LEN 8

void CChannel::Clear()
{ 
	InvalidateSocketState(Socket);
	iPacketsDropped = 0; 
	iPacketsGood = 0;
	cIncomingRate.clear(); 
	cOutgoingRate.clear();
	iOutgoingBytes = 0; 
	iIncomingBytes = 0; 
	fLastSent = fLastPckRecvd = fLastPingSent = -9999; 
	iCurrentIncomingBytes = 0;
	iCurrentOutgoingBytes = 0;
};

///////////////////
// Setup the channel
void CChannel::Create(NetworkAddr *_adr, NetworkSocket _sock)
{
	Clear();
	RemoteAddr = *_adr;
	fLastPckRecvd = tLX->fCurTime;
	Socket = _sock;
	fLastSent = tLX->fCurTime-1;
	fLastPingSent = fLastSent;
	iPing = 0;
}

void CChannel_056b::Clear()
{
	CChannel::Clear();
	iOutgoingSequence = 0;
	iReliableSequence = 0;
	iLast_ReliableSequence = 0;
	iIncomingSequence = 0;
	iIncomingAcknowledged = 0;
	iOutgoingBytes = 0;
	iIncomingBytes = 0;
	fLastSent = -9999;
	bNewReliablePacket = false;
}

void CChannel_056b::Create(NetworkAddr *_adr, NetworkSocket _sock)
{
	Clear();
	CChannel::Create( _adr, _sock );
};

////////////////////
// Adds a packet to reliable queue
void CChannel_056b::AddReliablePacketToSend(CBytestream& bs)
{
	if (bs.GetLength() > MAX_PACKET_SIZE - RELIABLE_HEADER_LEN)  {
		cout
			<< "ERROR: trying to send a reliable packet of size " << bs.GetLength()
			<< " which is bigger than allowed size (" << (MAX_PACKET_SIZE - RELIABLE_HEADER_LEN)
			<< "), packet might not be sent at all!" << endl;
		Messages.push_back(bs); // Try to send it anyway, perhaps we're lucky...
		return;
	}

	// If no messages at all, add the first one
	if (Messages.size() == 0)  {
		Messages.push_back(bs);
		return;
	}

	// Some reliable messages already in queue, see if we should already split the packet
	if (bs.GetLength() + (Messages.rbegin()->GetLength()) > MAX_PACKET_SIZE)
		Messages.push_back(bs);
	else
		Messages.rbegin()->Append(&bs);
}


///////////////////
// Transmitt data, as well as handling reliable packets
void CChannel_056b::Transmit( CBytestream *bs )
{
	CBytestream outpack;
	Uint32 SendReliable = 0;
	ulong r1,r2;

	// If the remote side dropped the last reliable packet, re-send it
	if(iIncomingAcknowledged > iLast_ReliableSequence && iIncoming_ReliableAcknowledged != iReliableSequence)  {
//		printf("Remote side dropped a reliable packet, resending...\n");
		SendReliable = 1;
	}


	// We send reliable message in these cases:
	// 1. The reliable buffer is empty, we copy the reliable message into it and send it
	// 2. We need to refresh ping
	if(Reliable.GetLength() == 0 && (Messages.size() > 0 || (tLX->fCurTime - fLastPingSent >= 1.0f && iPongSequence == -1))) {
		if (Messages.size() > 0)  {
			Reliable = *Messages.begin();
			Messages.erase(Messages.begin());
			// XOR the reliable sequence
			iReliableSequence ^= 1;	// Do not XOR sequence when pinging - may cause packet loss!
		}

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
		iLast_ReliableSequence = iOutgoingSequence;

		// If we are sending a reliable message, remember this time and use it for ping calculations
		if (iPongSequence == -1)  {
			iPongSequence = iOutgoingSequence - 1;
			fLastPingSent = GetMilliSeconds();
		}

	}

	// And add on the un reliable data if room in the packet struct
	if(bs) {
		if(outpack.GetLength() + bs->GetLength() < 4096) // Backward compatibility, the old bytestream has a fixed buffer of 4096 bytes
			outpack.Append(bs);
		else
			printf("not adding unrealiable data to avoid too big packets\n");
	}


	// Send the packet
	SetRemoteNetAddr(Socket, RemoteAddr);
	outpack.Send(Socket);

	// Update statistics
	iOutgoingBytes += outpack.GetLength();
	iCurrentOutgoingBytes += outpack.GetLength();
	fLastSent = GetMilliSeconds();

	// Calculate the bytes per second
	cOutgoingRate.addData( tLX->fCurTime, outpack.GetLength() );
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

	// Got a packet (good or bad), update the received time
	fLastPckRecvd = tLX->fCurTime;

	// Read the reliable packet header
	Sequence = bs->readInt(4);
	SequenceAck = bs->readInt(4);


	// Get the reliable bits
	ReliableMessage = Sequence >> 31;
	ReliableAck = SequenceAck >> 31;

	// Get rid of the reliable bits
	Sequence &= ~(1<<31);
	SequenceAck &= ~(1<<31);

	// Calculate the bytes per second
	iIncomingBytes += bs->GetRestLen();
	iCurrentIncomingBytes += bs->GetRestLen();
	cIncomingRate.addData( tLX->fCurTime, bs->GetLength() );

	// Get rid of the old packets
	// Small hack: there's a bug in old clients causing the first packet being ignored and resent later
	// It caused a delay when joining (especially on high-ping servers), this hack improves it
	if((Sequence <= iIncomingSequence) && (Sequence != 0 && iIncomingSequence != 0)) {
//		printf("Warning: Packet dropped\n");
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
		iPing = (int)((tLX->fCurTime - fLastPingSent) * 1000);
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
};

void TestCChannelRobustness()
{
	printf("Testing CBytestream\n");
	CBytestream bsTest;
	bsTest.Test();
	printf("\n\n\n\nTesting CChannel robustness\n");
	int lagMin = 100;
	int lagMax = 300;
	int packetLoss = 10; // In percents
	int packetsPerSecond1 = 10; // One channel sends faster than another
	int packetsPerSecond2 = 1;
	int packetExtraData = 32; // Extra data in bytes to add to packet to check buffer overflows
	
	CChannel_056b c1, c2;
	NetworkSocket s1 = OpenUnreliableSocket(0);
	NetworkSocket s2 = OpenUnreliableSocket(0);
	NetworkSocket s1lag = OpenUnreliableSocket(0);
	NetworkSocket s2lag = OpenUnreliableSocket(0);
	NetworkAddr a1, a2, a1lag, a2lag;
	GetLocalNetAddr( s1, a1 );
	GetLocalNetAddr( s2, a2 );
	GetLocalNetAddr( s1lag, a1lag );
	GetLocalNetAddr( s2lag, a2lag );
	c1.Create( &a1lag, s1 );
	c2.Create( &a2lag, s2 );
	SetRemoteNetAddr( s1lag, a2 );
	SetRemoteNetAddr( s2lag, a1 );
	
	std::multimap< int, CBytestream > s1buf, s2buf;
	
	int i1=0, i2=0, i1r=0, i2r=0;
	float packetDelay1 = 1000000;
	if( packetsPerSecond1 > 0 )
		packetDelay1 = 1000.0 / packetsPerSecond1;
	float packetDelay2 = 1000000;
	if( packetsPerSecond2 > 0 )
		packetDelay2 = 1000.0 / packetsPerSecond2;
	float nextPacket1 = 0;
	float nextPacket2 = 0;
	for( int testtime=0; testtime < 100000; testtime+= 10, nextPacket1 += 10, nextPacket2 += 10 )
	{
		tLX->fCurTime = testtime / 1000.0;
		
		// Transmit number sequence and some unreliable info
		CBytestream b1, b2, b1u, b2u;

		if( nextPacket1 >= packetDelay1 )
		{
			nextPacket1 = 0;
			i1++;
			b1.writeInt(i1, 4);
			for( int f=0; f<packetExtraData; f++ )
				b1.writeByte(0);
			c1.AddReliablePacketToSend(b1);
		}
		
		if( nextPacket2 >= packetDelay2 )
		{
			nextPacket2 = 0;
			i2++;
			b2.writeInt(i2, 4);
			for( int f=0; f<packetExtraData; f++ )
				b2.writeByte(0);
			c2.AddReliablePacketToSend(b2);
		};

		c1.Transmit( &b1u );
		c2.Transmit( &b2u );
		
		b1.Clear();
		b2.Clear();
		
		b1.Read(s1lag);
		b2.Read(s2lag);
		b1.ResetPosToBegin();
		b2.ResetPosToBegin();
		
		// Add the lag
		if( b1.GetLength() != 0 )
		{
			if( GetRandomInt(100) + 1 < packetLoss )
				printf("%i: c1 sent packet - lost (%i in buf): %s\n", testtime, c1.Messages.size(), printBinary(b1.readData()).c_str() );
			else
			{
				int lag = ((testtime + lagMin + GetRandomInt(lagMax-lagMin)) / 10)*10; // Round to 10
				s1buf.insert( std::make_pair( lag, b1 ) );
				printf("%i: c1 sent packet - lag %i (%i in buf): %s\n", testtime, lag, c1.Messages.size(), printBinary(b1.readData()).c_str() );
			};
		};
		
		for( std::multimap< int, CBytestream > :: iterator it = s1buf.lower_bound(testtime);
				it != s1buf.upper_bound(testtime); it++ )
		{
			it->second.ResetPosToBegin();
			it->second.ResetPosToBegin();
			it->second.Send(s1lag);
		};

		if( b2.GetLength() != 0 )
		{
			if( GetRandomInt(100) + 1 < packetLoss )
				printf("%i: c2 sent packet - lost (%i in buf): %s\n", testtime, c2.Messages.size(), printBinary(b2.readData()).c_str() );
			else
			{
				int lag = ((testtime + lagMin + GetRandomInt(lagMax-lagMin)) / 10)*10; // Round to 10
				s2buf.insert( std::make_pair( lag, b2 ) );
				printf("%i: c2 sent packet - lag %i (%i in buf): %s\n", testtime, lag, c2.Messages.size(), printBinary(b2.readData()).c_str() );
			};
		};
		
		for( std::multimap< int, CBytestream > :: iterator it = s2buf.lower_bound(testtime);
				it != s2buf.upper_bound(testtime); it++ )
		{
			it->second.ResetPosToBegin();
			it->second.ResetPosToBegin();
			it->second.Send(s2lag);
		};
		
		// Receive and check number sequence and unreliable info
		b1.Clear();
		b2.Clear();
		
		b1.Read(s1);
		b2.Read(s2);
		b1.ResetPosToBegin();
		b2.ResetPosToBegin();
		
		if( b1.GetLength() != 0 )
		{
			printf("%i: c1 recv packet: %s\n", testtime, printBinary(b1.readData()).c_str() );
			b1.ResetPosToBegin();
			if( c1.Process( &b1 ) )
			{
				while( b1.GetRestLen() != 0 )
				{
					int i1rr = b1.readInt(4);
					printf("%i: c1 reliable packet, data %i expected %i - %s\n", testtime,
							i1rr, i1r+1, i1rr == i1r+1 ? "good" : "ERROR!" );
					i1r = i1rr;
					for( int f=0; f<packetExtraData; f++ )
						b1.readByte();
				};
			}
			else
				printf("%i: c1 cannot parse packet\n", testtime);
		};

		if( b2.GetLength() != 0 )
		{
			printf("%i: c2 recv packet: %s\n", testtime, printBinary(b2.readData()).c_str() );
			b2.ResetPosToBegin();
			if( c2.Process( &b2 ) )
			{
				while( b2.GetRestLen() != 0 )
				{
					int i2rr = b2.readInt(4);
					printf("%i: c2 reliable packet, data %i expected %i - %s\n", testtime,
							i2rr, i2r+1, i2rr == i2r+1 ? "good" : "ERROR!" );
					i2r = i2rr;
					for( int f=0; f<packetExtraData; f++ )
						b2.readByte();
				};
			}
			else
				printf("%i: c2 cannot parse packet\n", testtime);
		};
	
	};
};
