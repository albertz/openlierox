#ifndef DEDICATED_ONLY

#include "CViewport.h"

#include "gusgame.h"
#include "sound/sfx.h"
#include "gfx.h"
#include "gusanos/allegro.h"
#include "CWorm.h"
#include "game/WormInputHandler.h"
#include "CWormHuman.h"
#include "glua.h"
#include "lua/bindings-gfx.h"
#include "blitters/blitters.h"
#include "culling.h"
#include "CMap.h"
#include "game/Game.h"
#include <list>

#include "sprite_set.h" // TEMP
#include "sprite.h" // TEMP
#include "CGameScript.h"

#include <iostream>

using namespace std;

void CViewport::gusInit()
{
	dest = 0;
	hud = 0;
	fadeBuffer = 0;
}

void CViewport::gusReset()
{
	if(luaReference) lua.destroyReference(luaReference); luaReference = LuaReference();
	destroy_bitmap(dest); dest = 0;
	destroy_bitmap(hud); hud = 0;
	destroy_bitmap(fadeBuffer); fadeBuffer = 0;
}

struct TestCuller : public Culler<TestCuller>
{
	TestCuller(ALLEGRO_BITMAP* dest_, ALLEGRO_BITMAP* src_, int scrOffX_, int scrOffY_, int destOffX_, int destOffY_, Rect const& rect)
			: Culler<TestCuller>(rect), dest(dest_), src(src_),
			scrOffX(scrOffX_), scrOffY(scrOffY_), destOffX(destOffX_), destOffY(destOffY_)
	{
	}

	bool block(int x, int y)
	{
		return !gusGame.level().unsafeGetMaterial(x, y).worm_pass;
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

	ALLEGRO_BITMAP* dest;
	ALLEGRO_BITMAP* src;

	int scrOffX;
	int scrOffY;
	int destOffX;
	int destOffY;
};

static ALLEGRO_BITMAP* testLight = 0;

void CViewport::setDestination(ALLEGRO_BITMAP* where, int x, int y, int width, int height)
{
	if(width > where->w
	   || height > where->h) {
		errors << "CViewport::setDestination: " << width << "x" << height << " too big" << endl;
		return;
	}
	
	if(luaReference) lua.destroyReference(luaReference);
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

	destroy_bitmap(fadeBuffer);
	fadeBuffer = create_bitmap_ex(8, width, height);

	if(!testLight) {
		static int s = 500;
		destroy_bitmap(testLight);
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

		
	lua.pushFullReference(*this, LuaBindings::CViewportMetaTable);
	//lua.pushLightReference(this, LuaBindings::viewportMetaTable);
	luaReference = lua.createReference();
}

void CViewport::drawLight(IVec const& v)
{
	IVec off(Left,Top);
	IVec loff(v - IVec(testLight->w/2, testLight->h/2));

	Rect r(0, 0, gusGame.level().GetWidth() - 1, gusGame.level().GetHeight() - 1);
	r &= Rect(testLight) + loff;

	TestCuller testCuller(fadeBuffer, testLight, -off.x, -off.y, -loff.x, -loff.y, r);

	testCuller.cullOmni(v.x, v.y);
}


void CViewport::gusRender()
{
	{
		int destx = Left/2;
		int desty = Top/2;
		int destw = Width;
		int desth = Height;
		bool needDestReset = false;
		if(!dest)
			needDestReset = true;
		else if(dest->sub_x != destx || dest->sub_y != desty || dest->w != destw || dest->h != desth )
			needDestReset = true;
		else if(dest->surf.get() != gfx.buffer->surf.get())
			needDestReset = true;
		
		if(needDestReset)
			setDestination(gfx.buffer, destx, desty, destw, desth);
	}
		
	int offX = static_cast<int>(WorldX);
	int offY = static_cast<int>(WorldY);

	gusGame.level().gusDraw(dest, offX, offY);

	if ( gusGame.level().config()->darkMode && gusGame.level().lightmap )
		blit( gusGame.level().lightmap, fadeBuffer, offX,offY, 0, 0, fadeBuffer->w, fadeBuffer->h );

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
		CWormInputHandler* player = game.localPlayers[0];

		CWorm* worm = player->getWorm();
		if(worm->isActive()) {
			IVec v(worm->pos);
			drawLight(v);
		}
	}
#endif

	if(gusGame.level().config()->darkMode)
		drawSprite_mult_8(dest, fadeBuffer, 0, 0);

	// only use the player/worm specific drawings in gus mods
	if(game.gameScript()->gusEngineUsed()) {
		CWormInputHandler* player = pcTargetWorm ? pcTargetWorm->inputHandler() : NULL;
		
		// Note that we only process worms in the Lua callbacks which have a player set.
		// Most Lua code depends on this assumption so it would break otherwise.
		
		EACH_CALLBACK(i, wormRender) {
			for(list<CWormInputHandler*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter) {
				CWorm* worm = (*playerIter)->getWorm();
				if( worm && worm->isActive() && worm->inputHandler() ) {
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

		if(pcTargetWorm && pcTargetWorm->inputHandler()) {
			EACH_CALLBACK(i, viewportRender) {
				//lua.callReference(0, *i, luaReference, worm->luaReference);
				(lua.call(*i), luaReference, pcTargetWorm->getLuaReference())();
			}
		}
	}
	
	// no gus mod
	else {
		
	}
}


#endif
