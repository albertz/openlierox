#ifndef CLIENT_H
#define CLIENT_H

#ifndef DISABLE_ZOIDCOM

#include "netstream.h"
//#include <string>

class PlayerOptions;

class Client : public Net_Control
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
	void Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply );
	
	// connection has closed
	void Net_cbConnectionClosed( Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata );
	
	void Net_cbDataReceived( Net_ConnID id, Net_BitStream &data);
	
	// zoidlevel transition finished
	void Net_cbNetResult(Net_ConnID _id, eNet_NetResult _result, Net_U8 _new_level, Net_BitStream &_reason);
	
	// server wants to tell us about new node
	void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id );
	
	virtual bool Net_cbConnectionRequest( Net_ConnID  _id, Net_BitStream &_request, Net_BitStream &_reply ){return false;}
	virtual void Net_cbConnectionSpawned( Net_ConnID _id ) {}
	virtual bool Net_cbNetRequest( Net_ConnID _id, Net_U8 _requested_level, Net_BitStream &_reason ) {return false;}
	virtual void Net_cbNodeRequest_Tag( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_U32 _tag ) {}
	//virtual void Net_cbDataReceived( Net_ConnID _id, Net_BitStream &_data ) {}
	virtual bool Net_cbDiscoverRequest( const Net_Address &_addr, Net_BitStream &_request, Net_BitStream &_reply ) {return false;}
	virtual void Net_cbDiscovered( const Net_Address & _addr, Net_BitStream &_reply )  {}
};

#endif

#endif // _CLIENT_H_
