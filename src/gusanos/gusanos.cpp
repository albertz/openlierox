#include "gusanos/allegro.h"

#include "gconsole.h"
#include "resource_list.h"
#include "sprite.h"

#include "level.h"
#include "gusgame.h"
#include "part_type.h"
#include "particle.h"
#include "CWormHuman.h"
#include "util/macros.h"

#ifndef DEDICATED_ONLY
#include "CViewport.h"
#include "font.h"
#include "gfx.h"
#include "sound/sfx.h"
#include "menu.h"
#include "distortion.h"
#endif
#include "sprite_set.h"
#include "network.h"
#include "game/CMap.h"


#include "script.h"
#include "glua.h"
#include "luaapi/context.h"
#include "util/log.h"
#include "game/Game.h"
#include <memory>
#include <string>
#include <vector>

#ifdef POSIX
#include <unistd.h>
#endif

using namespace std;

static Uint32 logicLast = 0;


bool gusInitBase() {
	if(!gusGame.init())
		return false;
	
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.clear();
#endif

	return true;
}

bool gusInit(const std::string& mod) {
	notes << "Gusanos: set mod " << mod << endl;
	
	gusGame.setMod(mod);
	
	logicLast = SDL_GetTicks() / 10;
	
	return true;
}

void gusLogicFrame() {
	Uint32 timer = SDL_GetTicks() / 10;
	
	size_t logicFrameCount = 0;
	while ( logicLast + 1 <= timer )
	{

		for ( Grid::iterator iter = game.objects.beginAll(); iter;)
		{
			if(iter->deleteMe)
				iter.erase();
			else
				++iter;
		}
		
		if ( game.shouldDoPhysicsFrame() && gusGame.isLoaded() && gusGame.isLevelLoaded() )
		{
						
			for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter)
			{
				iter->think();
				game.objects.relocateIfNecessary(iter);
			}
			
			game.objects.flush(); // Insert all new objects
			
			for ( vector<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
			{
				if(!(*iter)->deleteMe)
					(*iter)->think();
			}
		}
		
		gusGame.think();
		
#ifndef DEDICATED_ONLY
		sfx.think(); // WARNING: THIS MUST! BE PLACED BEFORE THE OBJECT DELETE LOOP
#endif
						
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
		++logicFrameCount;
		
		if(logicFrameCount > 10) { // don't be too slow
			logicLast = timer; // skip left frames
			break;
		}
	}
	
	network.update();
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

