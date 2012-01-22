#include "server.h"
#include "gconsole.h"
#include "gusgame.h"
#include "net_worm.h"
#include "game/CWorm.h"
#include "game/WormInputHandler.h"
#include "part_type.h"
#include "network.h"
#include "util/math_func.h"
#include "util/macros.h"
#include "util/log.h"
#include "encoding.h"
#include "game/CMap.h"
#include "game/Game.h"

#include "netstream.h"
#include "gusanos/allegro.h"
#include <list>


Server::Server()
		: Net_Control(true),
		m_preShutdown(false),
		socketsInited(false)
{
	Net_setControlID(0);
	Net_setDebugName("Net_CLI");
	console.addLogMsg("GUSANOS SERVER UP");
}

Server::~Server()
{}

void Server::Net_cbDataReceived(Net_ConnID _id, BitStream &_data)
{}

void Server::Net_cbConnectionSpawned( Net_ConnID _id )
{
	console.addLogMsg("* CONNECTION SPAWNED: " + itoa(_id));
	network.incConnCount();	

	network.sendEncodedLuaEvents(_id);
}

void Server::Net_cbConnectionClosed(Net_ConnID _id)
{
	console.addLogMsg("* A CONNECTION WAS CLOSED");
	for ( std::vector<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++) {
		if ( (*iter)->getConnectionID() == _id ) {
			(*iter)->quit();
		}
	}
	network.decConnCount();
	DLOG("A connection was closed");
}

