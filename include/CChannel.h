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

template< int AMOUNT, int TIMERANGEMS >
class Rate {
private:
	size_t buckets[AMOUNT];
	int curIndex;
	float curIndexTime;

public:
	void clear() { curIndex = -1; curIndexTime = 0; memset(buckets, 0, sizeof(buckets)); }
	Rate() { clear(); }

	float timeRange() { return (float)TIMERANGEMS / 1000.0f; }

	void addData(float curtime, size_t amount) {
		// calc diff of oldindex to newindex
		size_t dindex;
		if(curIndex >= 0) {
			dindex = (int) ( (float)AMOUNT * ((curtime - curIndexTime) / timeRange()) );
			if(dindex >= AMOUNT) { // our data is too old, just clear it
				clear();
			}
		}
		if(curIndex == -1) {
			curIndex = 0;
			dindex = 0;
			curIndexTime = curtime;
		}

		// reset the buckets in between
		for(size_t i = 0; i < dindex; i++) {
			buckets[ (curIndex + i + 1) % AMOUNT ] = 0;
		}

		// do updates
		curIndex += dindex; curIndex %= AMOUNT; curIndexTime += (float)dindex * timeRange() / (float)AMOUNT;

		// add data
		buckets[curIndex] += amount;
	}

	float getRate() {
		size_t sum = 0;
		for(int i = 0; i < AMOUNT; i++)
			sum += buckets[i];
		return (float)sum / timeRange();
	}

};



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
	Rate<100,2000>	cIncomingRate;
	Rate<100,2000>	cOutgoingRate;

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
	size_t		iPacketsDropped;
	size_t		iPacketsGood;
	size_t		iCurrentIncomingBytes;				// how many bytes received since last bandwidth calculation
	size_t		iCurrentOutgoingBytes;				// how many bytes sent since last bandwidth calculation
	size_t		iOutgoingBytes;
	size_t		iIncomingBytes;



public:
	// Methods
	void		Create(NetworkAddr *_adr, int _port, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	bool		Process(CBytestream *bs);
	void		Clear(void)				{ fLastPckRecvd = 0;
										  iPort = LX_PORT; InvalidateSocketState(Socket);
										  iPacketsDropped = 0; iPacketsGood = 0; bNewReliablePacket = false;
										  cIncomingRate.clear(); cOutgoingRate.clear(); }


	size_t		getPacketLoss(void)		{ return iPacketsDropped; }
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

	size_t	getOutoing(void)		{ return iOutgoingBytes; }
	size_t	getIncoming(void)		{ return iIncomingBytes; }

	int		getPing()			{ return iPing; }
	void	setPing(int _p)		{ iPing = _p; }

	float getIncomingRate()		{ return cIncomingRate.getRate(); }
	float getOutgoingRate()		{ return cOutgoingRate.getRate(); }

	NetworkSocket	getSocket(void)			{ return Socket; }
};













#endif  //  __CCHANNEL_H__
