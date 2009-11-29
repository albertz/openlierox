#ifndef PROXY_PLAYER_H
#define PROXY_PLAYER_H

#include "base_player.h"
#include <string>

class Worm;
class PlayerOptions;

class ProxyPlayer : public BasePlayer
{
public:

	ProxyPlayer(BaseWorm* worm);
	~ProxyPlayer();
	
	void subThink();
#ifndef DEDSERV
	void render();
#endif
private:

};

#endif  // _WORM_H_
