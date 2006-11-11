/////////////////////////////////////////
//
//             LieroX
//
//      Copyright Auxiliary Software 2001
//
//
/////////////////////////////////////////


// Network Channel class
// Created 16/8/01
// Jason Boettcher


#ifndef __CCHANNEL_H__
#define __CCHANNEL_H__


class CChannel {
public:
	// Constructor
	CChannel() {
		iPort = 23400;
		fLastPckRecvd = 0;
		iPacketsDropped = 0;
		iPacketsGood = 0;
		iOutgoingSequence = 0;
		iReliableSequence = 0;
		iLast_ReliableSequence = 0;
		iIncomingSequence = 0;
		iIncomingAcknowledged = 0;
		iOutgoingBytes = 0;
		iIncomingBytes = 0;
		fLastSent = -9999;
	}

private:
	// Attributes
	NLaddress	RemoteAddr;
	int			iPort;
	NetworkSocket	Socket;

	
	// Bandwidth Estimation
	double		dClearTime;
	double		dRate;								// Bandwidth rate (bytes/second)

	
	// Sequencing
	int			iIncomingSequence;
	int			iIncomingAcknowledged;
	int			iIncoming_ReliableAcknowledged;		// single bit

	int			iIncoming_ReliableSequence;			// single bit, maintained local

	int			iOutgoingSequence;
	int			iReliableSequence;					// single bit
	int			iLast_ReliableSequence;				// sequence number of last send

	// Packets
	CBytestream	Message;							// Reliable message
	
	CBytestream	Reliable;							// Reliable message waiting to be acknowledged


	// For timeouts & sending
	float		fLastPckRecvd;
	float		fLastSent;


	// Statistics
	int			iPacketsDropped;
	int			iPacketsGood;
	int			iOutgoingBytes;
	int			iIncomingBytes;



public:
	// Methods
	void		Create(address_t *_adr, int _port, NetworkSocket _sock);
	void		Transmit( CBytestream *bs );
	int			Process(CBytestream *bs);
	void		Clear(void)				{ fLastPckRecvd = 0; iPort = 23400; Socket = 0;
										  iPacketsDropped = 0; iPacketsGood = 0; }


	int			getPacketLoss(void)		{ return iPacketsDropped; }
	float		getLastReceived(void)	{ return fLastPckRecvd; }
	float		getLastSent(void)		{ return fLastSent; }
	NLaddress	*getAddress(void)		{ return &RemoteAddr; }	

	// Packets
	CBytestream	*getMessageBS(void)		{ return &Message; }
	
	int			getInSeq(void)			{ return iIncomingSequence; }
	int			getOutSeq(void)			{ return iOutgoingSequence; }
	void		setInSeq(int _s)		{ iIncomingSequence = _s; }
	void		setOutSeq(int _s)		{ iOutgoingSequence = _s; }

	int			getInAck(void)			{ return iIncomingAcknowledged; }

	int			getOutoing(void)		{ return iOutgoingBytes; }
	int			getIncoming(void)		{ return iIncomingBytes; }

	NetworkSocket	getSocket(void)			{ return Socket; }
};













#endif  //  __CCHANNEL_H__
