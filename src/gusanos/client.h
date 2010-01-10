#ifndef CLIENT_H
#define CLIENT_H

#include "netstream.h"
//#include <string>

struct PlayerOptions;
class CWorm;

class Client : public Net_Control
{
public:
	
	Client();
	~Client();
			
protected:	
	void sendConsistencyInfo();
	
	// connection has closed
	void Net_cbConnectionClosed( Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata );
	
	void Net_cbDataReceived( Net_ConnID id, Net_BitStream &data);
	
	void Net_cbConnectResult( eNet_ConnectResult res );

	// server wants to tell us about new node
	void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id );
	
	void Net_cbConnectionSpawned( Net_ConnID _id ) {}
};

#endif // _CLIENT_H_
