#ifndef ANIMATORS_H
#define ANIMATORS_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "base_animator.h"

class SpriteSet;

class AnimPingPong : public BaseAnimator
{
public:
	AnimPingPong( SpriteSet* sprite, int duration );
	//~AnimPingPong();
	
	//virtual int getFrame() const;
	virtual void tick();
	virtual void reset();
	
private:

	int m_totalFrames;
	int m_animPos;
	int m_duration;
	//int m_step;
	//int m_max;
	char m_currentDir;
};

class AnimLoopRight : public BaseAnimator
{
public:
	AnimLoopRight( SpriteSet* sprite, int duration );
	//~AnimLoopRight();
	
	//virtual int getFrame() const;
	virtual void tick();
	virtual void reset();
	
private:

	int m_totalFrames;
	int m_animPos;
	int m_duration;
	//int m_step;
	//int m_max;
};

class AnimRightOnce : public BaseAnimator
{
public:
	AnimRightOnce( SpriteSet* sprite, int duration );
	//~AnimRightOnce();

	//virtual int getFrame() const;
	virtual void tick();
	virtual void reset();

private:

	int m_totalFrames;
	int m_animPos;
	int m_duration;
	//int m_step;
	//int m_max;
};

#endif // _ANIMATORS_

