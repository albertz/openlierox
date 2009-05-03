#include "bindings-game.h"
#include "bindings.h"

#include "luaapi/types.h"
#include "luaapi/macros.h"
#include "luaapi/classes.h"

#include "../glua.h"
#include "../gconsole.h"
#include "../game.h"
#include "../base_player.h"
#include "../player_options.h"
#include "../player.h"
#include "../base_worm.h"
#include "../level.h"
#include "util/log.h"
#include "util/stringbuild.h"

#include <cmath>
#include <iostream>
#include <list>
#include <allegro.h>
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
using boost::lexical_cast;

namespace LuaBindings
{
	
LuaReference playerIterator(0);
LuaReference BasePlayerMetaTable;

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
	
	Where //player// is the Player object the control applies to, and //state// is either true
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
	
	for(size_t i = 0; i < Game::MAX_LOCAL_PLAYERS; ++i)
	{
		console.registerCommands()
			((S_("+P") << i << '_' << name), boost::bind(LuaBindings::luaControl, ref, i, true, _1), true)
			((S_("-P") << i << '_' << name), boost::bind(LuaBindings::luaControl, ref, i, false, _1), true);
	}
	
	return 0;
}

//! version any

/*! game_players()

	Returns an iterator object that returns a Player object
	for every player in the game.
	
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
	typedef std::list<BasePlayer*>::iterator iter;
	iter& i = *(iter *)lua_newuserdata (L, sizeof(iter));
	i = game.players.begin();
	lua_pushnil(L);
	
	return 3;
}

/*! game_local_player(i)

	Returns a Player object of the local player with index i.
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
	if(i < game.playerOptions.size())
	{
		lua.push(game.playerOptions[i]->name);
		return 1;
	}
	else
		return 0;
}


/*! Player:kills()

	Returns the number of kills a player has made.
*/
METHODC(BasePlayer, player_kills,
	context.push(p->stats->kills);
	return 1;
)


/*! Player:deaths()

	Returns the number of deaths a player has suffered.
*/
METHODC(BasePlayer, player_deaths,
	context.push(p->stats->deaths);
	return 1;
)

/*! Player:name()

	Returns the name of the player.
*/
METHODC(BasePlayer, player_name,
	context.push(p->m_name.c_str());
	return 1;
)

/*! Player:team()

	Returns the team number of the player.
*/
METHODC(BasePlayer, player_team,
	context.push(p->team);
	return 1;
)

/*! Player:worm()

	Returns the worm of the player.
*/
METHODC(BasePlayer, player_worm,
	if(BaseWorm* worm = p->getWorm())
	{
		worm->pushLuaReference();
		return 1;
	}
	return 0;
)

//! version 0.9c

/*! Player:is_local()

	Returns true if this player is a local player, otherwise false.
*/
METHODC(BasePlayer, player_isLocal,
	context.push(p->local);
	return 1;
)

//! version any

/*! Player:data()

	Returns a lua table associated with this player.
*/
METHODC(BasePlayer, player_data,
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
)

/*! Player:stats()

	Returns a lua table associated with the stats of this player.
*/
METHODC(BasePlayer, player_stats,
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
)

/*! Player:say(text)

	Makes the player send 'text' as a chat message.
*/
METHODC(BasePlayer, player_say,
	char const* s = context.tostring(2);
	if(s)
		p->sendChatMsg(s);
	return 0;
)

/*! Player:select_weapons(weapons)

	Tries to change the player's weapons to the WeaponType objects
	in the array //weapons//.
*/
METHODC(BasePlayer, player_selectWeapons,
	
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
)

METHOD(BasePlayer, player_destroy,
	delete p;
	return 0;
)

int l_game_getClosestWorm(lua_State* L)
{
	Vec from((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2));
	
	BaseWorm* minWorm = 0;
	float minDistSqr = 10000000.f;
	
	for(std::list<BasePlayer*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
	{
		BaseWorm* worm = (*playerIter)->getWorm();
		
		if(worm->isActive())
		{
			float distSqr = (worm->pos - from).lengthSqr();
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
	typedef std::list<BasePlayer*>::iterator iter;
	iter& i = *(iter *)lua_touserdata(L, 1);
	if(i == game.players.end())
		lua_pushnil(L);
	else
	{
		(*i)->pushLuaReference();
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
	lua_pushboolean(L, game.level.trace(x1, y1, x2, y2, Level::ParticleBlockPredicate()));
	return 1;
}

/*! map_is_particle_pass(x1, y1)

	Returns true if the point (x1, y1) on the map is passable by particles.
*/

int l_map_isParticlePass(lua_State* L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);

	lua_pushboolean(L, game.level.getMaterial(x, y).particle_pass);
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
	;
	
	// Player method and metatable
	
	CLASSM(BasePlayer,
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
	
	ENUM(EndReason,
		("ServerQuit", Game::ServerQuit)
		("ServerChangeMap", Game::ServerChangeMap)
		("Kicked", Game::Kicked)
		//("LoadingLevel", Game::LoadingLevel)
		("IncompatibleProtocol", Game::IncompatibleProtocol)
		("IncompatibleData", Game::IncompatibleData)
	)
	
	ENUM(Error,
		("None", Game::ErrorNone)
		("MapNotFound", Game::ErrorMapNotFound)
		("MapLoading", Game::ErrorMapLoading)
		("ModNotFound", Game::ErrorModNotFound)
		("ModLoading", Game::ErrorModLoading)
	)
	
	ENUM(Player,
		("Left", Player::LEFT)
		("Right", Player::RIGHT)
		("Up", Player::UP)
		("Down", Player::DOWN)
		("Fire", Player::FIRE)
		("Jump", Player::JUMP)
		("Change", Player::CHANGE)
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
