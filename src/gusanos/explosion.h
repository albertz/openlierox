#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "game/CGameObject.h"
#include "exp_type.h"
#include "CVec.h"
//#include "animators.h"
#include <vector>

#ifndef DEDICATED_ONLY
class Sprite;
class BaseAnimator;
#endif
class CWormInputHandler;
class CViewport;

class Explosion : public CGameObject
{
public:
	
	Explosion(ExpType* type, const Vec& _pos = Vec(0,0), CWormInputHandler* owner = NULL);

	~Explosion(); //ZOMG WE HAD FORGOTTEN ABOUT THIS :O LEAKOFDEATH

#ifndef DEDICATED_ONLY
	void draw(CViewport* viewport);
	void think();
#endif
	ExpType* getType()
	{
		return m_type;
	}


private:

	ExpType* m_type;
	
#ifndef DEDICATED_ONLY	
	int m_timeout;
	
	float m_fadeSpeed;
	float m_alpha;
	SpriteSet* m_sprite;
	BaseAnimator* m_animator;
	//AnimLoopRight m_animator;
#endif
};

#endif  // _EXPLOSION_H_
