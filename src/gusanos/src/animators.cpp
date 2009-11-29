#ifndef DEDSERV

#include "animators.h"

#include "base_animator.h"
#include "sprite_set.h"

#include <allegro.h>

AnimPingPong::AnimPingPong( SpriteSet* sprite, int duration )
: BaseAnimator(0), m_totalFrames(sprite->getFramesWidth())
, m_duration(duration), m_animPos(duration)
{
	if(m_totalFrames == 1)
	{
		 // This will prevent single-frame sprite sets from breaking
		m_totalFrames = 0;
		m_animPos = 1;
	}
}

void AnimPingPong::tick()
{
	if ( freezeTicks <= 0 )
	{
		m_animPos -= m_totalFrames;

		while(m_animPos <= 0)
		{
			m_animPos += m_duration;

			if (m_currentDir == 1)
			{
				++m_frame;
				if(m_frame >= m_totalFrames)
				{
					m_frame -= 2;
					m_currentDir = -1;
				}				
			}
			else
			{
				--m_frame;
				if(m_frame < 0)
				{
					m_frame = 1;
					m_currentDir = 1;
				}
			}
		}
	}
	else
	{
		--freezeTicks;
	}
}

void AnimPingPong::reset()
{
	m_animPos = 0;
	m_frame = 0;
	m_currentDir = 1;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AnimLoopRight::AnimLoopRight( SpriteSet* sprite, int duration )
: BaseAnimator(0), m_totalFrames(sprite->getFramesWidth())
, m_duration(duration), m_animPos(duration)
{
	
}

void AnimLoopRight::tick()
{
	if ( freezeTicks <= 0)
	{
		m_animPos -= m_totalFrames;
		while(m_animPos <= 0)
		{
			m_animPos += m_duration;
			++m_frame;
			if(m_frame >= m_totalFrames)
				m_frame = 0;
		}
	}
	else
	{
		--freezeTicks;
	}
}

void AnimLoopRight::reset()
{
	m_animPos = 0;
	m_frame = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AnimRightOnce::AnimRightOnce( SpriteSet* sprite, int duration )
: BaseAnimator(0), m_totalFrames(sprite->getFramesWidth())
, m_duration(duration), m_animPos(duration)
{

}

void AnimRightOnce::tick()
{
	if ( freezeTicks <= 0)
	{
		m_animPos -= m_totalFrames;
		while(m_animPos <= 0)
		{
			m_animPos += m_duration;
			if(m_frame < m_totalFrames - 1)
				++m_frame;
		}
	}
	else
	{
		--freezeTicks;
	}
}

void AnimRightOnce::reset()
{
	m_animPos = 0;
	m_frame = 0;
}

#endif
