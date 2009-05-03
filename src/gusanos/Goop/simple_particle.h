#ifndef GUSANOS_BLOOD_H
#define GUSANOS_BLOOD_H

#include "base_object.h"
#include "util/vec.h"

class SimpleParticle : public BaseObject
{
public:
		
	SimpleParticle(Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), BasePlayer* owner = NULL, int timeout_ = 0, float gravity_ = 0.f, int colour_ = 0)
	: BaseObject(owner, pos_, spd_), timeout(timeout_), colour(colour_)
	, gravity(gravity_)
	//, gravity(int(gravity_ * 256.f))
	/*, posx(int(pos_.x * 256.f)), posy(int(pos_.y * 256.f))
	, spdx(int(spd_.x * 256.f)), spdy(int(spd_.y * 256.f))*/
	
	{
	
	}

	void draw(Viewport* viewport);
	void think();
		
	void* operator new(size_t count);
	
	void operator delete(void* block);
	
protected:
	
	int timeout;
	int colour;
	//int gravity;
	float gravity;
/*
	int posx;
	int posy;
	int spdx;
	int spdy;*/
};

#ifndef DEDSERV
class SimpleParticle32 : public SimpleParticle
{
public:
		
	SimpleParticle32(Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), BasePlayer* owner = NULL, int timeout_ = 0, float gravity_ = 0.f, int colour_ = 0)
	: SimpleParticle(pos_, spd_, owner, timeout_, gravity_, colour_)
	{
	
	}
	
	void draw(Viewport* viewport);
};

class SimpleParticle16 : public SimpleParticle
{
public:
		
	SimpleParticle16(Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), BasePlayer* owner = NULL, int timeout_ = 0, float gravity_ = 0.f, int colour_ = 0)
	: SimpleParticle(pos_, spd_, owner, timeout_, gravity_, colour_)
	{
	
	}
	
	void draw(Viewport* viewport);
};

class SimpleParticle32wu : public SimpleParticle
{
public:
		
	SimpleParticle32wu(Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), BasePlayer* owner = NULL, int timeout_ = 0, float gravity_ = 0.f, int colour_ = 0)
	: SimpleParticle(pos_, spd_, owner, timeout_, gravity_, colour_)
	{
	
	}
	
	void draw(Viewport* viewport);
};

class SimpleParticle16wu : public SimpleParticle
{
public:
		
	SimpleParticle16wu(Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), BasePlayer* owner = NULL, int timeout_ = 0, float gravity_ = 0.f, int colour_ = 0)
	: SimpleParticle(pos_, spd_, owner, timeout_, gravity_, colour_)
	{
	
	}
	
	void draw(Viewport* viewport);
};
#endif


#endif  // GUSANOS_BLOOD_H
