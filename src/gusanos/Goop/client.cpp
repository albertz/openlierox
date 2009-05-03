#include "client.h"
#include "gconsole.h"
#include "net_worm.h"
#include "particle.h"
#include "part_type.h"
#include "base_player.h"
#include "game.h"
#include "updater.h"
#include "network.h"
#include "player_options.h"
#include "encoding.h"
#include "updater.h"
#include <memory>
#include "util/log.h"

#ifndef DISABLE_ZOIDCOM

#include <zoidcom.h>

Client::Client( int _udpport )
{
	if(network.simLag > 0)
		ZCom_simulateLag(0, network.simLag);
	if(network.simLoss > 0.f)
		ZCom_simulateLoss(0, network.simLoss);
	if ( !ZCom_initSockets( true,_udpport, 1, 0 ) )
	{
		console.addLogMsg("* ERROR: FAILED TO INITIALIZE SOCKETS");
	}
	ZCom_setControlID(0);
	ZCom_setDebugName("ZCOM_CLI");
	ZCom_setUpstreamLimit(network.upLimit, network.upLimit); 
}

Client::~Client()
{
}

void Client::requestPlayer(PlayerOptions const& playerOptions)
{
	ZCom_BitStream *req = new ZCom_BitStream;
	req->addInt(Network::PLAYER_REQUEST,8);
	req->addString( playerOptions.name.c_str() );
	req->addInt(playerOptions.colour, 24);
	req->addSignedInt(playerOptions.team, 8);
	req->addInt(playerOptions.uniqueID, 32);
	ZCom_sendData( network.getServerID(), req, eZCom_ReliableOrdered );
}

void Client::requestPlayers()
{
	requestPlayer(*game.playerOptions[0]);
	if ( game.options.splitScreen )
		requestPlayer(*game.playerOptions[1]);
}

void Client::sendConsistencyInfo()
{
	std::auto_ptr<ZCom_BitStream> req(new ZCom_BitStream);
	req->addInt(Network::ConsistencyInfo, 8);
	req->addInt(Network::protocolVersion, 32);
	game.addCRCs(req.get());
	ZCom_sendData( network.getServerID(), req.release(), eZCom_ReliableOrdered );
}

void Client::ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply )
{
	if ( _result != eZCom_ConnAccepted )
	{
		Network::ConnectionReply::type r = static_cast<Network::ConnectionReply::type>(_reply.getInt(8));
		if(r == Network::ConnectionReply::Retry)
		{
			DLOG("Got retry from server");
			network.reconnect(50);
		}
		else if(r == Network::ConnectionReply::Banned)
		{
			console.addLogMsg("* YOU ARE BANNED FROM THIS SERVER");
		}
		else
		{
			console.addLogMsg("* COULDNT ESTABLISH CONNECTION");
		}
	}
	else
	{
		network.setClient(true);
		ZCom_requestDownstreamLimit(_id, network.downPPS, network.downBPP);
		console.addLogMsg("* CONNECTION ACCEPTED");
		network.setServerID(_id);
		network.incConnCount();
		
		std::string mod = _reply.getStringStatic();
		std::string map = _reply.getStringStatic();
		game.refreshLevels();
		game.refreshMods();
		bool hasLevel = game.hasLevel(map);
		bool hasMod = game.hasMod(mod);
		
		if(!hasMod)
		{
			game.error(Game::ErrorModNotFound);
			//This doesn't work somewhy: network.disconnect();
			//And maybe we don't want to do it since it would overwrite our error message
		}
		else if(!hasLevel)
		{
			if(network.autoDownloads)
			{
				ZCom_requestZoidMode(_id, 2); // We need to update
				if(!hasLevel)
					updater.requestLevel(map);
			}
			else
				game.error(Game::ErrorMapNotFound);
		}
		else
		{
			game.setMod( mod );
			if(game.changeLevel( map, false ) && game.isLoaded())
			{
				game.runInitScripts();
				sendConsistencyInfo();
				ZCom_requestZoidMode(_id, 1);
			}
			else
			{
				console.addLogMsg("* COULDN'T LOAD MOD OR LEVEL");
				network.disconnect();
			}
		}
	}
}

/*
void Client::loadNextGame()
{
	game.setMod( nextMod );
	if(game.changeLevel( nextMap ) && game.isLoaded())
	{
		game.runInitScripts();
		sendConsistencyInfo();
		ZCom_requestZoidMode(network.getServerID(), 1);
	}
	else
		network.disconnect();
}*/

void Client::ZCom_cbConnectionClosed(ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata)
{
	network.decConnCount();
	switch( _reason )
	{
		case eZCom_ClosedDisconnect:
		{
			Network::DConnEvents dcEvent = static_cast<Network::DConnEvents>( _reasondata.getInt(8) );
			switch( dcEvent )
			{
				case Network::ServerMapChange:
				{
					console.addLogMsg("* SERVER CHANGED MAP");
					network.reconnect(150);
					game.reset(Game::ServerChangeMap);
				}
				break;
				case Network::Quit:
				{
					console.addLogMsg("* CONNECTION CLOSED BY SERVER");
					game.reset(Game::ServerQuit);
				}
				break;
				case Network::Kick:
				{
					console.addLogMsg("* YOU WERE KICKED");
					game.reset(Game::Kicked);
				}
				break;
				case Network::IncompatibleData:
				{
					console.addLogMsg("* YOU HAVE INCOMPATIBLE DATA");
					game.reset(Game::IncompatibleData);
				}
				break;
				
				case Network::IncompatibleProtocol:
				{
					console.addLogMsg("* THE HOST RUNS AN INCOMPATIBLE VERSION OF GUSANOS");
					game.reset(Game::IncompatibleProtocol);
				}
				break;
				
				default:
				{
					console.addLogMsg("* CONNECTION CLOSED BY DUNNO WHAT :O");
					game.reset(Game::ServerQuit);
				}
				break;
			}
		}
		break;
		
		case eZCom_ClosedTimeout:
			console.addLogMsg("* CONNECTION TIMEDOUT");
			game.reset(Game::ServerQuit);
		break;
		
		case eZCom_ClosedReconnect:
			console.addLogMsg("* CONNECTION RECONNECTED");
		break;
		
		default:
		break;
	}
	
	DLOG("A connection was closed");
}

void Client::ZCom_cbDataReceived( ZCom_ConnID id, ZCom_BitStream& data) 
{
	int event = Encoding::decode(data, Network::ClientEvents::Max);
	switch(event)
	{
		case Network::ClientEvents::LuaEvents:
		{
			for(int t = Network::LuaEventGroup::Game;
				t < Network::LuaEventGroup::Max; ++t)
			{
				int c = data.getInt(8);
				for(int i = 0; i < c; ++i)
				{
					char const* name = data.getStringStatic();
					network.indexLuaEvent((Network::LuaEventGroup::type)t, name);
				}
			}
		}
		break;
	}
}

void Client::ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 new_level, ZCom_BitStream &_reason)
{
	if (_result != eZCom_ZoidEnabled)
	{
		console.addLogMsg("* ERROR: COULDNT ENTER ZOIDMODE");
	}else
	{
		console.addLogMsg("* JOINED ZOIDMODE");
		
		updater.removeNode();
		game.removeNode();
		
		switch(new_level)
		{
			case 1:
				game.assignNetworkRole( false );
				requestPlayers();
			break;
			
			case 2:
				updater.assignNetworkRole(false);
			break;
		}
	}
}

void Client::ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, ZCom_NodeID _net_id )
{
	// check the requested class
	if ( _requested_class == NetWorm::classID )
	{
		game.addWorm(false);
	}else if ( _requested_class == BasePlayer::classID )
	{
		// Creates a player class depending on the role
		if( _role == eZCom_RoleOwner )
		{
			BasePlayer* player = game.addPlayer ( Game::OWNER );
			player->assignNetworkRole(false);
		}else
		{
			BasePlayer* player = game.addPlayer ( Game::PROXY );
			player->assignNetworkRole(false);
		}
	}else if( _requested_class == Particle::classID )
	{
		int typeIndex = Encoding::decode(*_announcedata, partTypeList.size());
		BasePlayer* owner = game.findPlayerWithID(_announcedata->getInt(32));
		newParticle_requested(partTypeList[typeIndex], Vec(), Vec(), 1, owner, Angle());
	}else
	{
		console.addLogMsg("* ERROR: INVALID DYNAMIC NODE REQUEST");
	}
	
}

#endif



