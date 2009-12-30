#include "client.h"
#include "gconsole.h"
#include "net_worm.h"
#include "particle.h"
#include "part_type.h"
#include "game/WormInputHandler.h"
#include "gusgame.h"
#include "network.h"
#include "player_options.h"
#include "encoding.h"
#include <memory>
#include "util/log.h"

#include "netstream.h"

Client::Client( int _udpport ) : Net_Control(false)
{
	Net_setControlID(0);
	Net_setDebugName("Net_CLI");
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
	requestPlayer(*gusGame.playerOptions[0]);
	if ( gusGame.options.splitScreen )
		requestPlayer(*gusGame.playerOptions[1]);
}

void Client::sendConsistencyInfo()
{
	std::auto_ptr<Net_BitStream> req(new Net_BitStream);
	req->addInt(Network::ConsistencyInfo, 8);
	req->addInt(Network::protocolVersion, 32);
	gusGame.addCRCs(req.get());
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
			network.olxReconnect(50);
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
		console.addLogMsg("* CONNECTION ACCEPTED");
		network.incConnCount();

		// earlier, we did the mod/level loading here
		// _reply contained two strings, containing level/mod name
		sendConsistencyInfo();
		Net_requestNetMode(_id, 1);
	}
}

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
					network.olxReconnect(150);
					gusGame.reset(GusGame::ServerChangeMap);
				}
				break;
				case Network::Quit:
				{
					console.addLogMsg("* CONNECTION CLOSED BY SERVER");
					gusGame.reset(GusGame::ServerQuit);
				}
				break;
				case Network::Kick:
				{
					console.addLogMsg("* YOU WERE KICKED");
					gusGame.reset(GusGame::Kicked);
				}
				break;
				case Network::IncompatibleData:
				{
					console.addLogMsg("* YOU HAVE INCOMPATIBLE DATA");
					gusGame.reset(GusGame::IncompatibleData);
				}
				break;
				
				case Network::IncompatibleProtocol:
				{
					console.addLogMsg("* THE HOST RUNS AN INCOMPATIBLE VERSION OF VERMES");
					gusGame.reset(GusGame::IncompatibleProtocol);
				}
				break;
				
				default:
				{
					console.addLogMsg("* CONNECTION CLOSED BY DUNNO WHAT :O");
					gusGame.reset(GusGame::ServerQuit);
				}
				break;
			}
		}
		break;
		
		case eNet_ClosedTimeout:
			console.addLogMsg("* CONNECTION TIMEDOUT");
			gusGame.reset(GusGame::ServerQuit);
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
			for(int t = Network::LuaEventGroup::GusGame;
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


void Client::Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, Net_BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id )
{
	// check the requested class
	if ( _requested_class == NetWorm::classID )
	{
		gusGame.addWorm(false);
	}else if ( _requested_class == CWormInputHandler::classID )
	{
		// Creates a player class depending on the role
		if( _role == eNet_RoleOwner )
		{
			CWormInputHandler* player = gusGame.addPlayer ( GusGame::OWNER );
			player->assignNetworkRole(false);
		}else
		{
			CWormInputHandler* player = gusGame.addPlayer ( GusGame::PROXY );
			player->assignNetworkRole(false);
		}
	}else if( _requested_class == Particle::classID )
	{
		int typeIndex = Encoding::decode(*_announcedata, partTypeList.size());
		CWormInputHandler* owner = gusGame.findPlayerWithID(_announcedata->getInt(32));
		newParticle_requested(partTypeList[typeIndex], Vec(), Vec(), 1, owner, Angle());
	}else
	{
		console.addLogMsg("* ERROR: INVALID DYNAMIC NODE REQUEST");
	}
	
}



