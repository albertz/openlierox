// Lua callbacks
// code under LGPL
// created 2012-03-25

#include "LuaCallbacks.h"
#include "CWormHuman.h"

void LuaCallbackProxy::_pushFunc(LuaCallbackRef& c) {
	c.context.push(LuaContext::errorReport);
	c.context.push(c.ref);
}

int LuaCallbackProxy::_exec(LuaCallbackRef& c, int nparams) {
	if(lua_isnil(c.context, -nparams-1))
	{
		c.context.pop(nparams + 2);
		return 0;
	}

	int r = c.context.call(nparams, nreturns, -nparams-2);

	if(r < 0)
	{
		c.context.pop(1); // Pop error function
		c.ref.invalidate();
		return 0;
	}

	lua_remove(c.context, -nreturns-1); // Pop error function
	return nreturns;
}

LuaCallbacks luaCallbacks;

void LuaCallbacks::bind(const LuaContext& ctx, std::string callback, LuaReference ref)
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
		idx = localplayerEvent + CWormHumanInputHandler::LEFT;
	else if(callback == "localplayerRight")
		idx = localplayerEvent + CWormHumanInputHandler::RIGHT;
	else if(callback == "localplayerUp")
		idx = localplayerEvent + CWormHumanInputHandler::UP;
	else if(callback == "localplayerDown")
		idx = localplayerEvent + CWormHumanInputHandler::DOWN;
	else if(callback == "localplayerJump")
		idx = localplayerEvent + CWormHumanInputHandler::JUMP;
	else if(callback == "localplayerFire")
		idx = localplayerEvent + CWormHumanInputHandler::FIRE;
	else if(callback == "localplayerChange")
		idx = localplayerEvent + CWormHumanInputHandler::CHANGE;
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
	CB(networkStateChange);
	CB(gameError);
	CB(exit);
	CB(serverStart);
	CB(serverStop);
	CB(serverJoined);
	CB(serverLeft);
	CB(gamePrepare);
	CB(gameBegin);
	CB(gameOver);
	CB(gotoLobby);
	CB(wormPrepare);

	if(idx != -1)
		callbacks[idx].push_back(LuaCallbackRef(ctx, ref));
	else
		warnings << "LuaCallbacks::bind: '" << callback << "' unknown" << endl;
}

void LuaCallbacks::cleanup() {
	for(int e = 0; e < LuaCallbacks::max; ++e)
		for(LuaCallbackList::iterator i = callbacks[e].begin(); i != callbacks[e].end();) {
			if(!*i)
				i = callbacks[e].erase(i);
			else
				++i;
		}
}
