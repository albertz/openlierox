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

#include "netstream.h"

Client::Client( int _udpport )
{
	if(network.simLag > 0)
		Net_simulateLag(0, network.simLag);
	if(network.simLoss > 0.f)
		Net_simulateLoss(0, network.simLoss);
	if ( !Net_initSockets( true,_udpport, 1, 0 ) )
	{
		console.addLogMsg("* ERROR: FAILED TO INITIALIZE SOCKETS");
	}
	Net_setControlID(0);
	Net_setDebugName("Net_CLI");
	Net_setUpstreamLimit(network.upLimit, network.upLimit); 
}

Client::~Client()
{
}

void Client::requestPlayer(PlayerOptions const& playerOptions)
{
	Net_BitStream *req = new Net_BitStream;
	req->addInt(Network::PLAYER_REQUEST,8);
	req->addString( playerOptions.name.c_str() );
	req->addInt(playerOptions.colour, 24);
	req->addSignedInt(playerOptions.team, 8);
	req->addInt(playerOptions.uniqueID, 32);
	Net_sendData( network.getServerID(), req, eNet_ReliableOrdered );
}

void Client::requestPlayers()
{
	requestPlayer(*game.playerOptions[0]);
	if ( game.options.splitScreen )
		requestPlayer(*game.playerOptions[1]);
}

void Client::sendConsistencyInfo()
{
	std::auto_ptr<Net_BitStream> req(new Net_BitStream);
	req->addInt(Network::ConsistencyInfo, 8);
	req->addInt(Network::protocolVersion, 32);
	game.addCRCs(req.get());
	Net_sendData( network.getServerID(), req.release(), eNet_ReliableOrdered );
}

void Client::Net_cbConnectResult( Net_ConnID _id, eNet_ConnectResult _result, Net_BitStream &_reply )
{
	if ( _result != eNet_ConnAccepted )
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
		Net_requestDownstreamLimit(_id, network.downPPS, network.downBPP);
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
				Net_requestNetMode(_id, 2); // We need to update
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
				Net_requestNetMode(_id, 1);
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
		Net_requestNetMode(network.getServerID(), 1);
	}
	else
		network.disconnect();
}*/

void Client::Net_cbConnectionClosed(Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata)
{
	network.decConnCount();
	switch( _reason )
	{
		case eNet_ClosedDisconnect:
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
					console.addLogMsg("* THE HOST RUNS AN INCOMPATIBLE VERSION OF VERMES");
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
		
		case eNet_ClosedTimeout:
			console.addLogMsg("* CONNECTION TIMEDOUT");
			game.reset(Game::ServerQuit);
		break;
		
		case eNet_ClosedReconnect:
			console.addLogMsg("* CONNECTION RECONNECTED");
		break;
		
		default:
		break;
	}
	
	DLOG("A connection was closed");
}

void Client::Net_cbDataReceived( Net_ConnID id, Net_BitStream& data) 
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

void Client::Net_cbNetResult(Net_ConnID _id, eNet_NetResult _result, Net_U8 new_level, Net_BitStream &_reason)
{
	if (_result != eNet_NetEnabled)
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

void Client::Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id )
{
	// check the requested class
	if ( _requested_class == NetWorm::classID )
	{
		game.addWorm(false);
	}else if ( _requested_class == BasePlayer::classID )
	{
		// Creates a player class depending on the role
		if( _role == eNet_RoleOwner )
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



