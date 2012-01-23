#ifndef PROXY_PLAYER_H
#define PROXY_PLAYER_H

#include "game/WormInputHandler.h"
#include <string>

class Worm;

class ProxyPlayer : public CWormInputHandler
{
public:

	ProxyPlayer(CWorm* worm);
	
	void subThink();
#ifndef DEDICATED_ONLY
	void render();
#endif
	
	void initWeaponSelection() {}
	void doWeaponSelectionFrame(SDL_Surface*, CViewport*) {}
	void getInput() { /* not needed to set LX states, this input handler is only used for Gus right now */ }
	std::string name() { return "ProxyPlayer"; }
	
protected:
	~ProxyPlayer();

};

#endif  // _WORM_H_
