#include "proxy_player.h"
#include "game/Game.h"

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



