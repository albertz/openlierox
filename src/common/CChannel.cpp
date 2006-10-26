/////////////////////////////////////////
//
//             LieroX
//
//      Copyright Auxiliary Software 2001
//
//
/////////////////////////////////////////


// Network Channel class
// Created 16/6/01
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Setup the channel
void CChannel::Create(address_t *_adr, int _port, NLsocket _sock)
{
	RemoteAddr = *_adr;
	iPort = _port;
	fLastPckRecvd = tLX->fCurTime;
	Socket = _sock;
	dRate = 1.0f/2500.0f;			// 2500 bytes per second
	Reliable.Clear();
	Message.Clear();
	iPacketsDropped=0;
	iPacketsGood=0;
	fLastSent = tLX->fCurTime-1;

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
	int SendReliable = false;
	ulong r1,r2;	

	outpack.Clear();

	// Quick Hack
	/*if(Message.Overflowed()) {
		conprintf("Overflow!\n");
	}*/

	// If the remote side dropped the last reliable packet, re-send it
	if(iIncomingAcknowledged > iLast_ReliableSequence && iIncoming_ReliableAcknowledged != iReliableSequence)
		SendReliable = true;


	// If the reliable buffer is empty, copy the reliable message into it
	if(Reliable.GetLength() == 0 && Message.GetLength() > 0) {
		Reliable = Message;
		Message.Clear();
		
		// We got a reliable packet to send
		SendReliable = true;

		// XOR the reliable sequence
		iReliableSequence ^= 1;
	}


	// Create the reliable packet header
	r1 = iOutgoingSequence | (SendReliable<<31);
	r2 = iIncomingSequence | (iIncoming_ReliableSequence<<31);

	iOutgoingSequence++;
	
	outpack.writeInt(r1,4);
	outpack.writeInt(r2,4);

	
	// If were sending a reliable message, send it first
	if(SendReliable) {
		outpack.Append(&Reliable);
		iLast_ReliableSequence = iOutgoingSequence;
	}

	// And add on the un reliable data if room in the packet struct
	if(bs) {
		if(outpack.GetLength() + bs->GetLength() < MAX_DATA)
			outpack.Append(bs);
	}
	

	// Send the packet

	nlSetRemoteAddr(Socket,&RemoteAddr);
	outpack.Send(Socket);

	iOutgoingBytes += outpack.GetLength();
	fLastSent = tLX->fCurTime;


	// TODO: Setup the clear time for the choke

	// TODO: Calculate the bandwidth
}


///////////////////
// Process channel (after receiving data)
int CChannel::Process(CBytestream *bs)
{
	ulong Sequence, SequenceAck;
	ulong ReliableAck, ReliableMessage;	
	int drop;

	// Start from the beginning of the packet
	bs->Reset();

	// Read the reliable packet header
	Sequence = bs->readInt(4);
	SequenceAck = bs->readInt(4);


	// Get the reliable bits
	ReliableMessage = Sequence >> 31;
	ReliableAck = SequenceAck >> 31;

	// Get rid of the reliable bits
	Sequence &= ~(1<<31);	
	SequenceAck &= ~(1<<31);


	// TODO: Get rate estimation

	// Get rid of the old packets
	if(Sequence <= (ulong)iIncomingSequence) {
		//Con_Printf(CNC_WARNING,"Warning: Packet dropped");
		return false;
	}

	
	// Check for dropped packets
	drop = Sequence - (iIncomingSequence+1);
	//iPacketDrop = drop;
	if(drop>0) {
		// Update statistics
		iPacketsDropped++;

		//printl("Packets Dropped: %d\n",PacketsDropped);
	}


	// If the outgoing reliable message has been acknowledged, clear it for more reliable messages
	if(ReliableAck == (ulong)iReliableSequence)
		Reliable.Clear();


	// If this packet contained a reliable message, update the sequences
	iIncomingSequence = Sequence;
	iIncomingAcknowledged = SequenceAck;
	iIncoming_ReliableAcknowledged = ReliableAck;
	if(ReliableMessage)
		iIncoming_ReliableSequence ^= 1;


	// Update the statistics
	fLastPckRecvd = tLX->fCurTime;
	iPacketsGood++;

	iIncomingBytes += bs->GetLength();


	return true;
}
