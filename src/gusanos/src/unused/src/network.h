#ifndef NETWORK_H
#define NETWORK_H

#include <zoidcom.h>
#include <string.h>
#include <stdlib.h>
#include <allegro.h>
#include "console.h"

class Server : public ZCom_Control
{
public:
  bool is_ok;

  Server( int _internalport, int _udpport );
  ~Server();

  void update();
protected:
  // called on incoming connections
  bool ZCom_cbConnectionRequest( ZCom_ConnID _id, ZCom_BitStream &_request, ZCom_BitStream &_reply );
  // called when incoming connection has been established
  void ZCom_cbConnectionSpawned( ZCom_ConnID _id );
  // called when a connection closed
  void ZCom_cbConnectionClosed( ZCom_ConnID _id, ZCom_BitStream &_reason );
  // called when a connection wants to enter a zoidlevel
  bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason);
  // called when a connection entered a zoidlevel
  void ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason);
  // called when broadcast has been received
  //bool ZCom_cbDiscoverRequest(const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply);
  void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data );
  
  virtual void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) {}
  virtual void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, eZCom_NodeRole _role, ZCom_NodeID _net_id ) {}
  virtual void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, eZCom_NodeRole _role, zU32 _tag ) {}
  virtual void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
  virtual bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply ) {return false;}
};

class Client : public ZCom_Control
{
protected:
  //ZCom_ConnID m_id;
  ZCom_ConnID srv_id;
  int         m_localnode;
public:
  bool m_exitnow;

  Client( int _internalport, int _udpport );
  ~Client();
  void request_players();
  // draw known objects to screen

protected:
  // called when initiated connection process yields a result
  void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply );

  // connection has closed
  void ZCom_cbConnectionClosed( ZCom_ConnID _id, ZCom_BitStream &_reason );

  // zoidlevel transition finished
  void ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason);

  // server wants to tell us about new node
  void ZCom_cbNodeRequest_Dynamic(ZCom_ConnID _id, ZCom_ClassID _requested_class, eZCom_NodeRole _role, ZCom_NodeID _net_id);

  virtual bool ZCom_cbConnectionRequest( ZCom_ConnID  _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ){return false;}
  virtual void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) {}
  virtual bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason ) {return false;}
  virtual void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, eZCom_NodeRole _role, zU32 _tag ) {}
  virtual void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) {}
  virtual bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply ) {return false;}
  virtual void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
};

extern ZoidCom *zcom;
extern Server *srv;
extern Client *cli;
extern ZCom_Address adr_srv;

#endif