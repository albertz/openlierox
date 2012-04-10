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
	Net_requestNetMode(NetConnID_server(), 1);
}

Client::~Client()
{
}

void Client::Net_cbConnectionClosed(Net_ConnID _id)
{
	network.decConnCount();	
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
		
		CWorm* worm = game.wormById(wormid, false);
		if(!worm) {
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
		
		CWorm* worm = game.wormById(wormid, false);
		if(!worm) {
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
			worm->m_inputHandler->deleteThis();
			worm->m_inputHandler = NULL;			
		}
		
		worm->m_inputHandler = player;

		cClient->SetupGameInputs(); // we must init the inputs for the new inputhandler
		
		if(game.state >= Game::S_Preparing) {
			if(game.state == Game::S_Playing)
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



