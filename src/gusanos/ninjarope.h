#ifndef NINJAROPE_H
#define NINJAROPE_H

#include "CGameObject.h"
#include "events.h"
#include "util/vec.h"
#include "util/angle.h"
#include "timer_event.h"
#include "particle.h"
#include <vector>
//#include <boost/variant.hpp>

#ifndef DEDICATED_ONLY
class SpriteSet;
class BaseAnimator;
#endif
class PartType;
//class CWorm;

class NinjaRope : public CGameObject
{
public:

	NinjaRope(CGameObject* worm);
	
	void shoot(Vec _pos, Vec _spd);
	void remove();

#ifndef DEDICATED_ONLY
	void draw(CViewport *viewport);
#endif
	void think();

	Angle getPointingAngle();
	void addAngleSpeed(AngleDiff);
	
	void addLength(float length_)
	{
		m_length += length_;
		if ( m_length < 0.f )
			m_length = 0.f;
	}
	
	int getColour();
	CVec& getPosReference();
	float& getLengthReference()
	{
		return m_length;
	}
	
	bool active;
	bool attached;
	
private:
	
	std::vector< TimerEvent::State > timer;
	Angle m_angle;
	AngleDiff m_angleSpeed;
	float m_length;
	CGameObject* m_worm;
#ifndef DEDICATED_ONLY
	SpriteSet* m_sprite;
	BaseAnimator* m_animator;
#endif
	
	bool justCreated;
};

#endif  // _PARTICLE_H_
