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


#include "LieroX.h"
#include "CChannel.h"
#include "StringUtils.h"

///////////////////
// Setup the channel
void CChannel::Create(NetworkAddr *_adr, int _port, NetworkSocket _sock)
{
	RemoteAddr = *_adr;
	iPort = _port;
	fLastPckRecvd = tLX->fCurTime;
	Socket = _sock;
	Reliable.Clear();
	Message.Clear();
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


///////////////////
// Transmitt data, as well as handling reliable packets
void CChannel::Transmit( CBytestream *bs )
{
	CBytestream outpack;
	Uint32 SendReliable = 0;
	ulong r1,r2;	

	outpack.Clear();

	// If the remote side dropped the last reliable packet, re-send it
	if(iIncomingAcknowledged > iLast_ReliableSequence && iIncoming_ReliableAcknowledged != iReliableSequence)  {
		printf("Remote side dropped a reliable packet, resending...\n");
		SendReliable = 1;
	}


	// If the reliable buffer is empty, copy the reliable message into it
	if(Reliable.GetLength() == 0 && Message.GetLength() > 0) {
		Reliable = Message;
		Message.Clear();
		
		// We got a reliable packet to send
		SendReliable = 1;

		// XOR the reliable sequence
		iReliableSequence ^= 1;
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
			iPongSequence = iOutgoingSequence;
			fLastPingSent = tLX->fCurTime;
		}

	}

	// And add on the un reliable data if room in the packet struct
	if(bs) {
		if(outpack.GetLength() + bs->GetLength() < 4096) // Backward compatibility
			outpack.Append(bs);
	}
	

	// Send the packet
	SetRemoteNetAddr(Socket, &RemoteAddr);
	outpack.Send(Socket);

	// Update statistics
	iOutgoingBytes += outpack.GetLength();
	iCurrentOutgoingBytes += outpack.GetLength();
	fLastSent = tLX->fCurTime;

	// Calculate the bytes per second
	if (tLX->fCurTime - fOutgoingClearTime >= 2.0f)  {
		fOutgoingRate = (float)iCurrentOutgoingBytes/(tLX->fCurTime - fOutgoingClearTime);
		iCurrentOutgoingBytes = 0;
		fOutgoingClearTime = tLX->fCurTime;
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
	iIncomingBytes += bs->GetLength();
	iCurrentIncomingBytes += bs->GetLength();
	if (tLX->fCurTime - fIncomingClearTime >= 2.0f)  {
		fIncomingRate = (float)iCurrentIncomingBytes/(tLX->fCurTime - fIncomingClearTime);
		iCurrentIncomingBytes = 0;
		fIncomingClearTime = tLX->fCurTime;
	}

	// Get rid of the old packets
	// Small hack: there's a bug in old clients causing the first packet being ignored and resent later
	// It caused a delay when joining (especially on high-ping servers), this hack improves it
	if((Sequence <= iIncomingSequence) && (Sequence != 0 && iIncomingSequence != 0)) {
		printf("Warning: Packet dropped\n");
		bs->Dump();
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
	if(SequenceAck >= iPongSequence)  {
		iPongSequence = -1;  // Ready for new pinging
		iPing = (int)((tLX->fCurTime - fLastPingSent) * 1000);
	}


	// If this packet contained a reliable message, update the sequences
	iIncomingSequence = Sequence;
	iIncomingAcknowledged = SequenceAck;
	iIncoming_ReliableAcknowledged = ReliableAck;
	if(ReliableMessage)
		iIncoming_ReliableSequence ^= 1;


	// Update the statistics
	fLastPckRecvd = tLX->fCurTime;
	iPacketsGood++;


	return true;
}
