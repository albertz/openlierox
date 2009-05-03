#ifndef DEDSERV

#include "viewport.h"

#include "game.h"
#include "sfx.h"
#include "gfx.h"
#include <allegro.h>
#include "base_worm.h"
#include "base_player.h"
#include "player.h"
#include "glua.h"
#include "lua/bindings-gfx.h"
#include "blitters/blitters.h"
#include "culling.h"
#include <list>

#include "sprite_set.h" // TEMP
#include "sprite.h" // TEMP

#include <iostream>

using namespace std;

Viewport::Viewport()
: dest(0), hud(0)
{
	lua.pushFullReference(*this, LuaBindings::ViewportMetaTable);
	//lua.pushLightReference(this, LuaBindings::viewportMetaTable);
	luaReference = lua.createReference();
}

Viewport::~Viewport()
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
	: dest(dest_), src(src_), scrOffX(scrOffX_), scrOffY(scrOffY_), destOffX(destOffX_), destOffY(destOffY_)
	, Culler<TestCuller>(rect)
	{
		
	}
	
	bool block(int x, int y)
	{
		return !game.level.unsafeGetMaterial(x, y).worm_pass;
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

void Viewport::setDestination(BITMAP* where, int x, int y, int width, int height)
{
	if(width > where->w
	|| height > where->h)
		return;
	
	destroy_bitmap(dest);
	destroy_bitmap(hud);
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( x + width > where->w ) x = where->w - width;
	if ( y + height > where->h ) y = where->h - height;
	dest = create_sub_bitmap(where,x,y,width,height);
	hud = create_sub_bitmap(where,x,y,width,height);
	
	m_listener = sfx.newListener();

	fadeBuffer = create_bitmap_ex(8, width, height);

	if(!testLight)
	{
		static int s = 500;
		testLight = create_bitmap_ex(8, s, s);
		
		for(int y = 0; y < s; ++y)
		for(int x = 0; x < s; ++x)
		{
			double v = 1.0*(double(s)/2 - (IVec(x, y) - IVec(s/2, s/2)).length());
			if(v < 0.0)
				v = 0.0;
			int iv = int(v);
			putpixel_solid(testLight, x, y, iv);
		}
	}
}

void Viewport::drawLight(IVec const& v)
{
	IVec off(m_pos);
	IVec loff(v - IVec(testLight->w/2, testLight->h/2));
	
	Rect r(0, 0, game.level.width() - 1, game.level.height() - 1);
	r &= Rect(testLight) + loff;
	
	TestCuller testCuller(fadeBuffer, testLight, -off.x, -off.y, -loff.x, -loff.y, r);

	testCuller.cullOmni(v.x, v.y);
}


void Viewport::render(BasePlayer* player)
{
	int offX = static_cast<int>(m_pos.x);
	int offY = static_cast<int>(m_pos.y);
	
	game.level.draw(dest, offX, offY);

	if ( game.level.config()->darkMode && game.level.lightmap )
		blit( game.level.lightmap, fadeBuffer, offX,offY, 0, 0, fadeBuffer->w, fadeBuffer->h );

#ifdef USE_GRID
	for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter)
	{
		//iter->draw(dest, offX, offY);
		iter->draw(this);
	}
#else
	for ( int i = 0; i < RENDER_LAYERS_AMMOUNT ; ++i)
	{
		ObjectsList::RenderLayerIterator iter;
		for ( iter = game.objects.renderLayerBegin(i); (bool)iter; ++iter)
		{
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
	if(gfx.m_haxWormLight)
	{
		BasePlayer* player = game.localPlayers[0];
	
		BaseWorm* worm = player->getWorm();
		if(worm->isActive())
		{
			IVec v(worm->pos);
			drawLight(v);
		}
	}
#endif

	if(game.level.config()->darkMode)
		drawSprite_mult_8(dest, fadeBuffer, 0, 0);

	EACH_CALLBACK(i, wormRender)
	{
		for(list<BasePlayer*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
		{
			BaseWorm* worm = (*playerIter)->getWorm();
			if( worm && worm->isActive() )
			{
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
	
	if(BaseWorm* worm = player->getWorm())
	{
		EACH_CALLBACK(i, viewportRender)
		{
			//lua.callReference(0, *i, luaReference, worm->luaReference);
			(lua.call(*i), luaReference, worm->getLuaReference())();
		}
	}
}

void Viewport::setPos(float x, float y)
{
	m_pos.x=x;
	m_pos.y=y;
	
	if (m_listener) m_listener->pos = m_pos + Vec(dest->w/2,dest->h/2);
	
	if ( m_pos.x + dest->w > game.level.width() ) m_pos.x = game.level.width() - dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + dest->h > game.level.height() ) m_pos.y = game.level.height() - dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
	
}

void Viewport::interpolateTo(float x, float y, float factor)
{
	Vec destPos(x-dest->w/2,y-dest->h/2);

	m_pos = m_pos + (destPos-m_pos)*factor;
	

	if (m_listener) m_listener->pos = Vec(x,y);

	if ( m_pos.x + dest->w > game.level.width() ) m_pos.x = game.level.width() - dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + dest->h > game.level.height() ) m_pos.y = game.level.height() - dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
}

void Viewport::interpolateTo(Vec destPos, float factor)
{
	m_pos = m_pos + (destPos-Vec(dest->w/2,dest->h/2)-m_pos)*factor;
	
	if (m_listener) m_listener->pos = destPos;
	
	if ( m_pos.x + dest->w > game.level.width() ) m_pos.x = game.level.width() - dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + dest->h > game.level.height() ) m_pos.y = game.level.height() - dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
}

#endif
