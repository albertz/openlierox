#include "proxy_player.h"
#include "player_options.h"
#include "game/Game.h"

ProxyPlayer::ProxyPlayer(CWorm* worm)
: CWormInputHandler(shared_ptr<PlayerOptions>(new PlayerOptions), worm)
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



