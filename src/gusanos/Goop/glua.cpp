#include "glua.h"
#include "player.h"
#include "luaapi/context.h"

//LuaContext lua;

LuaCallbacks luaCallbacks;

void LuaCallbacks::bind(std::string callback, LuaReference ref)
{
	int idx = -1;
	
	#define CB(x_) else if(callback == #x_) idx = x_
	if(callback == "afterRender")
		idx = afterRender;
	else if(callback == "afterUpdate")
		idx = afterUpdate;
	else if(callback == "wormRender")
		idx = wormRender;
	else if(callback == "viewportRender")
		idx = viewportRender;
	else if(callback == "localplayerLeft")
		idx = localplayerEvent + Player::LEFT;
	else if(callback == "localplayerRight")
		idx = localplayerEvent + Player::RIGHT;
	else if(callback == "localplayerUp")
		idx = localplayerEvent + Player::UP;
	else if(callback == "localplayerDown")
		idx = localplayerEvent + Player::DOWN;
	else if(callback == "localplayerJump")
		idx = localplayerEvent + Player::JUMP;
	else if(callback == "localplayerFire")
		idx = localplayerEvent + Player::FIRE;
	else if(callback == "localplayerChange")
		idx = localplayerEvent + Player::CHANGE;
	else if(callback == "localplayerInit")
		idx = localplayerInit;
	else if(callback == "wormDeath")
		idx = wormDeath;
	else if(callback == "playerUpdate")
		idx = playerUpdate;
	else if(callback == "playerInit")
		idx = playerInit;
	else if(callback == "localplayerEvent")
		idx = localplayerEventAny;
	CB(wormRemoved);
	CB(playerNetworkInit);
	CB(playerRemoved);
	CB(gameNetworkInit);
	CB(gameEnded);
	CB(transferUpdate);
	CB(transferFinished);
	CB(networkStateChange);
	CB(gameError);
	
	if(idx != -1)
	{
		callbacks[idx].push_back(ref);
	}
}

void LuaObject::pushLuaReference()
{
	lua.push(getLuaReference());
}

void LuaObject::makeReference()
{
	lua_pushnil(lua);
}

LuaReference LuaObject::getLuaReference()
{
	assert(!deleted);
	if(luaReference)
		return luaReference;
	else
	{
		makeReference();
		luaReference = lua.createReference();
		return luaReference;
	}
}

void LuaObject::deleteThis()
{
	finalize();

#ifndef NDEBUG
	deleted = true;
#endif
	
	if(luaReference)
	{
		lua.destroyReference(luaReference);
		luaReference.reset();
	}
	else
		delete this;
	
	
}
