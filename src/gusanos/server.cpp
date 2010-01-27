#include "gusanos/server.h"
#include "gusanos/gconsole.h"
#include "gusanos/gusgame.h"
#include "gusanos/net_worm.h"
#include "CWorm.h"
#include "game/WormInputHandler.h"
#include "gusanos/part_type.h"
#include "gusanos/player_options.h"
#include "gusanos/network.h"
#include "util/math_func.h"
#include "util/macros.h"
#include "util/log.h"
#include "gusanos/encoding.h"
#include "CMap.h"
#include "game/Game.h"

#include "gusanos/netstream.h"
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

void Server::Net_cbDataReceived( Net_ConnID  _id, Net_BitStream &_data)
{
	Network::NetEvents event = (Network::NetEvents) _data.getInt(8);
	switch( event ) {
			case Network::RConMsg: {
				std::string passwordSent = _data.getString();
				if ( !gusGame.options.rConPassword.empty() && gusGame.options.rConPassword == passwordSent ) {
					//console.addQueueCommand(_data.getStringStatic());
					console.parseLine(_data.getString());
				}
			}
			break;

			case Network::ConsistencyInfo: {
				int clientProtocol = _data.getInt(32);
				if(clientProtocol != Network::protocolVersion) {
					network.disconnect(_id, Network::IncompatibleProtocol);
				}

				if(!gusGame.checkCRCs(_data) && network.checkCRC) // We call checkCRC anyway so that the stream is advanced
					network.disconnect(_id, Network::IncompatibleData);

			}
			break;
	}
}

void Server::Net_cbConnectionSpawned( Net_ConnID _id )
{
	console.addLogMsg("* CONNECTION SPAWNED: " + itoa(_id));
	network.incConnCount();	

	network.sendEncodedLuaEvents(_id);
}

void Server::Net_cbConnectionClosed(Net_ConnID _id, eNet_CloseReason _reason, Net_BitStream &_reasondata)
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

