#ifndef DEDICATED_ONLY

#include "CViewport.h"

#include "game.h"
#include "sfx.h"
#include "gfx.h"
#include "gusanos/allegro.h"
#include "base_worm.h"
#include "base_player.h"
#include "player.h"
#include "glua.h"
#include "lua/bindings-gfx.h"
#include "blitters/blitters.h"
#include "culling.h"
#include "CMap.h"
#include <list>

#include "sprite_set.h" // TEMP
#include "sprite.h" // TEMP

#include <iostream>

using namespace std;

void CViewport::gusInit()
{
	dest = 0;
	hud = 0;

	lua.pushFullReference(*this, LuaBindings::CViewportMetaTable);
	//lua.pushLightReference(this, LuaBindings::viewportMetaTable);
	luaReference = lua.createReference();
}

void CViewport::gusShutdown()
{
	lua.destroyReference(luaReference);
	destroy_bitmap(dest);
	destroy_bitmap(hud);
	sfx.freeListener(m_listener);
	destroy_bitmap(fadeBuffer);
}

struct TestCuller : public Culler<TestCuller>
{
	TestCuller(BITMAP* dest_, BITMAP* src_, int scrOffX_, int scrOffY_, int destOffX_, int destOffY_, Rect const& rect)
			: Culler<TestCuller>(rect), dest(dest_), src(src_),
			scrOffX(scrOffX_), scrOffY(scrOffY_), destOffX(destOffX_), destOffY(destOffY_)
	{
	}

	bool block(int x, int y)
	{
		return !game.level().unsafeGetMaterial(x, y).worm_pass;
	}

	void line(int y, int x1, int x2)
	{
		//hline_add(dest, x1 + scrOffX, y + scrOffY, x2 + scrOffX + 1, makecol(50, 50, 50), 255);


		drawSpriteLine_add(
		    dest,
		    src,
		    x1 + scrOffX,
		    y + scrOffY,
		    x1 + destOffX,
		    y + destOffY,
		    x2 + destOffX + 1,
		    255);
	}

	BITMAP* dest;
	BITMAP* src;

	int scrOffX;
	int scrOffY;
	int destOffX;
	int destOffY;
};

static BITMAP* testLight = 0;

void CViewport::setDestination(BITMAP* where, int x, int y, int width, int height)
{
	if(width > where->w
	        || height > where->h)
		return;

	destroy_bitmap(dest);
	destroy_bitmap(hud);
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;
	if ( x + width > where->w )
		x = where->w - width;
	if ( y + height > where->h )
		y = where->h - height;
	dest = create_sub_bitmap(where,x,y,width,height);
	hud = create_sub_bitmap(where,x,y,width,height);

	m_listener = sfx.newListener();

	fadeBuffer = create_bitmap_ex(8, width, height);

	if(!testLight) {
		static int s = 500;
		testLight = create_bitmap_ex(8, s, s);

		for(int y = 0; y < s; ++y)
			for(int x = 0; x < s; ++x) {
				double v = 1.0*(double(s)/2 - (IVec(x, y) - IVec(s/2, s/2)).length());
				if(v < 0.0)
					v = 0.0;
				int iv = int(v);
				putpixel_solid(testLight, x, y, iv);
			}
	}
}

void CViewport::drawLight(IVec const& v)
{
	IVec off(Left,Top);
	IVec loff(v - IVec(testLight->w/2, testLight->h/2));

	Rect r(0, 0, game.level().GetWidth() - 1, game.level().GetHeight() - 1);
	r &= Rect(testLight) + loff;

	TestCuller testCuller(fadeBuffer, testLight, -off.x, -off.y, -loff.x, -loff.y, r);

	testCuller.cullOmni(v.x, v.y);
}


void CViewport::render(BasePlayer* player)
{
	int offX = static_cast<int>(Left);
	int offY = static_cast<int>(Top);

	game.level().gusDraw(dest, offX, offY);

	if ( game.level().config()->darkMode && game.level().lightmap )
		blit( game.level().lightmap, fadeBuffer, offX,offY, 0, 0, fadeBuffer->w, fadeBuffer->h );

#ifdef USE_GRID

	for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter) {
		//iter->draw(dest, offX, offY);
		iter->draw(this);
	}
#else
	for ( int i = 0; i < RENDER_LAYERS_AMMOUNT ; ++i) {
		ObjectsList::RenderLayerIterator iter;
		for ( iter = game.objects.renderLayerBegin(i); (bool)iter; ++iter) {
			//(*iter)->draw(dest, offX, offY);
			(*iter)->draw(this);
		}
	}
#endif

	/*
		static double a = 0.0;
		a += 0.003;
		*/


#if 0
	if(gfx.m_haxWormLight) {
		BasePlayer* player = game.localPlayers[0];

		BaseWorm* worm = player->getWorm();
		if(worm->isActive()) {
			IVec v(worm->pos);
			drawLight(v);
		}
	}
#endif

	if(game.level().config()->darkMode)
		drawSprite_mult_8(dest, fadeBuffer, 0, 0);

	EACH_CALLBACK(i, wormRender) {
		for(list<BasePlayer*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter) {
			BaseWorm* worm = (*playerIter)->getWorm();
			if( worm && worm->isActive() ) {
				IVec renderPos( worm->getRenderPos() );
				int x = renderPos.x - offX;
				int y = renderPos.y - offY;
				//bool ownViewport = (*playerIter == player);
				LuaReference ownerRef;

				if ( player )
					ownerRef = player->getLuaReference();

				//lua.callReference(0, *i, (lua_Number)x, (lua_Number)y, worm->luaReference, luaReference, ownViewport);
				(lua.call(*i), (lua_Number)x, (lua_Number)y, worm->getLuaReference(), luaReference, ownerRef)();
			}
		}
	}

	if(BaseWorm* worm = player->getWorm()) {
		EACH_CALLBACK(i, viewportRender) {
			//lua.callReference(0, *i, luaReference, worm->luaReference);
			(lua.call(*i), luaReference, worm->getLuaReference())();
		}
	}
}


#endif
