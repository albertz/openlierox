/////////////////////////////////////////
//
//             OpenLieroX
//
//   work by JasonB
//   code under LGPL
//   enhanced by Dark Charlie and Albert Zeyer
//
/////////////////////////////////////////


// Network Channel class
// Created 16/8/01
// Jason Boettcher


#ifndef __CCHANNEL_H__
#define __CCHANNEL_H__

#include "CBytestream.h"
#include <list>

// NOTE: in no part of the whole code is the Socket set 


class CChannel {
public:
	// Constructor
	CChannel() {
		Clear();
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

private:
	// Attributes
	NetworkAddr		RemoteAddr;
	int				iPort;
	NetworkSocket	Socket;
	bool			bNewReliablePacket;

	
	// Bandwidth Estimation
	float		fIncomingClearTime;
	float		fOutgoingClearTime;
	float		fIncomingRate;								// Bandwidth rate (bytes/second)
	float		fOutgoingRate;

	
	// Sequencing
	Uint32		iIncomingSequence;
	Uint32		iIncomingAcknowledged;
	Uint32		iIncoming_ReliableAcknowledged;		// single bit

	Uint32		iIncoming_ReliableSequence;			// single bit, maintained local

	Uint32		iOutgoingSequence;
	Uint32		iReliableSequence;					// single bit
	Uint32		iLast_ReliableSequence;				// sequence number of last send

	// Packets
	std::list<CBytestream>	Messages;							// Reliable message
	
	CBytestream	Reliable;							// Reliable message waiting to be acknowledged


	// For timeouts & sending
	float		fLastPckRecvd;
	float		fLastSent;

	// Pinging
	int			iPing;								// current ping
	float		fLastPingSent;
	int			iPongSequence;						// expected pong sequence, -1 when not pinging


	// Statistics
	int			iPacketsDropped;
	int			iPacketsGood;
	int			iCurrentIncomingBytes;				// how many bytes received since last bandwidth calculation
	int			iCurrentOutgoingBytes;				// how many bytes sent since last bandwidth calculation
	int			iOutgoingBytes;
	int			iIncomingBytes;



public:
	// Methods
	void		Create(NetworkAddr *_adr, int _port, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	bool		Process(CBytestream *bs);
	void		Clear(void)				{ fLastPckRecvd = 0;
										  iPort = LX_PORT; InvalidateSocketState(Socket);
										  iPacketsDropped = 0; iPacketsGood = 0; bNewReliablePacket = false; }


	int			getPacketLoss(void)		{ return iPacketsDropped; }
	float		getLastReceived(void)	{ return fLastPckRecvd; }
	float		getLastSent(void)		{ return fLastSent; }
	NetworkAddr	getAddress(void)		{ return RemoteAddr; }
	bool		gotNewReliablePacket()	{ return bNewReliablePacket; }

	// Packets
	void	AddReliablePacketToSend(CBytestream& bs);
	
	int		getInSeq(void)			{ return iIncomingSequence; }
	int		getOutSeq(void)			{ return iOutgoingSequence; }
	void	setInSeq(int _s)		{ iIncomingSequence = _s; }
	void	setOutSeq(int _s)		{ iOutgoingSequence = _s; }

	int	getInAck(void)			{ return iIncomingAcknowledged; }

	int	getOutoing(void)		{ return iOutgoingBytes; }
	int	getIncoming(void)		{ return iIncomingBytes; }

	int		getPing()			{ return iPing; }
	void	setPing(int _p)		{ iPing = _p; }

	float getIncomingRate()		{ return fIncomingRate; }
	float getOutgoingRate()		{ return fOutgoingRate; }

	NetworkSocket	getSocket(void)			{ return Socket; }
};













#endif  //  __CCHANNEL_H__
