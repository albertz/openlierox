#ifndef CLIENT_H
#define CLIENT_H

#include "netstream.h"

class Client : public Net_Control
{
public:
	
	Client();
	~Client();
			
protected:	
	void sendConsistencyInfo();
	
	// connection has closed
	void Net_cbConnectionClosed( Net_ConnID _id, eNet_CloseReason _reason, BitStream &_reasondata );
	
	void Net_cbDataReceived( Net_ConnID id, BitStream &data);
	
	void Net_cbConnectResult( eNet_ConnectResult res );

	// server wants to tell us about new node
	void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id );
	
	void Net_cbConnectionSpawned( Net_ConnID _id ) {}
};

#endif // _CLIENT_H_
