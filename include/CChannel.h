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

#include <list>
#include "CBytestream.h"
#include "olx-types.h"
#include "Networking.h"

template< int AMOUNT, int TIMERANGEMS, typename _Amount = size_t >
class Rate {
private:
	_Amount buckets[AMOUNT];
	int curIndex;
	AbsTime curIndexTime;

public:
	void clear() { curIndex = -1; curIndexTime = AbsTime(); memset(buckets, 0, sizeof(buckets)); }
	Rate() { clear(); }

	TimeDiff timeRange() { return TimeDiff(TIMERANGEMS); }

	void addData(const AbsTime& curtime, _Amount amount) {
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
		curIndex += dindex; curIndex %= AMOUNT; curIndexTime += TimeDiff((float)dindex * timeRange().seconds() / (float)AMOUNT);

		// add data
		buckets[curIndex] += amount;
	}

	float getRate() {
		_Amount sum = 0;
		for(int i = 0; i < AMOUNT; i++)
			sum += buckets[i];
		return (float)sum / timeRange().seconds();
	}
	
	float getRate(int timerange) {
		if(curIndex == -1)
			return 0;

		if(timerange >= TIMERANGEMS)
			return getRate(); // we cannot get a smaller range
		
		// calc diff of oldindex to newindex
		size_t dindex = (int) ( (float)AMOUNT * (float(timerange) / float(TIMERANGEMS)) );
		
		_Amount sum = 0;
		size_t startIndex = AMOUNT + curIndex - dindex;
		for(size_t i = 0; i < dindex; i++) {
			sum += buckets[ (startIndex + i) % AMOUNT ];
		}
		
		return float(sum) / (float(timerange) / 1000.0f);
	}

};


class CChannel {
	
protected:
	
	// Attributes
	NetworkAddr		RemoteAddr;
	SmartPointer<NetworkSocket>	Socket;

	// For timeouts & sending
	AbsTime			fLastPckRecvd;
	AbsTime			fLastSent;

	// Bandwidth Estimation
	Rate<100,2000>	cIncomingRate;
	Rate<100,2000>	cOutgoingRate;

	// Pinging
	int				iPing;								// current ping
	AbsTime			fLastPingSent;

	// Statistics
	size_t			iPacketsDropped;
	size_t			iPacketsGood;
	size_t			iCurrentIncomingBytes;				// how many bytes received since last bandwidth calculation
	size_t			iCurrentOutgoingBytes;				// how many bytes sent since last bandwidth calculation
	size_t			iOutgoingBytes;
	size_t			iIncomingBytes;

	void			UpdateTransmitStatistics( int sentDataSize );
	void			UpdateReceiveStatistics( int receivedDataSize );

	// Packets
	std::list<CBytestream>	Messages;					// List of reliable messages to be sent
	
	// Bandwidth limiter - for reliable stream only, it won't count unreliable data - you should limit it outside of CChannel
	// Each time Transmit() it increases BandwidthCounter for (CurTime - LastUpdate) * BandwidthLimit
	// If the next reliable packet is smaller than BandwidthCounter it will send it and decrease BandwidthCounter
	// Also if bandwidth limit is hit it will accumulate and send bigger packets, controlled by MaxPacketRate var.
	// And if it is not hit, CChannel will act as usual - send as much data as possible, right when data to send is available
	// TODO: Albert you may wish to put here your advanced bandwidth calculating class
	// Also I love long long names for variables
	float			ReliableStreamBandwidthLimit; // In bytes/sec
	float			ReliableStreamMaxPacketRate; // In packets/sec
	float			ReliableStreamBandwidthCounter; // In bytes
	float			ReliableStreamBandwidthCounterMaxValue; // In bytes, to prevent ReliableDataBandwidthCounter to accumulate gigabyte of bandwidth and send it in single Transmit()
	AbsTime			ReliableStreamLastSentTime;
	AbsTime			ReliableStreamBandwidthCounterLastUpdate;
	
	void			UpdateReliableStreamBandwidthCounter(); // Should be called in the beginning of each Transmit()
	
	
public:
	
	CChannel() { Clear(); }
	virtual ~CChannel() { Clear(); };

	// Should be called by child class
	virtual void	Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock);
	virtual void	Clear();

	// Should be overridden by child class
	// This function will send reliable data from AddReliablePacketToSend() plus unreliable data in bs argument
	virtual void	Transmit( CBytestream *unreliableData ) = 0;
	// This function behaves differently for CChannel2, see below
	// It should return empty data from time to time when channel is inactive, so clients won't timeout.
	virtual bool	Process( CBytestream *bs ) = 0;
	virtual void	AddReliablePacketToSend(CBytestream& bs); // Common for CChannel_056b and CChannel2
	
	size_t			getPacketLoss()		{ return iPacketsDropped; }
	AbsTime			getLastReceived()	{ return fLastPckRecvd; }
	AbsTime			getLastSent()		{ return fLastSent; }
	NetworkAddr		getAddress()		{ return RemoteAddr; }
	// Returns true if CChannel has sent all pending packets, and got acknowledges about them.
	// That includes only packets after last Transmit() call, 
	// calling AddReliablePacketToSend() won't change the value this function returns.
	virtual bool	getBufferEmpty() = 0;
	// Not the same as "! getBufferEmpty()" for new CChannel implementation - it can buffer up multiple packets.
	virtual bool	getBufferFull() = 0;

	size_t			getOutgoing()		{ return iOutgoingBytes; }
	size_t			getIncoming()		{ return iIncomingBytes; }

	int				getPing()			{ return iPing; }
	void			setPing(int _p)		{ iPing = _p; }

	float 			getIncomingRate()		{ return cIncomingRate.getRate(); }
	float 			getOutgoingRate()		{ return cOutgoingRate.getRate(); }
	float 			getOutgoingRate(float timeRange)		{ return cOutgoingRate.getRate((int)(timeRange * 1000.0f)); }

	bool			ReliableStreamBandwidthLimitHit(); // Should we wait and accumulate packets instead of sending many small packets immediately
	bool			CheckReliableStreamBandwidthLimit( float dataSizeToSend, bool doUpdate = true ); // Returns true if data is allowed to send, and decreases counter value if doUpdate
	float			MaxDataPossibleToSendInstantly();
	
	virtual size_t	currentReliableOutSize();
	virtual size_t	maxPossibleAdditionalReliableOutPackages();
	
	SmartPointer<NetworkSocket>	getSocket()			{ return Socket; }
	
	virtual void	recheckSeqs() {} // Implemented only in CChannel_056b, not required for others

	void			LimitReliableStreamBandwidth( float BandwidthLimit, float MaxPacketRate = 5.0f, float BandwidthCounterMaxValue = 512.0f );
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
	void		Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock);
	void		Transmit( CBytestream *bs );
	// This function just skips header in bs, non-reliable data is at the end of the stream, bs not modified.
	bool		Process( CBytestream *bs );
	void		Clear();

	bool		getBufferEmpty()	{ return Reliable.GetLength() == 0; };
	bool		getBufferFull()		{ return ! getBufferEmpty(); };
	int			getInSeq()			{ return iIncomingSequence; }
	int			getOutSeq()			{ return iOutgoingSequence; }
	void		setInSeq(int _s)		{ iIncomingSequence = _s; }
	void		setOutSeq(int _s)		{ iOutgoingSequence = _s; }

	int			getInAck()			{ return iIncomingAcknowledged; }

	void		recheckSeqs();

	friend void TestCChannelRobustness();
};


// Reliable and less messy CChannel implementation by pelya.
class CChannel2: public CChannel {
	
private:
	typedef std::list< std::pair< CBytestream, int > > PacketList_t;
	PacketList_t	ReliableOut;		// Reliable messages waiting to be acknowledged, with their ID-s, sorted
	int				LastReliableOut;	// Last acknowledged packet from remote side
	int				LastAddedToOut;		// Last packet that was added to ReliableOut buf

	PacketList_t	ReliableIn;			// Reliable messages from the other side, not sorted, with their ID-s
	int				LastReliableIn;		// Last packet acknowledged by me
	
	// Pinging
	int				PongSequence;						// expected pong sequence, -1 when not pinging
	
	// Misc vars to shape packet flow
	int				LastReliableIn_SentWithLastPacket;	// Required to check if we need to send empty packet with acknowledges
	int				LastReliablePacketSent;				// To make packet flow smooth
	int				NextReliablePacketToSend;			// To make packet flow smooth
	
	// Constants to shape packet flow - in the future we may want to change them dynamically from connection characteristics
	
	// How much to wait before sending another empty keep-alive packet, sec (doesn't really matter much).
	float			KeepAlivePacketTimeout;
	// How much to wait before sending data packet again, sec - 
	// if packets rarely get lost over net it will decrease bandwidth dramatically, for little lag tradeoff.
	// Set to 0 to flood net with packets instantly as in CChannel_056b.
	// If any new data available to send, or unreliable data present, packet is sent anyway.
	float			DataPacketTimeout;
	// Max amount of packets that can be flying through the net at the same time.
	int				MaxNonAcknowledgedPackets;

	#ifdef DEBUG
	AbsTime			DebugSimulateLaggyConnectionSendDelay; // Self-explanatory
	#endif

	bool			GetPacketFromBuffer(CBytestream *bs);

public:

	// Constructor
	CChannel2() { Clear(); }
	
	// Methods
	void		Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock);
	void		Transmit( CBytestream *bs );
	// This function will first return non-reliable data,
	// and then one or many reliable packets - it will modify bs for that,
	// so you should call it in a loop, clearing bs after each call.
	bool		Process( CBytestream *bs );
	void		Clear();

	bool		getBufferEmpty()	{ return ReliableOut.empty(); };
	bool		getBufferFull()		{ return (int)ReliableOut.size() >= MaxNonAcknowledgedPackets; };

	friend void TestCChannelRobustness();
};

// The copy of previous one, but with CRC16 added, plus packets > 512 bytes got splitted into smaller ones.
// UDP packets contain checksum which is optional, and can be stripped if lesser than 512 bytes,
// and I thought UDP cannot contain wrong data when created previous one
class CChannel3: public CChannel {
	
private:
	struct Packet_t
	{
		CBytestream data;
		int idx;
		bool fragmented;

		Packet_t( const CBytestream & d, int i, bool f ): data(d), idx(i), fragmented(f) { };
		
		bool operator < ( const Packet_t & p ) const; // For sorting
	};
	typedef std::list< Packet_t > PacketList_t;
	PacketList_t	ReliableOut;		// Reliable messages waiting to be acknowledged, with their ID-s, sorted
	int				LastReliableOut;	// Last acknowledged packet from remote side
	int				LastAddedToOut;		// Last packet that was added to ReliableOut buf

	PacketList_t	ReliableIn;			// Reliable messages from the other side, not sorted, with their ID-s
	int				LastReliableIn;		// Last packet acknowledged by me
	
	// Pinging
	int				PongSequence;						// expected pong sequence, -1 when not pinging
	
	// Misc vars to shape packet flow
	int				LastReliableIn_SentWithLastPacket;	// Required to check if we need to send empty packet with acknowledges
	int				LastReliablePacketSent;				// To make packet flow smooth
	int				NextReliablePacketToSend;			// To make packet flow smooth
	
	// Constants to shape packet flow - in the future we may want to change them dynamically from connection characteristics
	
	// How much to wait before sending another empty keep-alive packet, sec (doesn't really matter much).
	float			KeepAlivePacketTimeout;
	// How much to wait before sending data packet again, sec - 
	// if packets rarely get lost over net it will decrease bandwidth dramatically, for little lag tradeoff.
	// Set to 0 to flood net with packets instantly as in CChannel_056b.
	// If any new data available to send, or unreliable data present, packet is sent anyway.
	float			DataPacketTimeout;
	// Max amount of packets that can be flying through the net at the same time.
	int				MaxNonAcknowledgedPackets;

	#ifdef DEBUG
	AbsTime			DebugSimulateLaggyConnectionSendDelay; // Self-explanatory
	#endif

	bool			GetPacketFromBuffer(CBytestream *bs);

public:

	// Constructor
	CChannel3() { Clear(); }
	
	// Methods
	void		Create(const NetworkAddr& _adr, const SmartPointer<NetworkSocket>& _sock);
	void		Transmit( CBytestream *bs );
	// This function will first return non-reliable data,
	// and then one or many reliable packets - it will modify bs for that,
	// so you should call it in a loop, clearing bs after each call.
	bool		Process( CBytestream *bs );
	void		Clear();

	bool		getBufferEmpty()	{ return ReliableOut.empty(); };
	bool		getBufferFull()		{ return (int)ReliableOut.size() >= MaxNonAcknowledgedPackets; };
	size_t	currentReliableOutSize();
	size_t	maxPossibleAdditionalReliableOutPackages();

	void		AddReliablePacketToSend(CBytestream& bs); // The same as in CChannel but without error msg

	friend void TestCChannelRobustness();
};

void TestCChannelRobustness();

#endif  //  __CCHANNEL_H__
