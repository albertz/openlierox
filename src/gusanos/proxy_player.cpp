#include "proxy_player.h"
#include "player_options.h"

ProxyPlayer::ProxyPlayer(CWorm* worm)
: CWormInputHandler(shared_ptr<PlayerOptions>(new PlayerOptions), worm)
{

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



