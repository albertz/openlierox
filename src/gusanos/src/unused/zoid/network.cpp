#include "network.h"
#include "engine.h"
#include "player.h"
#include "level.h"
#include "console.h"
#include <string>

Client *cli;
Server *srv;
ZoidCom *zcom;
Net_Address adr_srv;


Server::Server( int _internalport, int _udpport )
{
	// this will allocate the sockets and create local bindings
  is_ok=true;
	if ( !Net_initSockets( true, _udpport, _internalport, 0 ) )
	{
		allegro_message("unable to initialize the sockets");
    is_ok=false;
	} else
  con->log.create_msg("SOCKETS INITIALIZED");
  
  Net_setControlID(0);

	// string shown in zcoms debug output
	Net_setDebugName("Net_SRV");


  game_classid=Net_registerClass("game");
  player_classid=Net_registerClass("player");

	Net_setUpstreamLimit(0, 0);
}

Server::~Server() 
{
  
}


void Server::Net_cbDataReceived( Net_ConnID  _id, Net_BitStream &_data) 
{
  int event;
  char tmpstr[255];
  event = _data.getInt(8);
  switch(event)
  {
    case 0:
    {
      //players where requested
      // spawn new playerobject
      player[player_count] = new worm;
      player[player_count]->init_node(true);
      sprintf(tmpstr, "NEW PLAYER CREATED AS INDEX %d", player_count);
      con->log.create_msg(tmpstr);
      player[player_count]->id = _id;
      player[player_count]->local_slot=local_players;
      // make connection owner of object so connection may change x and y of object (see NObject::init())
      player[player_count]->node->setOwner(_id, true);
      player_count++;
      if (_data.getBool())
      {
        //Client is playing splitscreen so we create another player for him :)
        player[player_count] = new worm;
        player[player_count]->init_node(true);
        sprintf(tmpstr, "NEW PLAYER CREATED AS INDEX %d", player_count);
        con->log.create_msg(tmpstr);
        player[player_count]->id = _id;
        player[player_count]->local_slot=local_players;
        // make connection owner of object so connection may change x and y of object (see NObject::init())
        player[player_count]->node->setOwner(_id, true);
        player_count++;
      };
    };
  };
};


// called on incoming connections
bool Server::Net_cbConnectionRequest( Net_ConnID _id, Net_BitStream &_request, Net_BitStream &_reply )
{
	const char * req = _request.getStringStatic();

	// address information
	/*const Net_Address* addr = Net_getPeer( _id );
	if ( addr )
	{
		if ( addr->getType() == eNet_AddressLocal );
			sys_print( "Server: Incoming connection from localport: %d", addr->getPort() );
		else if ( addr->getType() == eNet_AddressUDP );
			sys_print( "Server: Incoming connection from UDP: %d.%d.%d.%d:%d", addr->getIP( 0 ), addr->getIP( 1 ), addr->getIP( 2 ), addr->getIP( 3 ), addr->getPort() );
	}*/

	// check what the client is requesstig
	if ( req && strlen( req ) > 0 && strcmp( req, "letmein" ) == 0 )
	{
		//sys_print( "Server: Incoming connection with ID: %d requesting: '%s'... accepted", _id, req );
		// accept the connection request
		_reply.addString(map->name);
		return true;
	}
	else
	{
		//sys_print( "Server: Incoming connection with ID: %d requesting: '%s'... denied", _id, req );
		// deny connection request and send reason back to requester
		_reply.addString( "invalid request" );
		return false;
	}
}

// called when incoming connection has been established
void Server::Net_cbConnectionSpawned( Net_ConnID _id )
{
  con->log.create_msg("A CLIENT HAS JOINED THE GAME");
	// request 20 packets/second and 200 bytes per packet from client (maximum values of course)
	Net_requestDownstreamLimit(_id, 20, 200);
}

// called when a connection closed
void Server::Net_cbConnectionClosed( Net_ConnID _id, Net_BitStream &_reason )
{
  int i;
	const char * reason = _reason.getStringStatic();
  con->log.create_msg("A PLAYER DISCONNECTED");
  for (i=0;i<player_count;i++)
  {
    if (player[i]->id==_id)
    {
			player[i]->remove_player(i);
      i--;
    };
  };
	//sys_print( "Server: Connection with ID: %d has been closed, reason is: '%s'.", _id, reason );

	// retrieve object belonging to client
	//int slot = (int) Net_getUserData(_id);

}

// a client wants to enter a zoidlevel
bool Server::Net_cbZoidRequest( Net_ConnID _id, Net_U8 _requested_level, Net_BitStream &_reason)
{
	// check level and accept
	if (_requested_level == 2)
	{
		//sys_print("Server: accepted Zoidrequest for level [%d] from %d", _requested_level, _id);
		return true;
	}
	// or deny
	else
		return false;
}

// client entered a zoidlevel or failed
void Server::Net_cbZoidResult(Net_ConnID _id, eNet_NetResult _result, Net_U8 _new_level, Net_BitStream &_reason)
{
	// failed
	if (_result != eNet_NetEnabled)
	{
		//sys_print("Server: %d failed to enter zoidmode", _id);
		return;
	}
};


Client::Client( int _internalport, int _udpport )
{
	//m_id = Net_Invalid_ID;

	// this will allocate the sockets and create local bindings
	if ( !Net_initSockets( true,_udpport, _internalport, 0 ) )
	{
		allegro_message("unable to initialize the sockets");
	} else
  con->log.create_msg("SOCKETS INITIALIZED");

  Net_setControlID(0);
  
	Net_setDebugName("Net_CLI");
  
  game_classid = Net_registerClass("game");
  player_classid = Net_registerClass("player");
  

}

Client::~Client()
{
};

void Client::request_players()
{
  Net_BitStream *req = Net_Control::Net_createBitStream();
  req->addInt(0,8);
  if (*game->SPLIT_SCREEN==1) req->addBool( true );
  else req->addBool( false );
  Net_sendData(srv_id,req,eNet_Reliable);
};

// called when initiated connection process yields a result
void Client::Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply )
{
  char tmpstr[255];
	//sys_print( "Client: The connection process for %d returned with resultcode %d, the reply was '%s'.", _id, _result, _reply.getStringStatic() );
	if ( _result != eNet_ConnAccepted )
  {
    con->log.create_msg("YOU WERE UNABLE TO JOIN THE SERVER");
		m_exitnow = true;
  }
	else
	{
    game->init_node(cli,false);
		Net_requestDownstreamLimit(_id, 20, 200);
		Net_requestNetMode(_id, 2);
    srv_id=_id;
    request_players();
    //delete_players();
    player_count=0;
    local_players=0;
    game->client=true;
    const char * svmap = _reply.getStringStatic();
    sprintf(tmpstr, "SERVERS MAP IS %s", svmap);
    con->log.create_msg(tmpstr);
    strcpy(con->arg,svmap);
    change_level();
		//m_id = _id;
	};
} 

void Client::Net_cbConnectionClosed( Net_ConnID _id, Net_BitStream &_reason )
{
	//sys_print( "Client: Connection with ID: %d has been closed, reason is: '%s'.", _id, _reason.getStringStatic() );
  con->log.create_msg("YOU WERE DISCONNECTED FROM SERVER");
	m_exitnow = true;
}

void Client::Net_cbZoidResult(Net_ConnID _id, eNet_NetResult _result, Net_U8 _new_level, Net_BitStream &_reason)
{
	// disconnect on failure
	if (_result != eNet_NetEnabled)
	{
		//sys_print("Client: Zoidmode failed, disconnecting");
		Net_Disconnect(_id, NULL);
		return;
	}

	//sys_print("Client: Zoidlevel [%d] entered", _new_level);
}


void Client::Net_cbNodeRequest_Dynamic(Net_ConnID _id, Net_ClassID _requested_class, eNet_NodeRole _role, Net_NodeID _net_id)
{
  char tmpstr[255];
	// check the requested class
	if (_requested_class == player_classid)
	{
    if(_role==eNet_RoleOwner)
    {
      sprintf(tmpstr, "LOCAL PLAYER STORED IN INDEX %d", local_players);
      con->log.create_msg(tmpstr);
      if (player[local_players]) delete player[local_players];
      player[local_players] = new worm;
      player[local_players]->init_node(false);
      player[local_players]->islocal=true;
      player[local_players]->local_slot=local_players;
			player[local_players]->id=_id;
      if (local_players<2)local_player[local_players]=local_players;
      change_nick(local_players);
      player_count++;
      local_players++;
    }else
    {
			int pos = 1;
			if (*game->SPLIT_SCREEN)
				pos++;
			pos= pos + player_count - local_players;
      sprintf(tmpstr, "NET PLAYER STORED IN INDEX %d", pos);
      con->log.create_msg(tmpstr);
      if (player[pos]) delete player[pos];
      player[pos]=new worm;
      player[pos]->init_node(false);
      player[pos]->local_slot=pos;
			player[pos]->id=_id;
      player_count++;
    };
	};
}
