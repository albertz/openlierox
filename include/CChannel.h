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
		size_t dindex = 0;
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
	
protected:
	
	// Attributes
	NetworkAddr		RemoteAddr;
	NetworkSocket	Socket;

	// For timeouts & sending
	float			fLastPckRecvd;
	float			fLastSent;

	// Bandwidth Estimation
	Rate<100,2000>	cIncomingRate;
	Rate<100,2000>	cOutgoingRate;

	// Pinging
	int				iPing;								// current ping
	float			fLastPingSent;

	// Statistics
	size_t			iPacketsDropped;
	size_t			iPacketsGood;
	size_t			iCurrentIncomingBytes;				// how many bytes received since last bandwidth calculation
	size_t			iCurrentOutgoingBytes;				// how many bytes sent since last bandwidth calculation
	size_t			iOutgoingBytes;
	size_t			iIncomingBytes;

	// Packets
	std::list<CBytestream>	Messages;					// List of reliable messages to be sent
	
public:
	
	CChannel() { Clear(); }
	virtual ~CChannel() { Clear(); };

	// Should be called by child class
	virtual void	Create(NetworkAddr *_adr, NetworkSocket _sock);
	virtual void	Clear();

	// Should be overridden by child class
	// This function will send reliable data from AddReliablePacketToSend() plus unreliable data in bs argument
	virtual void	Transmit( CBytestream *unreliableData ) = 0;
	// This function behaves differently for CChannel_UberPwnyReliable, see below
	// It should return empty data from time to time when channel is inactive, so clients won't timeout.
	virtual bool	Process( CBytestream *bs ) = 0;
	virtual void	AddReliablePacketToSend(CBytestream& bs);
	
	size_t			getPacketLoss(void)		{ return iPacketsDropped; }
	float			getLastReceived(void)	{ return fLastPckRecvd; }
	float			getLastSent(void)		{ return fLastSent; }
	NetworkAddr		getAddress(void)		{ return RemoteAddr; }

	size_t			getOutoing(void)		{ return iOutgoingBytes; }
	size_t			getIncoming(void)		{ return iIncomingBytes; }

	int				getPing()			{ return iPing; }
	void			setPing(int _p)		{ iPing = _p; }

	float 			getIncomingRate()		{ return cIncomingRate.getRate(); }
	float 			getOutgoingRate()		{ return cOutgoingRate.getRate(); }

	NetworkSocket	getSocket(void)			{ return Socket; }

};

// CChannel for LX 0.56b implementation - LOSES PACKETS, and that cannot be fixed.
class CChannel_056b: public CChannel {
	
private:

	bool		bNewReliablePacket;

	// Sequencing
	Uint32		iIncomingSequence;
	Uint32		iIncomingAcknowledged;
	Uint32		iIncoming_ReliableAcknowledged;		// single bit

	Uint32		iIncoming_ReliableSequence;			// single bit, maintained local

	Uint32		iOutgoingSequence;
	Uint32		iReliableSequence;					// single bit
	Uint32		iLast_ReliableSequence;				// sequence number of last send

	CBytestream	Reliable;							// Reliable message waiting to be acknowledged

	// Pinging
	int			iPongSequence;						// expected pong sequence, -1 when not pinging



public:

	// Constructor
	CChannel_056b() { Clear(); }
	
	// Methods
	void		Create(NetworkAddr *_adr, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	// This function just skips header in bs, non-reliable data is at the end of the stream, bs not modified.
	bool		Process( CBytestream *bs );
	void		Clear(void);

	int			getInSeq(void)			{ return iIncomingSequence; }
	int			getOutSeq(void)			{ return iOutgoingSequence; }
	void		setInSeq(int _s)		{ iIncomingSequence = _s; }
	void		setOutSeq(int _s)		{ iOutgoingSequence = _s; }

	int			getInAck(void)			{ return iIncomingAcknowledged; }

	friend void TestCChannelRobustness();
};

// Reliable and less messy CChannel implementation by pelya.
class CChannel_UberPwnyReliable: public CChannel {
	
private:
	typedef std::list< std::pair< CBytestream, int > > PacketList_t;
	PacketList_t	ReliableOut;		// Reliable messages waiting to be acknowledged, with their ID-s
	int				LastReliableOut;	// Last acknowledged packet from remote side

	PacketList_t	ReliableIn;			// Reliable messages from the other side, not sorted, with their ID-s
	int				LastReliableIn;		// Last packet acknowledged by me
	
	// Pinging
	int				PongSequence;						// expected pong sequence, -1 when not pinging
	
	// Misc vars to shape packet flow
	int				LastReliableIn_SentWithLastPacket;	// Required to check if we need to send empty packet with acknowledges
	
	bool GetPacketFromBuffer(CBytestream *bs);

public:

	// Constructor
	CChannel_UberPwnyReliable() { Clear(); }
	
	// Methods
	void		Create(NetworkAddr *_adr, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	// This function will first return non-reliable data,
	// and then one or many reliable packets - it will modify bs for that,
	// so you should call it in a loop, clearing bs after each call.
	bool		Process( CBytestream *bs );
	void		Clear();

	friend void TestCChannelRobustness();
};

void TestCChannelRobustness();

#endif  //  __CCHANNEL_H__
