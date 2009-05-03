#include "proxy_player.h"
#include "player_options.h"

ProxyPlayer::ProxyPlayer(BaseWorm* worm)
: BasePlayer(shared_ptr<PlayerOptions>(new PlayerOptions), worm)
{

}

ProxyPlayer::~ProxyPlayer()
{

}

void ProxyPlayer::subThink()
{
}

#ifndef DEDSERV
void ProxyPlayer::render()
{
}
#endif



