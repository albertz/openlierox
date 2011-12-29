#ifndef VERMES_GLUA_H
#define VERMES_GLUA_H

#include <string>
#include <vector>
#include "luaapi/types.h"
#include "CodeAttributes.h"

#define C_LocalPlayer_ActionCount 8

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
		networkStateChange = localplayerEvent+C_LocalPlayer_ActionCount,
		gameError,
		max
	};
	void bind(std::string callback, LuaReference ref);
	std::vector<LuaReference> callbacks[max];
};

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
	
	virtual void deleteThis();
	
	virtual ~LuaObject()
	{
	}
	
	LuaReference luaReference;
#ifndef NDEBUG
	bool deleted;
#endif
};

template<class T>
INLINE void luaDelete(T* p)
{
	if(p)
		p->deleteThis();
}

#endif //VERMES_GLUA_H
