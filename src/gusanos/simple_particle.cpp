#include "simple_particle.h"

#include "CVec.h"
#include "gusgame.h"
#include "game/CGameObject.h"
#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "blitters/blitters.h"
#include "CViewport.h"
#endif
#include "game/CMap.h"

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
	velocity().write().y += gravity;
	CVec nextPos = pos() + velocity();
	if(!game.gameMap()->getMaterial(int(nextPos.x), int(nextPos.y)).particle_pass
	|| --timeout == 0)
		deleteMe = true;
	else
		pos() = nextPos;
	
	/*
	spdy += gravity;
	
	if(!game.gameMap()->getMaterial(posx >> 8, posy >> 8).particle_pass
	|| --timeout == 0)
		deleteMe = true;
		
	posx += spdx;
	posy += spdy;
	*/
}

#ifndef DEDICATED_ONLY
void SimpleParticle::draw(CViewport* viewport)
{
	// TODO: the viewport surface is most likely now our real
	// video surface, and thus *has* alpha (where Gus functions)
	// did not expect alpha. This has to be fixed here.
	// (And maybe at many other places, too...)
	IVec rPos = viewport->convertCoords(IVec(Vec(pos())));
	if(!wupixel)
	    putpixel2x2(viewport->dest, rPos.x, rPos.y, colour);
		//putpixel(where, (posx >> 8)-xOff, (posy >> 8)-yOff, colour);
	else {
		for(short dy = 0; dy < 2; ++dy)
		for(short dx = 0; dx < 2; ++dx)
		Blitters::putpixelwu_blend_32(viewport->dest, rPos.x+dx, rPos.y+dy, colour, 256);
	}
}
#endif
