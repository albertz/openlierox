#include "explosion.h"

#include "util/vec.h"
#include "game.h"
#include "base_object.h"
#include "base_worm.h"
#include "base_player.h"
#include "exp_type.h"
#include "events.h"
#ifndef DEDSERV
#include "viewport.h"
#include "distortion.h"
#include "gfx.h"
#include "base_animator.h"
#include "sprite.h"
#include "sprite_set.h"
#include "animators.h"
#endif
#include "detect_event.h"

#include <vector>

using namespace std;

Explosion::Explosion(ExpType *type, const Vec& _pos, BasePlayer* owner) : BaseObject(owner)
{
	pos = _pos;
	
	m_type = type;
	
#ifndef DEDSERV
	m_alpha = m_type->alpha;
	
	m_timeout = m_type->timeout + (int)( rnd() * (m_type->timeoutVariation+1) ); 
	// I add +1 or variation will always be 0 if the value of timeout variation is 1
	
	if ( m_type->destAlpha >= 0 )
	{
		m_fadeSpeed = ( m_type->destAlpha - m_alpha ) / m_timeout;
	}else m_fadeSpeed = 0;
	
	
	if ( (m_sprite = m_type->sprite) )
	{
		m_animator = new AnimRightOnce( m_sprite, m_timeout+2);
	}
	else m_animator = 0;
#endif

	if ( type->creation )
	{
		type->creation->run(this);
	}

	for ( vector< DetectEvent* >::iterator t = type->detectRanges.begin(); t != type->detectRanges.end(); ++t )
	{
		(*t)->check(this);
	}
	
#ifdef DEDSERV
	deleteMe = true; // We have no use of explosions except for the first frame
#else
	if(type->invisible)
		deleteMe = true;
#endif
}

Explosion::~Explosion()
{
#ifndef DEDSERV
	delete m_animator;
#endif
}

#ifndef DEDSERV

void Explosion::think()
{
	// Animation
	if ( m_animator ) m_animator->tick();
	
	// Alpha Fade
	if ( ( m_type->blender || m_type->lightHax ) && m_fadeSpeed )
	{
		if ( fabs( m_type->destAlpha - m_alpha ) < fabs(m_fadeSpeed) )
		{
			m_fadeSpeed = 0;
			m_alpha = m_type->destAlpha;
		}
		else
			m_alpha += m_fadeSpeed;
	}
	
	if ( --m_timeout < 0)
	{
		deleteMe = true;
	}
}

void Explosion::draw(Viewport* viewport)
{

	BITMAP* where = viewport->dest;

	IVec rPos = viewport->convertCoords( IVec( pos ) );
	Vec rPosPrec = viewport->convertCoordsPrec( pos );
	int x = rPos.x;
	int y = rPos.y;
	
	BlitterContext blitter(m_type->blender, (int)m_alpha);
	
	if (!m_sprite)
	{
		if(!m_type->invisible)
		{
			if(m_type->wupixels)
				blitter.putpixelwu(where, rPosPrec.x, rPosPrec.y, m_type->colour);
			else
				blitter.putpixel(where, x, y, m_type->colour);
		}
	}
	else
	{	
		if ( !m_type->rockHidden )
		{
			m_sprite->getSprite(m_animator->getFrame(), Angle(0))->draw(where, x, y, blitter);
		}
		else
		{
			game.level.specialDrawSprite( m_sprite->getSprite(m_animator->getFrame(), Angle(0)), where, rPos, IVec(pos), blitter );
		}
	}
	if (m_type->distortion)
	{
		m_type->distortion->apply( where, x, y, m_type->distortMagnitude );
	}
	if ( m_type->lightHax )
	{
		game.level.culledDrawLight( m_type->lightHax, viewport, IVec(pos), (int)m_alpha );
	}
}

#endif
