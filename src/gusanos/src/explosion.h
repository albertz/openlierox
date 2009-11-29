#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "base_object.h"
#include "exp_type.h"
#include "util/vec.h"
//#include "animators.h"
#include <vector>

#ifndef DEDSERV
class Sprite;
class BaseAnimator;
#endif
class BasePlayer;
class Viewport;

class Explosion : public BaseObject
{
public:
	
	Explosion(ExpType* type, const Vec& _pos = Vec(0,0), BasePlayer* owner = NULL);

	~Explosion(); //ZOMG WE HAD FORGOTTEN ABOUT THIS :O LEAKOFDEATH

#ifndef DEDSERV
	void draw(Viewport* viewport);
	void think();
#endif
	ExpType* getType()
	{
		return m_type;
	}


private:

	ExpType* m_type;
	
#ifndef DEDSERV	
	int m_timeout;
	
	float m_fadeSpeed;
	float m_alpha;
	SpriteSet* m_sprite;
	BaseAnimator* m_animator;
	//AnimLoopRight m_animator;
#endif
};

#endif  // _EXPLOSION_H_
