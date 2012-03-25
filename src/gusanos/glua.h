#ifndef VERMES_GLUA_H
#define VERMES_GLUA_H

#include "luaapi/types.h"

class LuaContext;

struct LuaObject
{
	bool deleted;
	LuaReference luaReference;

	LuaObject() : deleted(false) {}
	virtual ~LuaObject() {}

	// copy semantics: each LuaObject instance has its own unique LuaReference. nothing is copied
	LuaObject(const LuaObject&) : deleted(false) {}
	LuaObject& operator=(const LuaObject&) { return *this; }

	void pushLuaReference(LuaContext& context);
	LuaReference getLuaReference();	
	virtual LuaReference getMetaTable() const;

	virtual void finalize() {}
	virtual void deleteThis();	
};

template<class T>
INLINE void luaDelete(T* p)
{
	if(p)
		p->deleteThis();
}

#endif //VERMES_GLUA_H
