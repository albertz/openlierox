#include "proxy_player.h"
#include "game/Game.h"
#include "CServer.h"

bool ProxyPlayer::weOwnThis() const {
	// we always own the ProxyPlayer when we are server
	// otherwise, there shouldn't be any ProxyPlayers
	return game.isServer();
}

CServerConnection* ProxyPlayer::ownerClient() const {
	// we always own the ProxyPlayer
	if(game.isServer() && cServer && cServer->isServerRunning())
		return cServer->localClientConnection();
	return NULL;
}

ProxyPlayer::ProxyPlayer(CWorm* worm)
: CWormInputHandler(worm)
{
	game.onNewPlayer( this );
	game.onNewPlayer_Lua( this );
}

ProxyPlayer::~ProxyPlayer()
{

}

void ProxyPlayer::subThink()
{
}

#ifndef DEDICATED_ONLY
void ProxyPlayer::render()
{
}
#endif



