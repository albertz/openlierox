#include "glua.h"
#include "CWormHuman.h"
#include "luaapi/context.h"


void LuaObject::pushLuaReference(LuaContext& context)
{
	context.push(getLuaReference());
}

// this function is overriden by each sub-object-type (e.g. CWorm, etc.)
LuaReference LuaObject::getMetaTable() const
{
	return LuaReference();
}

LuaReference LuaObject::getLuaReference()
{
	assert(!deleted);
	if(luaReference)
		return luaReference;
	else
	{
		LuaReference metatable = getMetaTable();
		if(!metatable) return LuaReference();

		lua.pushFullReference(*this, metatable);
		luaReference = lua.createReference();

		return luaReference;
	}
}

void LuaObject::deleteThis()
{
	finalize();

	deleted = true;
	
	if(luaReference)
	{
		lua.destroyReference(luaReference);
		luaReference.reset();
	}
	else
		delete this;
}
