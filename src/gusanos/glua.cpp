#include "util/BaseObject.h"
#include "CWormHuman.h"
#include "luaapi/context.h"


void BaseObject::pushLuaReference(LuaContext& context)
{
	context.push(getLuaReference().get(context));
}

LuaReferenceLazy BaseObject::getLuaReference()
{
	assert(!deleted);
	LuaReferenceLazy r;
	r.obj = thisRef.obj;
	assert(r.obj);
	r.ref = luaReference;
	return r;
}

LuaReference LuaReferenceLazy::get(LuaContext& ctx) {
	if(!obj) return LuaReference();
	if(ref.isSet(ctx))
		return ref;
	else
	{
		LuaReference metatable = obj.get()->getMetaTable();
		assert(metatable.isSet(ctx));

		ctx.pushFullReference(*obj.get(), metatable);
		ref.create(ctx);

		return ref;
	}
}

void BaseObject::deleteThis()
{
	finalize();

	deleted = true;
	
	luaReference.destroy();
	delete this;
}
