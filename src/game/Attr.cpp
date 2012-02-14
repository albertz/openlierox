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
	return std::string(LuaClassName(objTypeId)) + ":" + attrName + "(" + to_string(attrId) + ")";
}

static std::vector< WeakRef<BaseObject> > objUpdates;


void registerAttrDesc(AttrDesc& attrDesc) {
	
}

void pushObjAttrUpdate(WeakRef<BaseObject> obj) {
	objUpdates.push_back(obj);
}

void iterAttrUpdates(boost::function<void(BaseObject*, const AttrDesc* attrDesc, ScriptVar_t oldValue)> callback) {
	foreach(o, objUpdates) {
		BaseObject* oPt = o->get();
		if(oPt == NULL) continue;

		foreach(u, oPt->attrUpdates) {
			u->attrDesc->getAttrExtPtr(oPt)->updated = false;
			if(u->oldValue == u->attrDesc->get(oPt).asScriptVar()) continue;

			if(callback)
				callback(oPt, u->attrDesc, u->oldValue);
			if(u->attrDesc->onUpdate)
				u->attrDesc->onUpdate(oPt, u->attrDesc, u->oldValue);

			notes << "<" << typeid(*oPt).name() << " 0x" << hex((uintptr_t)oPt) << "> " << u->attrDesc->description() << ": update " << u->oldValue.toString() << " -> " << u->attrDesc->get(oPt).toString() << endl;
		}

		oPt->attrUpdates.clear();
	}

	objUpdates.clear();
}

