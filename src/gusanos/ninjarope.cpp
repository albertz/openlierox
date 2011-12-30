#include "ninjarope.h"

#include "util/vec.h"
#include "util/macros.h"
#include "gusgame.h"
#include "CGameObject.h"
#include "part_type.h"
#ifndef DEDICATED_ONLY
#include "sprite_set.h"
#include "sprite.h"
#include "base_animator.h"
#include "animators.h"
#include "CViewport.h"
#endif
#include "part_type.h"
#include "CMap.h"

#include <vector>

using namespace std;

NinjaRope::NinjaRope(PartType *type, CGameObject* worm)
: m_worm(worm)
{
	justCreated = false;
	active = false;
	attached = false;
	m_type = type;

	m_length = 0;
	m_angle = 0;
	m_angleSpeed = 0;
	m_animator = NULL;
	m_sprite = NULL;
		
	if(m_type != NULL) {
#ifndef DEDICATED_ONLY
		m_sprite = m_type->sprite;	
		m_animator = m_type->allocateAnimator();
#endif
		
		// Why this?? :OO // Re: Modders may want to make the rope leave trails or sth :o
		foreach(i, m_type->timer)
		{
			timer.push_back( (*i)->createState() );
		}

	}
	else
		errors << "NinjaRope::NinjaRope: particle type is NULL" << endl;
}

void NinjaRope::shoot(Vec _pos, Vec _spd)
{
	if(m_type == NULL) return;
	
	pos() = CVec(_pos);
	velocity() = CVec(_spd);
	m_length = gusGame.options.ninja_rope_startDistance;
	
	justCreated = true;
	active = true;
	attached = false;
	
	m_angle = Vec(velocity()).getAngle();
	m_angleSpeed = 0;
	
	foreach(t, timer)
	{
		t->reset();
	}
}

void NinjaRope::remove()
{
	active = false;
	justCreated = false;
	attached = false;
}

void NinjaRope::think()
{
	if ( m_length > gusGame.options.ninja_rope_maxLength )
		m_length = gusGame.options.ninja_rope_maxLength;

	if (!active)
		return;
	
	if(m_type == NULL)
		return;
		
	if ( justCreated && m_type->creation )
	{
		m_type->creation->run(this);
		justCreated = false;
	}
	
	for ( int i = 0; !deleteMe && i < m_type->repeat; ++i)
	{
		pos() += velocity();
		
		BaseVec<long> ipos = BaseVec<long>(Vec(pos()));
		
		// TODO: Try to attach to worms/objects
				
		Vec diff(m_worm->pos(), pos());
		float curLen = (float)diff.length();
		Vec force(diff * gusGame.options.ninja_rope_pullForce);
		
		/*
		if(<attached to object>)
		{
			//Apply force to object
		}
		else
		*/
		if(!gusGame.level().getMaterial( ipos.x, ipos.y ).particle_pass)
		{
			if(!attached)
			{
				m_length = 450.f / 16.f - 1.0f;
				attached = true;
				velocity() = CVec();
				if ( m_type->groundCollision  )
					m_type->groundCollision->run(this);
			}
		}
		else
			attached = false;
			
		if(attached)
		{
			if(curLen > m_length)
			{
				m_worm->addSpeed(force / curLen);
			}
		}
		else
		{
			velocity().y += m_type->gravity;
			
			if(curLen > m_length)
			{
				velocity() -= CVec(force) / curLen;
			}
		}
		
#ifndef DEDICATED_ONLY
		if ( m_animator )
			m_animator->tick();
#endif		
	}
}

Angle NinjaRope::getPointingAngle()
{
	return m_angle;
}

void NinjaRope::addAngleSpeed( AngleDiff speed )
{
	m_angleSpeed += speed;
}

int NinjaRope::getColour()
{
	if(m_type)
		return m_type->colour;
	else
		return 0;
}

CVec& NinjaRope::getPosReference()
{
	return pos();
}

#ifndef DEDICATED_ONLY
void NinjaRope::draw(CViewport *viewport)
{
	if(m_type == NULL) return;
	
	ALLEGRO_BITMAP* where = viewport->dest;
	IVec rPos = viewport->convertCoords( IVec(Vec(pos())) );
	if (active)
	{
		if (!m_sprite)
			putpixel(where,rPos.x,rPos.y,m_type->colour);
		else
		{
			m_sprite->getSprite(m_animator->getFrame(), m_angle)->draw(where,rPos.x,rPos.y);
		}
		if (m_type->distortion)
		{
			m_type->distortion->apply( where, rPos.x,rPos.y, m_type->distortMagnitude );
		}
	}
}
#endif
