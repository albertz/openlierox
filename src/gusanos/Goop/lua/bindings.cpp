#include "bindings.h"

#ifndef DEDSERV
#include "lua/bindings-gui.h"
#endif
#include "bindings-math.h"
#include "bindings-network.h"
#include "bindings-objects.h"
#include "bindings-resources.h"
#include "bindings-gfx.h"
#include "bindings-game.h"

#include "luaapi/types.h"
#include "luaapi/macros.h"

#include "../game.h"
//#include "../vec.h"
//#include "../gfx.h"
#include "../network.h"
#include "../glua.h"
#include "util/log.h"
#include "http.h"

#include "../gconsole.h"
#include "../gusanos.h"
#ifndef DEDSERV
#include "../keys.h"
#include "../menu.h"
#include "../blitters/context.h"
#include "../viewport.h"
#endif
#include <cmath>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <vector>
#include <allegro.h>
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
using boost::lexical_cast;
namespace fs = boost::filesystem;

namespace LuaBindings
{


int print(lua_State* L)
{
	cerr << "LUA: ";
	
	int c = lua_gettop(L);
	for(int i = 1; i <= c; ++i)
	{
		if(const char* s = lua_tostring(L, i))
		{
			cerr << s;
		}
	}
	cerr << '\n';
	
	return 0;
}

/*! bindings.afterUpdate()

	This is called after every logic cycle is complete.
*/

/*@ bindings.atGameStart() // Not sure if this will be kept

	This is called at the beginning of a game.
*/

/*! bindings.afterRender()

	This is called after a rendering cycle is complete
*/

/*! bindings.wormRender(x, y, worm, viewport, ownerPlayer)

	This is called for every worm and viewport combination when it's time to render
	the worm HUD.
	
	(x, y) is the position of the worm in viewport coordinates.
	
	//worm// is the Worm object for which HUD should be rendered and //viewport// is
	the Viewport object it should be rendered to. Use the bitmap() method of Viewport
	to retrieve the relevant bitmap to draw on.
	
	//ownerPlayer// is the Player object that owns //viewport//.
*/

/*! bindings.viewportRender(viewport, worm)

	This is called for every viewport when it's time to render the viewport HUD.
	
	//viewport// is the Viewport object it should be rendered to and //worm// is the
	Worm object of the Player object that owns //viewport//.
*/

/*! bindings.wormDeath(worm)

	This is called when a worm dies. //worm// is the Worm object that died.
*/

/*! bindings.wormRemoved(worm)

	This is called when a worm is removed from the game. //worm// is the Worm object that will be removed.
*/

/*! bindings.playerUpdate(player)

	This is called in every logic cycle for every player. //player// is the relevant Player object.
*/

/*! bindings.playerInit(player)

	This is called when a new player is added to the game. //player// is the Player object that was added.
*/

/*! bindings.playerNetworkInit(player, connID)

	This is called when a player is replicated to a new client. //player// is the player replicated.
	//connID// is the connection ID of the new client.
	
	The connection ID can be passed to the send() method of a NetworkPlayerEvent to send events only
	to the player on the new client.
*/

//! version 0.9c

/*! bindings.playerRemoved(player)
	
	This is called when a player is removed from the game. //player// is the Player object that will be removed.
*/

/*! bindings.gameNetworkInit(connID)
	
	This is called when a new client joins the game. //connID// is the connection ID of the new client.
	
	The connection ID can be passed to the send() method of a NetworkGameEvent to send events only
	to the new client.
*/

/*! bindings.gameEnded(reason)

	This is called when the game ended and no new game is pending.
	
	//reason// can be one of the following:
	* EndReason.ServerQuit : The server disconnected.
	* EndReason.Kicked : You were kicked from the server.
	* EndReason.IncompatibleProtocol : You are running a protocol incompatible with the server's.
	* EndReason.IncompatibleData : Your data does not match the server's.
	
*/

//! version any

int l_bind(lua_State* L)
{
	char const* s = lua_tostring(L, 2);
	if(!s)
		return 0;
		
	lua_pushvalue(L, 3);
	LuaReference ref = lua.createReference();
	luaCallbacks.bind(s, ref);

	return 0;
}

int l_console_set(lua_State* L)
{
	char const* s = lua_tostring(L, 2);
	if(!s)
		return 0;
	
	char const* v = lua_tostring(L, 3);
	if(!v)
		return 0;
	
	std::list<std::string> args;
	args.push_back(v);
	
	console.invoke(s, args, false);
	return 0;
}

int l_console_get(lua_State* L)
{
	char const* s = lua_tostring(L, 2);
	if(!s)
		return 0;
	
	std::list<std::string> args;

	lua_pushstring(L, console.invoke(s, args, false).c_str());
	return 1;
}

int l_console_bind(lua_State* L)
{
	int k = lua_tointeger(L, 1);
	char const* action = lua_tostring(L, 2);
	if(!action)
	{
		if(lua_isnil(L, 2))
			action = "";
		else
			return 0;
	}
	
	console.bind(static_cast<char>(k), action);
	return 0;
}

int l_console_key_for_action(lua_State* L)
{
	char const* s = lua_tostring(L, 1);
	if(!s)
		return 0;
	
	char k = console.getKeyForBinding(s);
	if(k == -1)
		return 0;
	
	lua_pushinteger(L, static_cast<unsigned char>(k));
	return 1;
}

int l_console_action_for_key(lua_State* L)
{
	char k = static_cast<char>(lua_tointeger(L, 1));
	
	std::string action = console.getActionForBinding(k);
	if(action.empty())
		return 0;
	
	lua_pushlstring(L, action.data(), action.size());
	return 1;
}

int l_quit(lua_State* L)
{
	exit();
	return 0;
}

#ifndef DEDSERV
int l_clear_keybuf(lua_State* L)
{
	clear_keybuf();
	return 0;
}

int l_key_name(lua_State* L)
{
	int k = lua_tointeger(L, 1);
	lua_pushstring(L, keyNames[k].c_str());
	return 1;
}
#endif


/*! connect(address)

	Connects to the address passed.
*/
int l_connect(lua_State* L)
{
	char const* s = lua_tostring(L, 1);
	if(!s)
		return 0;
	//network.connect(s);
	console.addQueueCommand(std::string("connect ") + s);
	return 0;
}

/*! host(map)

	Hosts a networked game with the current mod and map //map//.
*/
int l_host(lua_State* L)
{
	char const* map = lua_tostring(L, 1);
	if(!map)
		return 0;

	/*
	game.options.host = 1;
	if(!game.changeLevelCmd( map ))
		return 0;
	lua_pushboolean(L, true);*/
	
	//console.addQueueCommand("host 1");
	//console.addQueueCommand(std::string("map \"") + map + '"');
	game.options.host = 1;
	game.changeLevelCmd( map );
	return 0;
}

/*! map(map)

	Loads the map //map// with the current mod.
*/
int l_map(lua_State* L)
{
	char const* map = lua_tostring(L, 1);
	if(!map)
		return 0;

	/*
	game.options.host = 1;
	if(!game.changeLevelCmd( map ))
		return 0;
	lua_pushboolean(L, true);*/
	
	//console.addQueueCommand(std::string("map \"") + map + '"');
	game.options.host = 0;
	game.changeLevelCmd( map );
	return 0;
}


LUA_CALLBACK(luaConsoleCommand(LuaReference ref, std::list<std::string> const& args))
	for(std::list<std::string>::const_iterator i = args.begin();
		i != args.end();
		++i)
	{
		lua_pushstring(lua, i->c_str());
		++params;
	}
END_LUA_CALLBACK()


/*! console_register_command(name, function)

	Registers the function //function// as a command in the Gusanos console.
	When it is called, it will be passed each console parameter as a seperate parameter to the function.
	The command will be removed automatically when a new map is loaded.
*/
int l_console_register_command(lua_State* L)
{
	char const* name = lua_tostring(L, 1);
	if(!name) return 0;
	lua_pushvalue(L, 2);
	LuaReference ref = lua.createReference();
	
	console.registerCommands()
			(name, boost::bind(LuaBindings::luaConsoleCommand, ref, _1), true);

	return 0;
}

int l_dump(lua_State* L)
{
	LuaContext context(L);
	
	lua_pushvalue(context, 2);
	lua_pushvalue(context, 3);
	lua_rawset(context, 1); // Set table entry
	
	char const* s = lua_tostring(context, 2);
	if(!s)
		return 0;
	
	// Allow only [A-Za-z0-9\-]
	for(char const* p = s; *p; ++p)
	{
		if(!isalnum(*p) && *p != '-' && *p != '_')
		{
			LUA_ELOG("Persistence name '" << s << "' invalid. Data not stored.");
			return 0;
		}
	}

	try
	{	
		fs::path dumpPath(fs::path("persistance") / (std::string(s) + ".lpr"));
		fs::create_directories( dumpPath.branch_path() );
		fs::ofstream f(dumpPath, std::ios::binary);
		
		if(!f.is_open())
			return 0;
			
		context.serializeT(f, 3);
	}
	catch(std::exception& e)
	{
		cerr << "dump() failed with error: " << e.what() << endl;
		return 0;
	}

	return 0;
}

int l_undump(lua_State* L)
{
	LuaContext context(L);
	
	char const* s = lua_tostring(context, 2);
	if(!s)
		return 0;
	
	// Allow only [A-Za-z0-9\-_]
	for(char const* p = s; *p; ++p)
	{
		if(!isalnum(*p) && *p != '-' && *p != '_')
		{
			LUA_ELOG("Persistence name '" << s << "' invalid. Data not retrieved.");
			return 0;
		}
	}
	
	try
	{
		fs::path dumpPath(fs::path("persistance") / (std::string(s) + ".lpr"));
	
		fs::ifstream f(dumpPath, std::ios::binary);
		
		if(!f.is_open())
			return 0;
			
		//context.deserialize(f);
		
		int r = context.evalExpression("<persistent value>", f);
		if(r != 1)
			return 0;
		context.pushvalue(2); // Key
		context.pushvalue(-2); // Value
		lua_rawset(context, 1); // Set table entry
	}
	catch(std::exception& e)
	{
		cerr << "undump() failed with error: " << e.what() << endl;
		return 0;
	}

	return 1;
}


/*
std::string runLua(LuaReference ref, std::list<std::string> const& args)
{
	AssertStack as(lua);
	
	lua.push(LuaContext::errorReport);
	lua.pushReference(ref);
	if(lua_isnil(lua, -1))
	{
		lua.pop(2);
		return "";
	}
	int params = 0;
	
	for(std::list<std::string>::const_iterator i = args.begin();
		i != args.end();
		++i)
	{
		lua_pushstring(lua, i->c_str());
		++params;
	}

	int r = lua.call(params, 1, -params-2);
	if(r < 0)
	{
		lua_pushnil(lua);
		lua.assignReference(ref);
		lua.pop(1);
		return "";
	}
	lua_remove(lua, -1-1);
	
	if(char const* s = lua_tostring(lua, -1))
	{
		std::string ret(s);
		lua.pop(1);
		return ret;
	}

	lua.pop(1);
	
	return "";
}*/

void serverListCallb(lua_State* L, LuaReference ref, HTTP::Request* req)
{
	static char const* fields[] =
	{
		"ip",
		"title",
		"desc",
		"mod",
		"map"
	};
	
	LuaContext context(L);
	AssertStack as(context);
	
	context.pushReference(ref);
	
	if(!req->success)
	{
		lua_pushnil(context);
		lua.call(1, 0);
		return;
	}
	
	lua_newtable(context);
	
	std::string::size_type p = 0, endp;
	
	for(int i = 1; (endp = req->data.find('|', p)) != std::string::npos; p = endp + 1, ++i)
	{
		std::string::size_type p2 = p, endp2;
		lua_newtable(context);
		
		for(int f = 0; f < 5 && (endp2 = req->data.find('^', p2)) < endp; p2 = endp2 + 1, ++f)
		{
			lua_pushlstring(context, req->data.c_str() + p2, endp2 - p2);
			lua_setfield(context, -2, fields[f]);
		}
		
		lua_rawseti(context, -2, i);
	}
	
	delete req; // We don't need this anymore

	lua.call(1, 0);
}

/*! fetch_server_list(handler)

	Fetches the server list from the master server and calls
	//handler// with one parameter containing an array of
	the servers.
*/
int l_fetch_server_list(lua_State* L)
{
	lua_pushvalue(L, 1);
	LuaReference ref = lua.createReference();
	
	HTTP::Request* req = network.fetchServerList();
	
	if(!req)
		return 0;

	network.addHttpRequest(req,
		boost::bind(serverListCallb, L, ref, _1));

	return 0;
}

void init()
{
	LuaContext& context = lua;
	
	initMath();
#ifndef DEDSERV
	initGUI(OmfgGUI::menu, context);
#endif
	initNetwork(context);
	initObjects();
	initResources();
	initGfx();
	initGame();

	context.functions()
		("print", print)
		("console_register_command", l_console_register_command)
		("console_key_for_action", l_console_key_for_action)
		("console_bind", l_console_bind)
		("console_action_for_key", l_console_action_for_key)
		//("dump", l_dump)
		//("undump", l_undump)
		("fetch_server_list", l_fetch_server_list)
#ifndef DEDSERV
		("clear_keybuf", l_clear_keybuf)
		("key_name", l_key_name)
#endif

		("quit", l_quit)
		("bind", l_bind)
		("connect", l_connect)
		("host", l_host)
		("map", l_map)
	;

	// Bindings table and metatable
	lua_pushstring(context, "bindings");
	lua_newtable(context); // Bindings table
	
	lua_newtable(context); // Bindings metatable
	lua_pushstring(context, "__newindex");
	lua_pushcfunction(context, l_bind);
	lua_rawset(context, -3);
	
	lua_setmetatable(context, -2);

	lua_rawset(context, LUA_GLOBALSINDEX);
	
	// Console table and metatable
	lua_pushstring(context, "console");
	lua_newtable(context); // Console table
	
	lua_newtable(context); // Console metatable
	
	lua_pushstring(context, "__newindex");
	lua_pushcfunction(context, l_console_set);
	lua_rawset(context, -3);
	
	lua_pushstring(context, "__index");
	lua_pushcfunction(context, l_console_get);
	lua_rawset(context, -3);
	
	lua_setmetatable(context, -2);

	lua_rawset(context, LUA_GLOBALSINDEX);

	lua_pushstring(context, "DEDSERV");
#ifdef DEDSERV
	lua_pushboolean(context, 1);
#else
	lua_pushboolean(context, 0);
#endif
	lua_rawset(context, LUA_GLOBALSINDEX);
	
#ifndef DEDSERV
	lua_newtable(context); // Key table
	for(size_t i = 0; i < keyNames.size(); ++i)
	{
		lua_pushinteger(context, i);
		lua_setfield(context, -2, keyNames[i].c_str());
	}
	lua_setfield(context, LUA_GLOBALSINDEX, "Keys");
#endif

	SHADOW_TABLE("persistence", l_undump, l_dump);
}

}

