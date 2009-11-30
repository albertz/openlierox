#ifndef NETWORK_H
#define NETWORK_H

#include "netstream.h"
#include <string.h>
#include <stdlib.h>
#include <allegro.h>
#include "console.h"

class Server : public Net_Control
{
public:
  bool is_ok;

  Server( int _internalport, int _udpport );
  ~Server();

  void update();
protected:
  // called on incoming connections
  bool Net_cbConnectionRequest( Net_ConnID _id, Net_BitStream &_request, Net_BitStream &_reply );
  // called when incoming connection has been established
  void Net_cbConnectionSpawned( Net_ConnID _id );
  // called when a connection closed
  void Net_cbConnectionClosed( Net_ConnID _id, Net_BitStream &_reason );
  // called when a connection wants to enter a zoidlevel
  bool Net_cbZoidRequest( Net_ConnID _id, Net_U8 _requested_level, Net_BitStream &_reason);
  // called when a connection entered a zoidlevel
  void Net_cbZoidResult(Net_ConnID _id, eNet_ZoidResult _result, Net_U8 _new_level, Net_BitStream &_reason);
  // called when broadcast has been received
  //bool Net_cbDiscoverRequest(const Net_Address &_addr, Net_BitStream &_request, Net_BitStream &_reply);
  void Net_cbDataReceived( Net_ConnID _id, Net_BitStream &_data );
  
  virtual void Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply ) {}
  virtual void Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, eNet_NodeRole _role, Net_NodeID _net_id ) {}
  virtual void Net_cbNodeRequest_Tag( Net_ConnID _id, Net_ClassID _requested_class, eNet_NodeRole _role, Net_U32 _tag ) {}
  virtual void Net_cbDiscovered( const Net_Address & _addr, Net_BitStream &_reply )  {}
  virtual bool Net_cbDiscoverRequest( const Net_Address &_addr, Net_BitStream &_request, Net_BitStream &_reply ) {return false;}
};

class Client : public Net_Control
{
protected:
  //Net_ConnID m_id;
  Net_ConnID srv_id;
  int         m_localnode;
public:
  bool m_exitnow;

  Client( int _internalport, int _udpport );
  ~Client();
  void request_players();
  // draw known objects to screen

protected:
  // called when initiated connection process yields a result
  void Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply );

  // connection has closed
  void Net_cbConnectionClosed( Net_ConnID _id, Net_BitStream &_reason );

  // zoidlevel transition finished
  void Net_cbZoidResult(Net_ConnID _id, eNet_ZoidResult _result, Net_U8 _new_level, Net_BitStream &_reason);

  // server wants to tell us about new node
  void Net_cbNodeRequest_Dynamic(Net_ConnID _id, Net_ClassID _requested_class, eNet_NodeRole _role, Net_NodeID _net_id);

  virtual bool Net_cbConnectionRequest( Net_ConnID  _id, Net_BitStream &_request, Net_BitStream &_reply ){return false;}
  virtual void Net_cbConnectionSpawned( Net_ConnID _id ) {}
  virtual bool Net_cbZoidRequest( Net_ConnID _id, Net_U8 _requested_level, Net_BitStream &_reason ) {return false;}
  virtual void Net_cbNodeRequest_Tag( Net_ConnID _id, Net_ClassID _requested_class, eNet_NodeRole _role, Net_U32 _tag ) {}
  virtual void Net_cbDataReceived( Net_ConnID _id, Net_BitStream &_data ) {}
  virtual bool Net_cbDiscoverRequest( const Net_Address &_addr, Net_BitStream &_request, Net_BitStream &_reply ) {return false;}
  virtual void Net_cbDiscovered( const Net_Address & _addr, Net_BitStream &_reply )  {}
};

extern ZoidCom *zcom;
extern Server *srv;
extern Client *cli;
extern Net_Address adr_srv;

#endif