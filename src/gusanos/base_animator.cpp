#ifndef DEDICATED_ONLY

#include "gusanos/base_animator.h"

BaseAnimator::BaseAnimator(int initFrame)
: freezeTicks(0), m_frame(initFrame)
{
	
}

BaseAnimator::~BaseAnimator() // <GLIP> Virtual dtor always needed for classes with virtual functions
{
}

void BaseAnimator::freeze(int ticks)
{
	freezeTicks = ticks;
}

#endif
