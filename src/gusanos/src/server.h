#ifndef SERVER_H
#define SERVER_H

#ifndef DISABLE_ZOIDCOM

#include "base_player.h"

#include <zoidcom.h>
#include <map>
#include <boost/shared_ptr.hpp>

class Server : public ZCom_Control
{
private:
	// Flag to refuse connection while waiting for all players to aknowledge disconnection
	bool m_preShutdown;
	std::map<unsigned int, boost::shared_ptr<BasePlayer::Stats> > savedScores;
	
public:
	Server( int _udpport );
	~Server();
	
	void preShutdown() { m_preShutdown = true; }
	
protected:
		
	// called on incoming connections
	bool ZCom_cbConnectionRequest( ZCom_ConnID _id, ZCom_BitStream &_request, ZCom_BitStream &_reply );
	// called when incoming connection has been established
	void ZCom_cbConnectionSpawned( ZCom_ConnID _id );
	// called when a connection closed
	void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata );
	
	bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason);
	
	void ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason);

	void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data );
	
	
	void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) {}
	void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, ZCom_NodeID _net_id ) {}
	void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, zU32 _tag ) {}
	void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
	bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply ) {return false;}
	
	int port;
	bool socketsInited;
};

#endif

#endif // _SERVER_H_

