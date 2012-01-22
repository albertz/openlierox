#include "bindings-game.h"
#include "bindings.h"

#include "../luaapi/types.h"
#include "../luaapi/macros.h"
#include "../luaapi/classes.h"

#include "../glua.h"
#include "../gconsole.h"
#include "../gusgame.h"
#include "game/WormInputHandler.h"
#include "CWormHuman.h"
#include "game/CWorm.h"
#include "../level.h"
#include "util/log.h"
#include "util/stringbuild.h"
#include "game/CMap.h"
#include "game/Game.h"
#include "CClientNetEngine.h"
#include "game/SinglePlayer.h"
#include "OLXCommand.h"

#include <cmath>
#include <iostream>
#include <list>
#include "gusanos/allegro.h"
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
using boost::lexical_cast;

namespace LuaBindings
{
	
LuaReference playerIterator(0);
LuaReference CWormInputHandlerMetaTable;

LUA_CALLBACK(luaControl(LuaReference ref, size_t playerIdx, bool state, std::list<std::string> const& args))
	if(playerIdx >= game.localPlayers.size())
		LUA_ABORT();
	game.localPlayers[playerIdx]->pushLuaReference();
	lua.push(state);
	params += 2;
END_LUA_CALLBACK()


//! version 0.9c

/*! console_register_control(name, function)

	Registers a number of console commands that work like player controls.
	
	//function// is a function that is called when the control is activated or
	deactivated. It is of the form:
	<code>
	function(player, state)
	end
	</code>
	
	Where //player// is the CWormHumanInputHandler object the control applies to, and //state// is either true
	or false depending on if the control is activated or deactivated.
	
	The control can be bound to keys like built-in controls such as FIRE and JUMP
	by binding to the command +P//x//_//name// where //x// is the local player number
	and //name// is the name passed to this function.
*/

int l_console_register_control(lua_State* L)
{
	char const* name = lua_tostring(L, 1);
	if(!name) return 0;
	lua_pushvalue(L, 2);
	LuaReference ref = lua.createReference();
	
	for(size_t i = 0; i < GusGame::MAX_LOCAL_PLAYERS; ++i)
	{
		console.registerCommands()
			((S_("+P") << i << '_' << name), boost::bind(LuaBindings::luaControl, ref, i, true, _1), true)
			((S_("-P") << i << '_' << name), boost::bind(LuaBindings::luaControl, ref, i, false, _1), true);
	}
	
	return 0;
}



static int l_olx_getVar(lua_State* L) {
	LuaContext context(L);
	
	// mostly copied from Cmd_getVar
	
	std::string var = lua_tostring(L,1);
	RegisteredVar* varptr = CScriptableVars::GetVar(var);
	if( varptr == NULL ) {
		LUA_ELOG("GetVar: no var with name " + var);
		return 0;
	}
	
	if( varptr->var.type == SVT_CALLBACK ) {
		LUA_ELOG("GetVar: callbacks are not allowed");
		// If we want supoort for that, I would suggest a seperated command like "call ...".
		return 0;
	}
	
	if( varptr->var.ptr.s == &tLXOptions->sServerPassword ) {
		LUA_ELOG("GetVar: this variable is restricted");
		// If you want to check if a worm is authorized, use another function for that.
		return 0;
	}
	
	switch(varptr->var.type) {
		case SVT_BOOL:	lua_pushnumber(L, *varptr->var.ptr.b); break;
		case SVT_INT:	lua_pushnumber(L, *varptr->var.ptr.i); break;
		case SVT_FLOAT:	lua_pushnumber(L, *varptr->var.ptr.f); break;
		default:		lua_pushstring(L, varptr->var.toString().c_str());
	}

	return 1;
}

static int l_olx_setLevelSucceeded(lua_State* L) {
	singlePlayerGame.setLevelSucceeded();
	return 0;
}

		
static int l_olx_message(lua_State* L) {
	std::string msg = lua_tostring(L,1);
	notes << "Lua: OLX message: " << msg << endl;
	cClient->SetPermanentText(msg);
	return 0;
}
		
		
//! version any

/*! game_players()

	Returns an iterator object that returns a CWormHumanInputHandler object
	for every player in the gusGame.
	
	Intended to be use together
	with a for loop, like this:
	<code>
	for p in game_players() do
	    -- Do something with p here
	end
	</code>
*/
int l_game_players(lua_State* L)
{
	lua.pushReference(LuaBindings::playerIterator);
	typedef long iter;
	iter& i = *(iter *)lua_newuserdata_init (L, sizeof(iter));
	i = 0;
	lua_pushnil(L);
	
	return 3;
}

/*! game_local_player(i)

	Returns a CWormHumanInputHandler object of the local player with index i.
	If the index is invalid, nil is returned.
*/
int l_game_localPlayer(lua_State* L)
{
	size_t i = (size_t)lua_tointeger(L, 1);
	if(i < game.localPlayers.size())
	{
		game.localPlayers[i]->pushLuaReference();
		return 1;
	}
	else
		return 0;
}

int l_game_localPlayerName(lua_State* L)
{
	size_t i = (int)lua_tointeger(L, 1);
	if(i < game.localPlayers.size())
	{
		lua.push(game.localPlayers[i]->name());
		return 1;
	}
	else
		return 0;
}


/*! CWormHumanInputHandler:kills()

	Returns the number of kills a player has made.
*/
METHODC(CWormInputHandler, player_kills,  {
	context.push(p->stats->kills);
	return 1;
})


/*! CWormHumanInputHandler:deaths()

	Returns the number of deaths a player has suffered.
*/
METHODC(CWormInputHandler, player_deaths,  {
	context.push(p->stats->deaths);
	return 1;
})

/*! CWormHumanInputHandler:name()

	Returns the name of the player.
*/
METHODC(CWormInputHandler, player_name,  {
	if(p->worm()) {
		context.push(p->worm()->getName().c_str());
		return 1;
	}
	return 0;
})

/*! CWormHumanInputHandler:team()

	Returns the team number of the player.
*/
METHODC(CWormInputHandler, player_team,  {
	if(p->worm()) {
		// +1 because OLX team nrs start at 0, gus team nrs at 1
		context.push(p->worm()->getTeam() + 1);
		return 1;		
	}
	return 0;
})

/*! CWormHumanInputHandler:worm()

	Returns the worm of the player.
*/
METHODC(CWormInputHandler, player_worm,  {
	if(CWorm* worm = p->getWorm())
	{
		worm->pushLuaReference();
		return 1;
	}
	return 0;
})

//! version 0.9c

/*! CWormHumanInputHandler:is_local()

	Returns true if this player is a local player, otherwise false.
*/
METHODC(CWormInputHandler, player_isLocal,  {
	context.push(p->local);
	return 1;
})

//! version any

/*! CWormHumanInputHandler:data()

	Returns a lua table associated with this player.
*/
METHODC(CWormInputHandler, player_data,  {
	if(p->luaData)
		context.pushReference(p->luaData);
	else
	{
		context
			.newtable()
			.pushvalue(-1);
		p->luaData = context.createReference();
	}
	
	return 1;
})

/*! CWormHumanInputHandler:stats()

	Returns a lua table associated with the stats of this player.
*/
METHODC(CWormInputHandler, player_stats,  {
	if(p->stats->luaData)
		context.pushReference(p->stats->luaData);
	else
	{
		context
			.newtable()
			.pushvalue(-1);
		p->stats->luaData = context.createReference();
	}
	
	return 1;
})

/*! CWormHumanInputHandler:say(text)

	Makes the player send 'text' as a chat message.
*/
METHODC(CWormInputHandler, player_say,  {
	char const* s = context.tostring(2);
	CWorm* w = p->worm();
	if(s && w) {
		cClient->getNetEngine()->SendText(s, w->getName());
	}
	return 0;
})

/*! CWormHumanInputHandler:select_weapons(weapons)

	Tries to change the player's weapons to the WeaponType objects
	in the array //weapons//.
*/
METHODC(CWormInputHandler, player_selectWeapons,  {
	
	std::vector<WeaponType *> weapons;
	for(size_t i = 1;; ++i)
	{
		context.rawgeti(2, i);
		if(lua_isnil(context, -1))
		{
			break;
		}
		else
		{
			//WeaponType* weapon = *static_cast<WeaponType **>(lua_touserdata(context, -1));
			WeaponType* weapon = ASSERT_OBJECT_P(WeaponType, -1, "in weapon array");
			weapons.push_back(weapon);
			context.pop(); // Pop value
		}
	}
	
	p->selectWeapons(weapons);
	
	return 0;
})

METHOD(CWormInputHandler, player_destroy, {
	p->removeWorm(); // just to be sure to not access it anymore
	delete p;
	return 0;
})

int l_game_getClosestWorm(lua_State* L)
{
	Vec from((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2));
	
	CWorm* minWorm = 0;
	float minDistSqr = 10000000.f;
	
	for(std::vector<CWormInputHandler*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
	{
		CWorm* worm = (*playerIter)->getWorm();
		
		if(worm->isActive())
		{
			float distSqr = (Vec(worm->pos()) - from).lengthSqr();
			if(distSqr < minDistSqr)
			{
				minDistSqr = distSqr;
				minWorm = worm;
			}
		}
	}
	
	if(!minWorm)
		return 0;
		
	minWorm->pushLuaReference();
	return 1;
}

int l_game_playerIterator(lua_State* L)
{
	typedef long iter;

	iter& i = *(iter *)lua_touserdata(L, 1);
	std::vector<CWormInputHandler*>::iterator it = game.players.begin() + i;
	if(it == game.players.end())
		lua_pushnil(L);
	else
	{
		(*it)->pushLuaReference();
		++i;
	}
	
	return 1;
}


/*! map_is_blocked(x1, y1, x2, y2)

	Returns true if the line between (x1, y1) and (x2, y2) on the map is blocked for particles.
	Otherwise false.
*/
int l_map_isBlocked(lua_State* L)
{
	int x1 = lua_tointeger(L, 1);
	int y1 = lua_tointeger(L, 2);
	int x2 = lua_tointeger(L, 3);
	int y2 = lua_tointeger(L, 4);
	lua_pushboolean(L, game.gameMap()->trace(x1, y1, x2, y2, CMap::ParticleBlockPredicate()));
	return 1;
}

/*! map_is_particle_pass(x1, y1)

	Returns true if the point (x1, y1) on the map is passable by particles.
*/

int l_map_isParticlePass(lua_State* L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);

	lua_pushboolean(L, game.gameMap()->getMaterial(x, y).particle_pass);
	return 1;
}

void initGame()
{
	LuaContext& context = lua;
	
	context.function("game_players", l_game_players);
	lua_pushcfunction(context, l_game_playerIterator);
	playerIterator = context.createReference();
	
	context.functions()
		("game_local_player", l_game_localPlayer)
		("game_local_player_name", l_game_localPlayerName)
		("console_register_control", l_console_register_control)
		("game_get_closest_worm", l_game_getClosestWorm)
		("map_is_blocked", l_map_isBlocked)
		("map_is_particle_pass", l_map_isParticlePass)
	// NOTE: Too much access (like executing any random command or setting any random setting)
	// is not what Lua mod code is supposed to be able to do.
	// It might be that we use Lua also for other stuff later but that should be a separate
	// Lua context then.
	// Here we have some restricted subset.
		("getVar", l_olx_getVar) // This is e.g. used in SinglePlayer to print some message about the key setup.
		("setLevelSucceeded", l_olx_setLevelSucceeded)
		("message", l_olx_message)
	;
	
	// CWormHumanInputHandler method and metatable
	
	CLASSM(CWormInputHandler,  
		("__gc", l_player_destroy)
	,
		("kills", l_player_kills)
		("deaths", l_player_deaths)
		("name", l_player_name)
		("team", l_player_team)
		("say", l_player_say)
		("data", l_player_data)
		("stats", l_player_stats)
		("worm", l_player_worm)
		("select_weapons", l_player_selectWeapons)
		("is_local", l_player_isLocal)
	)
	
	ENUM(Player,  
		("Left", CWormHumanInputHandler::LEFT)
		("Right", CWormHumanInputHandler::RIGHT)
		("Up", CWormHumanInputHandler::UP)
		("Down", CWormHumanInputHandler::DOWN)
		("Fire", CWormHumanInputHandler::FIRE)
		("Jump", CWormHumanInputHandler::JUMP)
		("Change", CWormHumanInputHandler::CHANGE)
		("Ninjarope", CWormHumanInputHandler::NINJAROPE)
	)
	
/*
	lua_newtable(context); 
	lua_pushstring(context, "__index");
	
	lua_newtable(context);
	
	context.tableFunction("kills", l_player_kills);
	context.tableFunction("deaths", l_player_deaths);
	context.tableFunction("name", l_player_name);
	context.tableFunction("say", l_player_say);
	
	lua_rawset(context, -3);
	playerMetaTable = context.createReference();*/
}

}
