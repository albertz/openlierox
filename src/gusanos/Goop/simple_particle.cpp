#include "simple_particle.h"

#include "util/vec.h"
#include "game.h"
#include "base_object.h"
#ifndef DEDSERV
#include "gfx.h"
#include "blitters/blitters.h"
#include "viewport.h"
#endif

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
	spd.y += gravity;
	Vec nextPos = pos + spd;
	if(!game.level.getMaterial(int(nextPos.x), int(nextPos.y)).particle_pass
	|| --timeout == 0)
		deleteMe = true;
	else
		pos = nextPos;
	
	/*
	spdy += gravity;
	
	if(!game.level.getMaterial(posx >> 8, posy >> 8).particle_pass
	|| --timeout == 0)
		deleteMe = true;
		
	posx += spdx;
	posy += spdy;
	*/
}

#ifndef DEDSERV
void SimpleParticle::draw(Viewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(pos));
	putpixel(viewport->dest, rPos.x, rPos.y, colour);
	//putpixel(where, (posx >> 8)-xOff, (posy >> 8)-yOff, colour);
}

void SimpleParticle32::draw(Viewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(pos));
	BITMAP* where = viewport->dest;

	if((unsigned int)rPos.x < (unsigned int)where->w
	&& (unsigned int)rPos.y < (unsigned int)where->h )
		Blitters::putpixel_solid_32(where, rPos.x, rPos.y, colour);
}

void SimpleParticle16::draw(Viewport* viewport)
{
	IVec rPos = viewport->convertCoords(IVec(pos));
	BITMAP* where = viewport->dest;

	if((unsigned int)rPos.x < (unsigned int)where->w
	&& (unsigned int)rPos.y < (unsigned int)where->h )
		Blitters::putpixel_solid_16(where, rPos.x, rPos.y, colour);
}

void SimpleParticle32wu::draw(Viewport* viewport)
{
	Vec rPos = viewport->convertCoordsPrec( pos );
	Blitters::putpixelwu_blend_32(viewport->dest, rPos.x, rPos.y, colour, 256);
}

void SimpleParticle16wu::draw(Viewport* viewport)
{
	Vec rPos = viewport->convertCoordsPrec( pos );
	Blitters::putpixelwu_blend_16(viewport->dest, rPos.x, rPos.y, colour, 32);
}

#endif
