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

#include "LieroX.h"
#include "CChannel.h"
#include "StringUtils.h"
#include "Timer.h"

using namespace std;

// default max size for UDP packets for windows is 1280
// only a size of 512 is guaranteed
#define MAX_PACKET_SIZE 512
#define RELIABLE_HEADER_LEN 8

///////////////////
// Setup the channel
void CChannel::Create(NetworkAddr *_adr, int _port, NetworkSocket _sock)
{
	RemoteAddr = *_adr;
	iPort = _port;
	fLastPckRecvd = tLX->fCurTime;
	Socket = _sock;
	Reliable.Clear();
	Messages.clear();
	iPacketsDropped=0;
	iPacketsGood=0;
	fLastSent = tLX->fCurTime-1;
	fLastPingSent = fLastSent;
	fIncomingClearTime = -9999;
	fOutgoingClearTime = -9999;
	fIncomingRate = 0;
	fOutgoingRate = 0;
	iCurrentIncomingBytes = 0;
	iCurrentOutgoingBytes = 0;
	iPongSequence = -1;
	iPing = 0;
	bNewReliablePacket = false;

	// Clear the sequences
	iIncomingSequence = 0;
	iIncomingAcknowledged = 0;
	iIncoming_ReliableAcknowledged = 0;
	iIncoming_ReliableSequence = 0;
	iOutgoingSequence = 0;
	iReliableSequence = 0;
	iLast_ReliableSequence = 0;
	iOutgoingBytes = 0;
	iIncomingBytes = 0;
}


////////////////////
// Adds a packet to reliable queue
void CChannel::AddReliablePacketToSend(CBytestream& bs)
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
void CChannel::Transmit( CBytestream *bs )
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
		}
		
		// We got a reliable packet to send
		SendReliable = 1;

		// XOR the reliable sequence
		iReliableSequence ^= 1;
	}

	// Don't flood packets
	if (!SendReliable)
		if (GetMilliSeconds() - fLastSent <= 1.0f/60.0f)
			return;

	
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
	}
	

	// Send the packet
	SetRemoteNetAddr(Socket, RemoteAddr);
	outpack.Send(Socket);

	// Update statistics
	iOutgoingBytes += outpack.GetLength();
	iCurrentOutgoingBytes += outpack.GetLength();
	fLastSent = GetMilliSeconds();

	// Calculate the bytes per second
	if (tLX->fCurTime - fOutgoingClearTime >= 2.0f)  {
		fOutgoingRate = (float)iCurrentOutgoingBytes/(GetMilliSeconds() - fOutgoingClearTime);
		iCurrentOutgoingBytes = 0;
		fOutgoingClearTime = GetMilliSeconds();
	}
}


///////////////////
// Process channel (after receiving data)
bool CChannel::Process(CBytestream *bs)
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
	if (tLX->fCurTime - fIncomingClearTime >= 2.0f)  {
		fIncomingRate = (float)iCurrentIncomingBytes/(GetMilliSeconds() - fIncomingClearTime);
		iCurrentIncomingBytes = 0;
		fIncomingClearTime = GetMilliSeconds();
	}

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
		iPing = (int)((GetMilliSeconds() - fLastPingSent) * 1000); // TODO: this highly depends on FPS, dunno why, but it's the reason why it is wrong
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
