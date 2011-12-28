#include "client.h"
#include "gconsole.h"
#include "net_worm.h"
#include "proxy_player.h"
#include "CWormHuman.h"
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
#include "game/Game.h"
#include "CGameScript.h"


Client::Client() : Net_Control(false)
{
	Net_setControlID(0);
	Net_setDebugName("Net_CLI");	
}

void Client::Net_cbConnectResult(eNet_ConnectResult res) {
	if(res != eNet_ConnAccepted) {
		errors << "Client::Net_cbConnectResult: connection not accepted" << endl;
		return;
	}

	console.addLogMsg("* CONNECTION ACCEPTED");
	network.incConnCount();
	
	// earlier, we did the mod/level loading here
	// _reply contained two strings, containing level/mod name
	sendConsistencyInfo();
	Net_requestNetMode(NetConnID_server(), 1);
}

Client::~Client()
{
}

void Client::sendConsistencyInfo()
{
	std::auto_ptr<BitStream> req(new BitStream);
	req->addInt(Network::ConsistencyInfo, 8);
	req->addInt(Network::protocolVersion, 32);
	gusGame.addCRCs(req.get());
	Net_sendData( network.getServerID(), req.release(), eNet_ReliableOrdered );
}

void Client::Net_cbConnectionClosed(Net_ConnID _id, eNet_CloseReason _reason, BitStream &_reasondata)
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

void Client::Net_cbDataReceived( Net_ConnID id, BitStream& data) 
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
					std::string name = data.getString();
					network.indexLuaEvent((Network::LuaEventGroup::type)t, name);
				}
			}
		}
		break;
	}
}


void Client::Net_cbNodeRequest_Dynamic( Net_ConnID _id, Net_ClassID _requested_class, BitStream *_announcedata, eNet_NodeRole _role, Net_NodeID _net_id )
{
	// check the requested class
	if ( _requested_class == CWorm::classID )
	{
		if(_announcedata == NULL) {
			warnings << "Net_cbNodeRequest_Dynamic for CWorm without announce data" << endl;
			return;
		}

		int wormid = _announcedata->getInt(8);
		if(wormid < 0 || wormid >= MAX_WORMS) {
			warnings << "Net_cbNodeRequest_Dynamic for CWorm: wormid " << wormid << " invalid" << endl;
			return;
		}
		
		notes << "Net_cbNodeRequest_Dynamic for worm " << wormid << ", nodeid " << _net_id << endl;
		
		CWorm* worm = &cClient->getRemoteWorms()[wormid];
		if(!worm->isUsed()) {
			warnings << "Net_cbNodeRequest_Dynamic for CWorm: worm " << wormid << " is not used" << endl;
			return;
		}
		
		worm->NetWorm_Init(false);
	}else if ( _requested_class == CWormInputHandler::classID )
	{
		if(_announcedata == NULL) {
			warnings << "Net_cbNodeRequest_Dynamic for CWormInputHandler without announce data" << endl;
			return;
		}
		
		int wormid = _announcedata->getInt(8);
		if(wormid < 0 || wormid >= MAX_WORMS) {
			warnings << "Net_cbNodeRequest_Dynamic for CWormInputHandler: wormid " << wormid << " invalid" << endl;
			return;
		}
		
		CWorm* worm = &cClient->getRemoteWorms()[wormid];
		if(!worm->isUsed()) {
			warnings << "Net_cbNodeRequest_Dynamic for CWormInputHandler: worm " << wormid << " is not used" << endl;
			return;
		}
		
		notes << "Net_cbNodeRequest_Dynamic: new player (node " << _net_id << ") for worm " << wormid << ", set to " << ((_role == eNet_RoleOwner) ? "owner" : "proxy") << endl;
		
		// Creates a player class depending on the role
		CWormInputHandler* player = (_role == eNet_RoleOwner) ? (CWormInputHandler*)new CWormHumanInputHandler(worm) : new ProxyPlayer(worm);
		player->assignNetworkRole(false);
		
		if(worm->m_inputHandler) {
			warnings << "Net_cbNodeRequest_Dynamic: worm " << worm->getName() << " has already the following input handler set: "; warnings.flush();
			warnings << worm->m_inputHandler->name() << endl;
			worm->m_inputHandler->quit();
			worm->m_inputHandler = NULL;			
		}
		
		worm->m_inputHandler = player;

		cClient->SetupGameInputs(); // we must init the inputs for the new inputhandler
		
		if(cClient->getGameReady()) {
			if(cClient->getStatus() == NET_PLAYING) // playing
				player->startGame();
			else if(!game.gameScript()->gusEngineUsed())
				player->initWeaponSelection();
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



