#ifndef CLIENT_H
#define CLIENT_H

#ifndef DISABLE_ZOIDCOM

#include <zoidcom.h>
//#include <string>

class PlayerOptions;

class Client : public ZCom_Control
{
public:
	
	Client( int _udpport );
	~Client();
	
	void requestPlayer(PlayerOptions const& playerOptions);
	void requestPlayers();
	void sendConsistencyInfo();
	
	void loadNextGame();
	
	//std::string nextMod;
	//std::string nextMap;

protected:

	// called when initiated connection process yields a result
	void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply );
	
	// connection has closed
	void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata );
	
	void ZCom_cbDataReceived( ZCom_ConnID id, ZCom_BitStream &data);
	
	// zoidlevel transition finished
	void ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason);
	
	// server wants to tell us about new node
	void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, ZCom_NodeID _net_id );
	
	virtual bool ZCom_cbConnectionRequest( ZCom_ConnID  _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ){return false;}
	virtual void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) {}
	virtual bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason ) {return false;}
	virtual void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, zU32 _tag ) {}
	//virtual void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) {}
	virtual bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply ) {return false;}
	virtual void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
};

#endif

#endif // _CLIENT_H_
