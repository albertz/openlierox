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
	
public:
	
	CChannel() { Clear(); }
	virtual ~CChannel() { Clear(); };

	// Should be called by child class
	virtual void	Create(NetworkAddr *_adr, NetworkSocket _sock);
	virtual void	Clear();

	// Should be overridden by child class
	virtual void	Transmit( CBytestream *bs ) = 0;
	virtual bool	Process(CBytestream *bs) = 0;
	virtual void	AddReliablePacketToSend(CBytestream& bs) = 0;
	
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

	// Packets
	std::list<CBytestream>	Messages;							// Reliable message

	CBytestream	Reliable;							// Reliable message waiting to be acknowledged


	// Pinging
	int			iPongSequence;						// expected pong sequence, -1 when not pinging



public:

	// Constructor
	CChannel_056b() { Clear(); }
	
	// Methods
	void		Create(NetworkAddr *_adr, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	bool		Process(CBytestream *bs);
	void		Clear(void);

	void		AddReliablePacketToSend(CBytestream& bs);

	int			getInSeq(void)			{ return iIncomingSequence; }
	int			getOutSeq(void)			{ return iOutgoingSequence; }
	void		setInSeq(int _s)		{ iIncomingSequence = _s; }
	void		setOutSeq(int _s)		{ iOutgoingSequence = _s; }

	int			getInAck(void)			{ return iIncomingAcknowledged; }

	friend void TestCChannelRobustness();
};

void TestCChannelRobustness();

#endif  //  __CCHANNEL_H__
