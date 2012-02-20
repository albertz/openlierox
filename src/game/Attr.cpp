//
//  Attr.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include <map>
#include <set>
#include <vector>
#include <typeinfo>
#include "Attr.h"
#include "util/macros.h"
#include "Debug.h"
#include "util/StringConv.h"
#include "gusanos/luaapi/classes.h"

std::string AttrDesc::description() const {
	return std::string(LuaClassName(objTypeId)) + ":" + attrName;
}

const AttrDesc* AttribRef::getAttrDesc() const {
	return NULL;
}

ScriptVar_t ObjAttrRef::get() const {
	const BaseObject* oPt = obj.get();
	assert(oPt != NULL); // or should we return the attr default?
	const AttrDesc* attrDesc = attr.getAttrDesc();
	assert(attrDesc != NULL);
	return attrDesc->get(oPt);
}


void registerAttrDesc(AttrDesc& attrDesc) {
	
}


static std::vector< WeakRef<BaseObject> > objUpdates;

void pushObjAttrUpdate(WeakRef<BaseObject> obj) {
	objUpdates.push_back(obj);
}

static void handleAttrUpdateLogging(BaseObject* oPt, const AttrDesc* attrDesc, ScriptVar_t oldValue) {
	// for now, just to console
	if(attrDesc->objTypeId == LuaID<CGameObject>::value && attrDesc->attrId <= 3 /* vPos or vVel */)
		return; // no debug msg, too annoying
	if(attrDesc->objTypeId == LuaID<Game>::value && attrDesc->attrId == 2 /* serverFrame */)
		return; // no debug msg, too annoying
	notes << "<" << typeid(*oPt).name() << " 0x" << hex((uintptr_t)oPt) << "> " << attrDesc->description() << ": update " << oldValue.toString() << " -> " << attrDesc->get(oPt).toString() << endl;
}

void iterAttrUpdates(boost::function<void(BaseObject*, const AttrDesc* attrDesc, ScriptVar_t oldValue)> callback) {
	foreach(o, objUpdates) {
		BaseObject* oPt = o->get();
		if(oPt == NULL) continue;

		foreach(u, oPt->attrUpdates) {
			const AttrDesc* const attrDesc = u->attrDesc;
			ScriptVar_t& oldValue = u->oldValue;

			attrDesc->getAttrExt(oPt).updated = false;
			if(oldValue == attrDesc->get(oPt)) continue;

			if(callback)
				callback(oPt, attrDesc, oldValue);
			if(attrDesc->onUpdate)
				attrDesc->onUpdate(oPt, attrDesc, oldValue);

			handleAttrUpdateLogging(oPt, attrDesc, oldValue);
		}

		oPt->attrUpdates.clear();
	}

	objUpdates.clear();
}

