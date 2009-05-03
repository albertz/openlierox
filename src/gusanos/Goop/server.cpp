#include "server.h"
#include "gconsole.h"
#include "game.h"
#include "net_worm.h"
#include "base_worm.h"
#include "base_player.h"
#include "part_type.h"
#include "player_options.h"
#include "network.h"
#include "util/math_func.h"
#include "util/macros.h"
#include "util/log.h"
#include "encoding.h"

#ifndef DISABLE_ZOIDCOM

#include <zoidcom.h>
#include <allegro.h>
#include <list>


Server::Server( int _udpport )
: m_preShutdown(false), socketsInited(false), port(_udpport)
{
	if(network.simLag > 0)
		ZCom_simulateLag(0, network.simLag);
	if(network.simLoss > 0.f)
		ZCom_simulateLoss(0, network.simLoss);

	// TODO: Asynchronize this loop
	int tries = 10;
	bool result = false;
	while ( !result && tries-- > 0 )
	{
		rest(100);
		result = ZCom_initSockets( true, _udpport, 1, 0 );
	}
	if(!result)
		console.addLogMsg("* ERROR: FAILED TO INITIALIZE SOCKETS");

	ZCom_setControlID(0);
	ZCom_setDebugName("ZCOM_CLI");
	ZCom_setUpstreamLimit(network.upLimit, network.upLimit);
	console.addLogMsg("SERVER UP");
}

Server::~Server()
{	
}

/*
bool Server::initSockets()
{
	if(socketsInited)
		return true;
	socketsInited = ZCom_initSockets( true, port, 1, 0 );
	return socketsInited; 
}*/

void Server::ZCom_cbDataReceived( ZCom_ConnID  _id, ZCom_BitStream &_data) 
{
	Network::NetEvents event = (Network::NetEvents) _data.getInt(8);
	switch( event )
	{
		case Network::PLAYER_REQUEST:
		{
			std::string name = _data.getStringStatic();
			int colour = _data.getInt(24);
			int team = _data.getSignedInt(8);
			unsigned int uniqueID = static_cast<unsigned int>(_data.getInt(32));

			BaseWorm* worm = game.addWorm(true);
			if ( NetWorm* netWorm = dynamic_cast<NetWorm*>(worm) )
			{
				netWorm->setOwnerId(_id);
			}
			BasePlayer* player = game.addPlayer ( Game::PROXY );
			
			let_(i, savedScores.find(uniqueID));
			if(i != savedScores.end())
			{
				player->stats = i->second;
				player->getOptions()->uniqueID = uniqueID;
			}
			else
			{
				do
				{
					uniqueID = rndgen();
				} while(!uniqueID);
				
				player->getOptions()->uniqueID = uniqueID;
				savedScores[uniqueID] = player->stats;
			}
			
			player->colour = colour;
			player->team = team;
			player->localChangeName( name );
			console.addLogMsg( "* " + player->m_name + " HAS JOINED THE GAME");
			player->assignNetworkRole(true);
			player->setOwnerId(_id);
			player->assignWorm(worm);
		}
		break;
		case Network::RConMsg:
		{
			char const* passwordSent = _data.getStringStatic();
			if ( !game.options.rConPassword.empty() && game.options.rConPassword == passwordSent )
			{
				//console.addQueueCommand(_data.getStringStatic());
				console.parseLine(_data.getStringStatic());
			}
		}
		break;
		
		case Network::ConsistencyInfo:
		{
			int clientProtocol = _data.getInt(32);
			if(clientProtocol != Network::protocolVersion)
			{
				network.disconnect(_id, Network::IncompatibleProtocol);
			}
			
			if(!game.checkCRCs(_data) && network.checkCRC) // We call checkCRC anyway so that the stream is advanced
				network.disconnect(_id, Network::IncompatibleData);
			
		}
		break;
	}
}

bool Server::ZCom_cbConnectionRequest( ZCom_ConnID id, ZCom_BitStream &_request, ZCom_BitStream &reply )
{
	if(network.isBanned(id))
	{
		reply.addInt(Network::ConnectionReply::Banned, 8);
		return false;
	}
	else if(network.clientRetry)
	{
		reply.addInt(Network::ConnectionReply::Retry, 8);
		return false;
	}
	else if ( !m_preShutdown )
	{
		console.addLogMsg("* CONNECTION REQUESTED");
		//_reply.addInt(Network::ConnectionReply::Ok, 8);
		reply.addString( game.getMod().c_str() );
		reply.addString( game.level.getName().c_str() );

		return true;
	}
	else
	{
		reply.addInt(Network::ConnectionReply::Refused, 8);
		return false;
	}
}

void Server::ZCom_cbConnectionSpawned( ZCom_ConnID _id )
{
	console.addLogMsg("* CONNECTION SPAWNED");
	ZCom_requestDownstreamLimit(_id, network.downPPS, network.downBPP);
	network.incConnCount();

	std::auto_ptr<ZCom_BitStream> data(new ZCom_BitStream);
	Encoding::encode(*data, Network::ClientEvents::LuaEvents, Network::ClientEvents::Max); 
	network.encodeLuaEvents(data.get());
	ZCom_sendData ( _id, data.release(), eZCom_ReliableOrdered);
}

void Server::ZCom_cbConnectionClosed(ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata)
{
	console.addLogMsg("* A CONNECTION WAS CLOSED");
	for ( std::list<BasePlayer*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
	{
		if ( (*iter)->getConnectionID() == _id )
		{
			(*iter)->deleteMe = true;
		}
	}
	network.decConnCount();
	DLOG("A connection was closed");
}

bool Server::ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 requested_level, ZCom_BitStream &_reason)
{
	switch(requested_level)
	{
		case 1: case 2:
		{
			console.addLogMsg("* ZOIDMODE REQUEST ACCEPTED");
			return true;
		}
		break;
		
		default:
			return false;
	}
}

void Server::ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason)
{
	console.addLogMsg("* NEW CONNECTION JOINED ZOIDMODE");
}

#endif

