#ifndef SERVER_H
#define SERVER_H

#include "game/WormInputHandler.h"

#include "netstream.h"
#include <map>
#include <boost/shared_ptr.hpp>

class Server : public Net_Control
{
private:
	// Flag to refuse connection while waiting for all players to aknowledge disconnection
	bool m_preShutdown;
	std::map<unsigned int, boost::shared_ptr<CWormInputHandler::Stats> > savedScores;
	
public:
	Server( int _udpport );
	~Server();
	
	void preShutdown() { m_preShutdown = true; }
	
protected:
		
	// called on incoming connections
	bool Net_cbConnectionRequest( Net_ConnID _id, Net_BitStream &_request, Net_BitStream &_reply );
	// called when incoming connection has been established
	void Net_cbConnectionSpawned( Net_ConnID _id );
	// called when a connection closed
	void Net_cbConnectionClosed( Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata );
	
	bool Net_cbNetRequest( Net_ConnID _id, Net_U8 _requested_level, Net_BitStream &_reason);
	
	void Net_cbNetResult(Net_ConnID _id, eNet_NetResult _result, Net_U8 _new_level, Net_BitStream &_reason);

	void Net_cbDataReceived( Net_ConnID _id, Net_BitStream &_data );
	
	
	void Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply ) {}
	void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id ) {}
	void Net_cbNodeRequest_Tag( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_U32 _tag ) {}
	void Net_cbDiscovered( const Net_Address & _addr, Net_BitStream &_reply )  {}
	bool Net_cbDiscoverRequest( const Net_Address &_addr, Net_BitStream &_request, Net_BitStream &_reply ) {return false;}
	
	int port;
	bool socketsInited;
};

#endif // _SERVER_H_

