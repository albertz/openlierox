#include <allegro.h>

#include "gconsole.h"
#include "resource_list.h"
#include "sprite.h"

#include "level.h"
#include "game.h"
#include "updater.h"
#include "part_type.h"
#include "particle.h"
#include "worm.h"
#include "player.h"
#include "util/macros.h"
//#include "util/log.h"
#ifndef DEDSERV
#include "mouse.h"
#include "viewport.h"
#include "font.h"
#include "gfx.h"
#include "sfx.h"
#include "menu.h"
#include "distortion.h"
#include "keyboard.h"
#endif
#include "sprite_set.h"
#include "player_ai.h"
#include "network.h"


#include "script.h"
#include "glua.h"
#include "luaapi/context.h"
#include "util/log.h"
#include "http.h"
#include <memory>

#ifdef WINDOWS
	#include <winalleg.h>
#endif

#include <string>
#include <vector>

#ifdef POSIX
#include <unistd.h>
#endif

using namespace std;

bool quit = false;
int showFps;
int showDebug;

//millisecond timer
volatile unsigned int timer = 0;
void timerUpdate(void) { timer++; } END_OF_FUNCTION(timerUpdate);

void exit()
{
	quit = true;
	network.disconnect();
}

string exitCmd(list<string> const& args)
{
	exit();
	return "";
}

int main(int argc, char **argv)
try
{
	console.registerVariables()
		("CL_SHOWFPS", &showFps, 1) 
		("CL_SHOWDEBUG", &showDebug, 0)
	;
	
	game.init(argc, argv);

	console.registerCommands()
		("QUIT", exitCmd)
	;
	
	console.parseLine("BIND F12 SCREENSHOT");

#ifndef DEDSERV
	OmfgGUI::menu.clear();
#endif
	//game.loadMod();
	game.reloadModWithoutMap();
	//game.runInitScripts();
	
	//install millisecond timer
	LOCK_VARIABLE(timer);
	LOCK_FUNCTION(timerUpdate);
	install_int_ex(timerUpdate, BPS_TO_TIMER(100));

	unsigned int fpsLast = 0;
	int fpsCount = 0;
	int fps = 0;
	unsigned int logicLast = 0;
	
#ifndef DEDSERV
	console.executeConfig("autoexec.cfg");
#else
	console.executeConfig("autoexec-ded.cfg");
#endif

	//main game loop
	while (!quit || !network.isDisconnected())
	{

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
			
			if ( game.isLoaded() && game.level.isLoaded() )
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
				
				for ( list<BasePlayer*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
				{
					(*iter)->think();
				}
			}
			
			game.think();
			updater.think(); // TODO: Move?
			
#ifndef DEDSERV
			sfx.think(); // WARNING: THIS ¡MUST! BE PLACED BEFORE THE OBJECT DELETE LOOP
#endif
			
			//for ( list<BasePlayer*>::iterator iter = game.players.begin(); iter != game.players.end();)
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
					if ( Player* player = dynamic_cast<Player*>(*iter) )
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

#ifndef DEDSERV
			console.checkInput();
			mouseHandler.poll();
#endif
			console.think();
			
			spriteList.think();
			
			EACH_CALLBACK(i, afterUpdate)
			{
				(lua.call(*i))();
			}
			
			++logicLast;
		}
		
#ifdef WINDOWS
		Sleep(0);
#else
#ifndef DEDSERV
		rest(0);
#else
		rest(2);
#endif
#endif

#ifndef DEDSERV
		//Update FPS
		if (fpsLast + 100 <= timer)
		{
			fps = fpsCount;
			fpsCount = 0;
			fpsLast = timer;
			
			//console.addLogMsg(cast<string>(fps));
		}


		if ( game.isLoaded() && game.level.isLoaded() )
		{

			for ( list<BasePlayer*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
			{
				(*iter)->render();
			}

			//debug info
			if (showDebug)
			{
				game.infoFont->draw(gfx.buffer, "OBJECTS: \01303" + cast<string>(game.objects.size()), 5, 10, 0, 255, 255, 255, 255, Font::Formatting);
				game.infoFont->draw(gfx.buffer, "PLAYERS: \01303" + cast<string>(game.players.size()), 5, 15, 0, 255, 255, 255, 255, Font::Formatting);
				game.infoFont->draw(gfx.buffer, "PING:    \01303" + cast<string>(network.getServerPing()), 5, 20, 0, 255, 255, 255, 255, Font::Formatting);
				game.infoFont->draw(gfx.buffer, "LUA MEM: \01303" + cast<string>(lua_gc(lua, LUA_GCCOUNT, 0)), 5, 25, 0, 255, 255, 255, 255, Font::Formatting);
			}
						
			int miny = 150;
			int maxw = 160;
			int y = 235;
			int w = 0;
			
			std::list<ScreenMessage>::reverse_iterator rmsgiter = game.messages.rbegin();
			
			for(;
		    rmsgiter != game.messages.rend() && y > miny;
		    ++rmsgiter)
			{
				ScreenMessage const& msg = *rmsgiter;
				
				string::const_iterator b = msg.str.begin(), e = msg.str.end(), n;
				
				do
				{
					pair<int, int> dim;
					n = game.infoFont->fitString(b, e, maxw, dim, 0, Font::Formatting);
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
			    msgiter != game.messages.end();
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
					n = game.infoFont->fitString(b, e, maxw, dim, 0, Font::Formatting);
					if(n == b)
						break;
					game.infoFont->draw(gfx.buffer, b, n, 5, y, format, 0, fact, Font::Formatting | Font::Shadow);
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
			game.infoFont->draw(gfx.buffer, "FPS: \01303" + cast<string>(fps), 5, 5, 0, 255, 255, 255, 255, Font::Formatting);
		}
		fpsCount++;
		
		if(quit)
			game.infoFont->draw(gfx.buffer, "Quitting...", 15, 110, 0, 255, 255, 255, 255);

		OmfgGUI::menu.render();
		console.render(gfx.buffer);

		EACH_CALLBACK(i, afterRender)
		{
			//lua.callReference(*i);
			(lua.call(*i))();
		}
		
		gfx.updateScreen();
#endif
	}
	
	//network.disconnect(); // If we haven't already, it's too late
	network.shutDown();
	game.unload();
#ifndef DEDSERV
	OmfgGUI::menu.destroy();
#endif
	console.shutDown();
#ifndef DEDSERV
	sfx.shutDown();
#endif
	gfx.shutDown();
	lua.close();

	allegro_exit();

	return(0);
}
catch(std::exception& e)
{
	std::cerr << "Unhandled exception: " << e.what() << '\n';
}
catch(...)
{
	std::cerr << "Unknown unhandled exception\n";
}
END_OF_MAIN();

