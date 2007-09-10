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

// TODO: recode this after dropping the backward compatibility

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
	bAckRequired = false;
	iReceivedSinceLastSent = 1;
}


///////////////////
// Transmitt data, as well as handling reliable packets
void CChannel::Transmit( CBytestream *bs )
{
	CBytestream outpack;
	int SendReliable = 0;
	bool SendPacket = false;
	Uint32 r1,r2;	

	outpack.Clear();

	// If the remote side dropped the last reliable packet, re-send it
	if(iIncomingAcknowledged > iLast_ReliableSequence && iIncoming_ReliableAcknowledged != iReliableSequence)  {
		SendReliable = 1;
		SendPacket = true;
	}


	// If the reliable buffer is empty, copy the reliable message into it
	if(Reliable.GetLength() == 0) {
		bool got_reliable = false;

		if (Message.GetLength() > 0)  {
			Reliable = Message;
			Message.Clear();
			got_reliable = true;
		}

		if (tLX->fCurTime - fLastPingSent >= 1.5f)  { // Send the packet if needed for pinging and wait for ack
			iPongSequence = iOutgoingSequence;
			fLastPingSent = tLX->fCurTime;
			got_reliable = true;
		}
		
		// We got a reliable packet to send
		if (got_reliable)  {
			SendReliable = 1;
			SendPacket = true;

			// XOR the reliable sequence
			iReliableSequence ^= 1;
		}
	}

	iOutgoingSequence++;

	// Create the reliable packet header
	r1 = iOutgoingSequence | (SendReliable << 31);
	r2 = iIncomingSequence | (iIncoming_ReliableSequence << 31);

	outpack.writeInt(r1,4);
	outpack.writeInt(r2,4);


	// If were sending a reliable message, send it first
	if(SendReliable) {
		outpack.Append(&Reliable);
		if (outpack.GetLength() > 4096) // Compatibility check
			printf("WARNING: the current reliable packet could be ignored!");
		iLast_ReliableSequence = iOutgoingSequence;
	}

	// And add on the un reliable data if room in the packet struct
	if(bs)
		if (bs->GetLength() + outpack.GetLength() <= 4096)  {  // Backward compatibility
			outpack.Append(bs);
			SendPacket = SendPacket || bs->GetLength() > 0;
		}
	

	// Send the packet
	if (SendPacket || bAckRequired || tLX->fCurTime - fLastSent >= (float)(LX_CLTIMEOUT - 5))  {
		if (iReceivedSinceLastSent <= 0)  {

			// Deficite :)
			iReceivedSinceLastSent--;

			//printf("Deficite: " + itoa(-iReceivedSinceLastSent) + "\n");
		}

		SetRemoteNetAddr(Socket,&RemoteAddr);
		if (outpack.Send(Socket))  {

			// Update statistics
			iOutgoingBytes += outpack.GetLength();
			iCurrentOutgoingBytes += outpack.GetLength();
			fLastSent = tLX->fCurTime;

			iReceivedSinceLastSent = 0;

			bAckRequired = false; // Ack sent
		}
	}

	// Calculate the bytes per second
	if (tLX->fCurTime - fOutgoingClearTime >= 2.0f)  {
		fOutgoingRate = (float)iCurrentOutgoingBytes/(tLX->fCurTime - fOutgoingClearTime);
		iCurrentOutgoingBytes = 0;
		fOutgoingClearTime = tLX->fCurTime;
	}
}


///////////////////
// Process channel (after receiving data)
int CChannel::Process(CBytestream *bs)
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
	iCurrentIncomingBytes += bs->GetLength();
	if (tLX->fCurTime - fIncomingClearTime >= 2.0f)  {
		fIncomingRate = (float)iCurrentIncomingBytes/(tLX->fCurTime - fIncomingClearTime);
		iCurrentIncomingBytes = 0;
		fIncomingClearTime = tLX->fCurTime;
	}

	// Get rid of the old packets
	// Small hack: there's a bug in old clients causing the first packet being ignored and resent later
	// It caused a delay when joining (especially on high-ping servers), this hack improves it
	if((Sequence <= (Uint32)iIncomingSequence) && (Sequence != 0 && iIncomingSequence != 0)) {
		printf("Warning: Packet dropped (Sequence: %i, IncomingSequence: %i)\n", Sequence, iIncomingSequence);
		bs->Dump();

		// HINT: we can get here if our acknowledgement has been dropped
		// To be sure, we simply send the ack once more. If we are wrong, the remote side will ignore it anyway
		/*if (ReliableMessage)  { // Not needed, we (sadly) send packet every frame (backward compatibility)
			printf("HINT: re-sending ACK\n");
			bAckRequired = true;
		}*/

		return false;
	}

	iReceivedSinceLastSent++;

	
	// Check for dropped packets
	drop = Sequence - (iIncomingSequence + 1);
	if(drop > 0) {
		// Update statistics
		iPacketsDropped++;

		//printl("Packets Dropped: %d\n",PacketsDropped);
	}


	// If the outgoing reliable message has been acknowledged, clear it for more reliable messages
	if(ReliableAck == (Uint32)iReliableSequence)
		Reliable.Clear();


	// If this packet contained a reliable message, update the sequences
	iIncomingSequence = Sequence;
	iIncomingAcknowledged = SequenceAck;
	iIncoming_ReliableAcknowledged = ReliableAck;
	if(ReliableMessage)  {
		iIncoming_ReliableSequence ^= 1;
		
		// Got a message that needs an acknowledgement
		bAckRequired = true;
	}

	// Ping update
	if ((Uint32)iPongSequence <= SequenceAck)  {
		iPing = (int)((tLX->fCurTime - fLastPingSent) * 1000); 
		iPongSequence = -1;
	}


	// Update the statistics
	fLastPckRecvd = tLX->fCurTime;
	iPacketsGood++;

	iIncomingBytes += bs->GetLength();


	return true;
}
