#ifndef GUSANOS_GLUA_H
#define GUSANOS_GLUA_H

#include <string>
#include <vector>
//#include "luaapi/context.h"
#include "luaapi/types.h"

#define EACH_CALLBACK(i_, type_) for(std::vector<LuaReference>::iterator i_ = luaCallbacks.callbacks[LuaCallbacks::type_].begin(); \
			i_ != luaCallbacks.callbacks[LuaCallbacks::type_].end(); ++i_)

struct LuaCallbacks
{
	enum
	{
		atGameStart = 0,
		afterRender = 1,
		afterUpdate = 2,
		wormRender = 3,
		viewportRender = 4,
		wormDeath = 5,
		wormRemoved = 6,
		playerUpdate = 7,
		playerInit = 8,
		playerRemoved = 9,
		playerNetworkInit = 10,
		gameNetworkInit = 11,
		gameEnded = 12,
		localplayerEventAny = 13,
		localplayerInit = 14,
		localplayerEvent = 15,
		transferUpdate = localplayerEvent+7,
		transferFinished = transferUpdate+1,
		networkStateChange = transferFinished+1,
		gameError = networkStateChange+1,
		max
	};
	void bind(std::string callback, LuaReference ref);
	/*
	std::vector<LuaReference> atGameStart;
	std::vector<LuaReference> afterRender;
	std::vector<LuaReference> afterUpdate;
	std::vector<LuaReference> wormRender;
	std::vector<LuaReference> viewportRender;
	std::vector<LuaReference> wormDeath;
	std::vector<LuaReference> wormRemoved;
	std::vector<LuaReference> playerUpdate;
	std::vector<LuaReference> playerInit;
	std::vector<LuaReference> playerRemoved;
	std::vector<LuaReference> playerNetworkInit;
	std::vector<LuaReference> gameNetworkInit;
	std::vector<LuaReference> gameEnded;
	//TODO: std::vector<LuaReference> connectionRequest;
	
	std::vector<LuaReference> localplayerEvent[7];
	std::vector<LuaReference> localplayerEventAny;
	std::vector<LuaReference> localplayerInit;
	*/
	std::vector<LuaReference> callbacks[max];
};

//extern LuaContext lua;

extern LuaCallbacks luaCallbacks;
/*
// This is GCC specific, because I can't find a way to do it in standard C++ :/
#define LUA_NEW(t_, param_) \
({ \
	void* space = lua.pushObject(t_::metaTable(), sizeof(t_)); \
	t_* p = new (space) t_ param_; \
	p->luaReference = lua.createReference(); \
	p; \
})


#define LUA_DELETE(t_, p_) { \
	t_* p = (p_); \
	p->~t_(); \
	lua.destroyReference(p->luaReference); \
}*/

struct LuaObject 
{
#ifndef NDEBUG
	LuaObject()
	: deleted(false)
	{
	}
#endif
	
	void pushLuaReference();
	
	LuaReference getLuaReference();
	
	virtual void makeReference();
	
	virtual void finalize()
	{
	}
	
	void deleteThis();
	
	virtual ~LuaObject()
	{
	}
	
	LuaReference luaReference;
#ifndef NDEBUG
	bool deleted;
#endif
};

template<class T>
inline void luaDelete(T* p)
{
	if(p)
		p->deleteThis();
}

#endif //GUSANOS_GLUA_H
