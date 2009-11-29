#ifndef PLAYER_H
#define PLAYER_H

#include "base_player.h"
#include <string>

#ifndef DEDSERV
class Viewport;
struct BITMAP;
#endif
class Worm;
class PlayerOptions;

class Player : public BasePlayer
{
public:
	
	enum Actions
	{
		LEFT = 0,
		RIGHT,
		UP,
		DOWN,
		FIRE,
		JUMP,
		CHANGE,
		ACTION_COUNT,
	};
			
	Player(shared_ptr<PlayerOptions> options, BaseWorm* worm);
	~Player();
	
	void subThink();
#ifndef DEDSERV
	void render();

	void assignViewport(Viewport* Viewport);
#endif
	void actionStart( Actions action );
	void actionStop( Actions action );
	
private:
	
	bool aimingUp;
	bool aimingDown;
	bool changing;
	bool jumping;
	bool walkingLeft;
	bool walkingRight;
#ifndef DEDSERV
	Viewport* m_viewport;
#endif
};

#endif  // _WORM_H_
