#include "gusanos/allegro.h"

#include "gconsole.h"
#include "resource_list.h"
#include "sprite.h"

#include "level.h"
#include "gusgame.h"
#include "updater.h"
#include "part_type.h"
#include "particle.h"
#include "worm.h"
#include "CWormHuman.h"
#include "util/macros.h"

#ifndef DEDICATED_ONLY
#include "CViewport.h"
#include "font.h"
#include "gfx.h"
#include "sound/sfx.h"
#include "menu.h"
#include "distortion.h"
#include "keyboard.h"
#endif
#include "sprite_set.h"
#include "player_ai.h"
#include "network.h"
#include "CMap.h"


#include "script.h"
#include "glua.h"
#include "lua51/luaapi/context.h"
#include "util/log.h"
#include "game/Game.h"
#include <memory>
#include <string>
#include <vector>

#ifdef POSIX
#include <unistd.h>
#endif

using namespace std;

bool quit = false;
int showFps = 1;
int showDebug = 0;

static unsigned int fpsLast = 0;
static int fpsCount = 0;
static int fps = 0;
static Uint32 logicLast = 0;

static bool debug_onlyOneLogicFrame = false;


bool gusInitBase() {
	console.registerVariables()
	("CL_SHOWFPS", &showFps, 1) 
	("CL_SHOWDEBUG", &showDebug, 0)
	;
	
	console.registerVariables()
			("SV_DEBUG_onlyOneLogicFrame", &debug_onlyOneLogicFrame, false)
			;
	
	if(!gusGame.init())
		return false;
	
	console.parseLine("BIND F12 SCREENSHOT");
	
	//gusGame.refreshLevels();

	
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.clear();
#endif
	//gusGame.loadMod();
	gusGame.reloadModWithoutMap();
	//gusGame.runInitScripts();
	
	// TODO: check bDedicated instead
#ifndef DEDICATED_ONLY
	console.executeConfig("autoexec.cfg");
#else
	console.executeConfig("autoexec-ded.cfg");
#endif
	

	return true;
}

bool gusInit(const std::string& mod) {
	notes << "Gusanos: set mod " << mod << endl;
	
	gusGame.setMod(mod);
	
	quit = false;
	fpsLast = 0;
	fpsCount = 0;
	fps = 0;
	logicLast = SDL_GetTicks() / 10;
	
	return true;
}

bool gusCanRunFrame() {
	return !quit || !network.isDisconnected();
}

void gusLogicFrame() {
	Uint32 timer = SDL_GetTicks() / 10;
	
	while ( logicLast + 1 <= timer )
	{
		
#ifdef USE_GRID
		for ( Grid::iterator iter = game.objects.beginAll(); iter;)
		{
			if(iter->deleteMe)
				iter.erase();
			else
				++iter;
		}
#else
		for ( ObjectsList::Iterator iter = game.objects.begin();  iter; )
		{
			if ( (*iter)->deleteMe )
			{
				ObjectsList::Iterator tmp = iter;
				++iter;
				delete *tmp;
				game.objects.erase(tmp);
			}
			else
				++iter;
		}
#endif
		
		if ( gusGame.isLoaded() && gusGame.isLevelLoaded() )
		{
			
#ifdef USE_GRID
			
			for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter)
			{
				iter->think();
				game.objects.relocateIfNecessary(iter);
			}
			
			game.objects.flush(); // Insert all new objects
#else
			for ( ObjectsList::Iterator iter = game.objects.begin(); (bool)iter; ++iter)
			{
				(*iter)->think();
			}
#endif
			
			for ( list<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
			{
				if(!(*iter)->deleteMe)
					(*iter)->think();
			}
		}
		
		gusGame.think();
		updater.think(); // TODO: Move?
		
#ifndef DEDICATED_ONLY
		sfx.think(); // WARNING: THIS MUST! BE PLACED BEFORE THE OBJECT DELETE LOOP
#endif
		
		//for ( list<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end();)
		foreach_delete(iter, game.players)
		{
			if ( (*iter)->deleteMe )
			{
				/* Done in deleteThis()
				 #ifdef USE_GRID
				 for (Grid::iterator objIter = game.objects.beginAll(); objIter; ++objIter)
				 {
				 objIter->removeRefsToPlayer(*iter);
				 }
				 #else
				 for ( ObjectsList::Iterator objIter = game.objects.begin(); (bool)objIter; ++objIter)
				 {
				 (*objIter)->removeRefsToPlayer(*iter);
				 }
				 #endif
				 */
				if ( CWormHumanInputHandler* player = dynamic_cast<CWormHumanInputHandler*>(*iter) )
				{
					foreach ( p, game.localPlayers )
					{
						if ( player == *p )
						{
							game.localPlayers.erase(p);
							break;
						}
					}
				}
				/*
				 (*iter)->removeWorm();
				 */
				(*iter)->deleteThis();
				game.players.erase(iter);
			}
		}
		
		network.update();
		
#ifndef DEDICATED_ONLY
		console.checkInput();
#endif
		console.think();
		
		spriteList.think();
		
		EACH_CALLBACK(i, afterUpdate)
		{
			(lua.call(*i))();
		}
		
		++logicLast;

		if(debug_onlyOneLogicFrame) break;
	}

#ifndef DEDICATED_ONLY
	//Update FPS
	if (fpsLast + 100 <= timer)
	{
		fps = fpsCount;
		fpsCount = 0;
		fpsLast = timer;
	}
#endif	
}

void gusRenderFrameMenu() {	
#ifndef DEDICATED_ONLY
	if ( gusGame.isLoaded() && gusGame.level().gusIsLoaded() )
	{

		for ( list<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
		{
			//(*iter)->render();
		}

		//debug info
		if (showDebug)
		{
			gusGame.infoFont->draw(gfx.buffer, "OBJECTS: \01303" + cast<string>(game.objects.size()), 5, 10, 0, 255, 255, 255, 255, Font::Formatting);
			gusGame.infoFont->draw(gfx.buffer, "PLAYERS: \01303" + cast<string>(game.players.size()), 5, 15, 0, 255, 255, 255, 255, Font::Formatting);
			gusGame.infoFont->draw(gfx.buffer, "LUA MEM: \01303" + cast<string>(lua_gc(lua, LUA_GCCOUNT, 0)), 5, 20, 0, 255, 255, 255, 255, Font::Formatting);
		}
					
		int miny = 150;
		int maxw = 160;
		int y = 235;
		int w = 0;
		
		std::list<ScreenMessage>::reverse_iterator rmsgiter = gusGame.messages.rbegin();
		
		for(;
		rmsgiter != gusGame.messages.rend() && y > miny;
		++rmsgiter)
		{
			ScreenMessage const& msg = *rmsgiter;
			
			string::const_iterator b = msg.str.begin(), e = msg.str.end(), n;
			
			do
			{
				pair<int, int> dim;
				n = gusGame.infoFont->fitString(b, e, maxw, dim, 0, Font::Formatting);
				if(n == b)
					break;
				b = n;
				y -= dim.second;
				
				if(dim.first > w)
					w = dim.first;
			}
			while(b != e);
		}
		
		//rectfill_blend(gfx.buffer, 3, y-2, 3+w+5, 237, 0, 130);
		
		for(std::list<ScreenMessage>::iterator msgiter = rmsgiter.base();
			msgiter != gusGame.messages.end();
			++msgiter)
		{
			ScreenMessage const& msg = *msgiter;
			
			string::const_iterator b = msg.str.begin(), e = msg.str.end(), n;
			
			int fact = 255;
			if(msg.timeOut < 100)
				fact = msg.timeOut * 255 / 100;
			
			Font::CharFormatting format;
			switch(msg.type)
			{
				case ScreenMessage::Chat:
					format.cur.color = Font::Color(255, 255, 255);
				break;
				
				case ScreenMessage::Death:
					format.cur.color = Font::Color(200, 255, 200);
				break;
			}
			
			do
			{
				pair<int, int> dim;
				n = gusGame.infoFont->fitString(b, e, maxw, dim, 0, Font::Formatting);
				if(n == b)
					break;
				gusGame.infoFont->draw(gfx.buffer, b, n, 5, y, format, 0, fact, Font::Formatting | Font::Shadow);
				y += dim.second;
				
				b = n;
			}
			while(b != e);
		}
	}
	else
	{
		clear_bitmap(gfx.buffer);
	}

	//show fps
	if (showFps)
	{
		gusGame.infoFont->draw(gfx.buffer, "FPS: \01303" + cast<string>(fps), 5, 5, 0, 255, 255, 255, 255, Font::Formatting);
	}
	fpsCount++;
	
	if(quit)
		gusGame.infoFont->draw(gfx.buffer, "Quitting...", 15, 110, 0, 255, 255, 255, 255);

	OmfgGUI::menu.render();
	console.render(gfx.buffer);

	EACH_CALLBACK(i, afterRender)
	{
		//lua.callReference(*i);
		(lua.call(*i))();
	}
	
	//gfx.updateScreen();
#endif
}

void gusQuit() {
	//network.disconnect(); // If we haven't already, it's too late
	network.shutDown();
	gusGame.unload();
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.destroy();
#endif
	console.shutDown();
#ifndef DEDICATED_ONLY
	sfx.shutDown();
#endif
	gfx.shutDown();
	lua.close();

	allegro_exit();
}

