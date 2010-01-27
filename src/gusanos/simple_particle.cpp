#include "gusanos/simple_particle.h"

#include "util/vec.h"
#include "gusanos/gusgame.h"
#include "CGameObject.h"
#ifndef DEDICATED_ONLY
#include "gusanos/gfx.h"
#include "gusanos/blitters/blitters.h"
#include "CViewport.h"
#endif
#include "CMap.h"

#define BOOST_NO_MT
#include <boost/pool/pool.hpp>

static boost::pool<> particlePool(sizeof(SimpleParticle));

void* SimpleParticle::operator new(size_t count)
{
	assert(count <= sizeof(SimpleParticle));
	return particlePool.malloc();
}


void SimpleParticle::operator delete(void* block)
{
	particlePool.free(block);
}

void SimpleParticle::think()
{
	velocity().y += gravity;
	CVec nextPos = pos() + velocity();
	if(!gusGame.level().getMaterial(int(nextPos.x), int(nextPos.y)).particle_pass
	|| --timeout == 0)
		deleteMe = true;
	else
		pos() = nextPos;
	
	/*
	spdy += gravity;
	
	if(!gusGame.level().getMaterial(posx >> 8, posy >> 8).particle_pass
	|| --timeout == 0)
		deleteMe = true;
		
	posx += spdx;
	posy += spdy;
	*/
}

#ifndef DEDICATED_ONLY
void SimpleParticle::draw(CViewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(Vec(pos())));
	putpixel(viewport->dest, rPos.x, rPos.y, colour);
	//putpixel(where, (posx >> 8)-xOff, (posy >> 8)-yOff, colour);
}

void SimpleParticle32::draw(CViewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(Vec(pos())));
	ALLEGRO_BITMAP* where = viewport->dest;

	if((unsigned int)rPos.x < (unsigned int)where->w
	&& (unsigned int)rPos.y < (unsigned int)where->h )
		Blitters::putpixel_solid_32(where, rPos.x, rPos.y, colour);
}

void SimpleParticle16::draw(CViewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(Vec(pos())));
	ALLEGRO_BITMAP* where = viewport->dest;

	if((unsigned int)rPos.x < (unsigned int)where->w
	&& (unsigned int)rPos.y < (unsigned int)where->h )
		Blitters::putpixel_solid_16(where, rPos.x, rPos.y, colour);
}

void SimpleParticle32wu::draw(CViewport* viewport)
{
	Vec rPos = viewport->convertCoordsPrec( pos() );
	Blitters::putpixelwu_blend_32(viewport->dest, rPos.x, rPos.y, colour, 256);
}

void SimpleParticle16wu::draw(CViewport* viewport)
{
	Vec rPos = viewport->convertCoordsPrec( pos() );
	Blitters::putpixelwu_blend_16(viewport->dest, rPos.x, rPos.y, colour, 32);
}

#endif
