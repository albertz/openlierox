#ifndef PLAYER_H
#define PLAYER_H

#include "base_player.h"
#include <string>

#ifndef DEDICATED_ONLY
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
#ifndef DEDICATED_ONLY
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
#ifndef DEDICATED_ONLY
	Viewport* m_viewport;
#endif
};

#endif  // _WORM_H_
