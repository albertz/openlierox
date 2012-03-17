#include "CNinjaRope.h"

#include "CVec.h"
#include "util/macros.h"
#include "gusgame.h"
#include "game/CGameObject.h"
#include "game/CWorm.h"
#include "part_type.h"
#ifndef DEDICATED_ONLY
#include "sprite_set.h"
#include "sprite.h"
#include "base_animator.h"
#include "animators.h"
#include "CViewport.h"
#endif
#include "part_type.h"
#include "game/CMap.h"
#include "luaapi/context.h"

#include <vector>

using namespace std;

void CNinjaRope::gusInit()
{
	justCreated = false;
	active = false;
	attached = false;

	m_length = 0;
	m_angle = 0;
	m_angleSpeed = 0;
	m_animator = NULL;
	m_sprite = NULL;
		
	if(gusGame.NRPartType != NULL) {
#ifndef DEDICATED_ONLY
		m_sprite = gusGame.NRPartType->sprite;	
		m_animator = gusGame.NRPartType->allocateAnimator();
#endif
		
		// Why this?? :OO // Re: Modders may want to make the rope leave trails or sth :o
		foreach(i, gusGame.NRPartType->timer)
		{
			timer.push_back( (*i)->createState() );
		}

	}
	else {
		if(gusGame.isLoaded()) // if Gus is not loaded, we are not going to use it
			errors << "NinjaRope::NinjaRope: particle type is NULL" << endl;
	}
}

void CNinjaRope::shoot(Vec _pos, Vec _spd)
{
	if(gusGame.NRPartType == NULL) return;
	
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

void CNinjaRope::remove()
{
	active = false;
	justCreated = false;
	attached = false;
}

void CNinjaRope::think()
{
	if (!active)
		return;
	
	if(gusGame.NRPartType == NULL)
		return;
		
	if ( justCreated && gusGame.NRPartType->creation )
	{
		gusGame.NRPartType->creation->run(this);
		justCreated = false;
	}
	
	for ( int i = 0; !deleteMe && i < gusGame.NRPartType->repeat; ++i)
	{
		pos() += velocity();
		
		VectorD2<long> ipos = VectorD2<long>(Vec(pos()));
		
		// TODO: Try to attach to worms/objects
				
		Vec diff = pos() - owner()->pos();
		float curLen = (float)diff.length();
		Vec force(diff * gusGame.options.ninja_rope_pullForce);
		
		/*
		if(<attached to object>)
		{
			//Apply force to object
		}
		else
		*/
		if(!game.gameMap()->getMaterial( ipos.x, ipos.y ).particle_pass)
		{
			if(!attached)
			{
				m_length = (float)(int)gusGame.options.ninja_rope_restLength;
				attached = true;
				velocity() = CVec();
				if ( gusGame.NRPartType->groundCollision  )
					gusGame.NRPartType->groundCollision->run(this);
			}
		}
		else
			attached = false;
			
		if(attached)
		{
			if(curLen > m_length)
			{
				owner()->addSpeed(force / curLen);
			}
		}
		else
		{
			velocity().write().y += gusGame.NRPartType->gravity;
			
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

Angle CNinjaRope::getPointingAngle()
{
	return m_angle;
}

void CNinjaRope::addAngleSpeed( AngleDiff speed )
{
	m_angleSpeed += speed;
}

int CNinjaRope::getColour() const
{
	if(gusGame.NRPartType)
		return gusGame.NRPartType->colour;
	else
		return 0;
}

#ifndef DEDICATED_ONLY
void CNinjaRope::draw(CViewport *viewport)
{
	if(gusGame.NRPartType == NULL) return;
	
	ALLEGRO_BITMAP* where = viewport->dest;
	IVec rPos = viewport->convertCoords( IVec(Vec(pos())) );
	if (active)
	{
		if (!m_sprite)
			putpixel(where,rPos.x,rPos.y,gusGame.NRPartType->colour);
		else
		{
			m_sprite->getSprite(m_animator->getFrame(), m_angle)->draw(where,rPos.x,rPos.y);
		}
		if (gusGame.NRPartType->distortion)
		{
			gusGame.NRPartType->distortion->apply( where, rPos.x,rPos.y, gusGame.NRPartType->distortMagnitude );
		}
	}
}
#endif

void CNinjaRope::deleteThis() {
	notes << "CNinjaRope:deleteThis: " << owner()->getName() << endl;
	
	finalize();
	
	if(luaReference)
	{
		lua.destroyReference(luaReference);
		luaReference.reset();
	}
	
	// dont delete the object itself because we store it in CClient.cWorms atm
	// also dont set deleted=true because we may reuse this object
}
