#ifndef BASE_ANIMATOR_H
#define BASE_ANIMATOR_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV
	
class BaseAnimator
{
public:
	BaseAnimator(int initFrame = 0);
	virtual ~BaseAnimator(); // <GLIP> Virtual dtor always needed for classes with virtual functions

	int getFrame() const
	{
		return m_frame;
	}
	
	virtual void tick() = 0;
	virtual void reset() = 0;
	void freeze(int ticks);
	
protected:
	
	int freezeTicks;
	int m_frame;
};

#endif // _BASE_ANIMATOR_

