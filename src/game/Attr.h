//
//  Attr.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 17.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_Attr_h
#define OpenLieroX_Attr_h

#include <stdint.h>
#include <string>
#include <boost/function.hpp>
#include "util/WeakRef.h"
#include "CScriptableVars.h"
#include "gusanos/luaapi/classes.h"
#include "util/macros.h"

struct AttrDesc {
	uint32_t objTypeId;
	ScriptVarType_t attrType;
	intptr_t attrMemOffset;
	std::string attrName;
	uint32_t attrId;
	
	bool serverside;
	boost::function<void(void* base, AttrDesc* attrDesc, ScriptVar_t oldValue)> onUpdate;
	boost::function<void(void* base, AttrDesc* attrDesc)> sync;
	
	AttrDesc()
	: objTypeId(0), attrType(SVT_INVALID), attrMemOffset(0), attrId(0) {}

	uintptr_t getPtr(void* base) { return uintptr_t(base) + attrMemOffset; }
	template<typename T> T& get(void* base) { return *(T*)getPtr(base); }
};

struct AttrUpdateInfo {
	WeakRef<CGameObject> parent;
	AttrDesc* attrDesc;
};

void pushAttrUpdateInfo(const AttrUpdateInfo& info);

template <
typename ParentT, typename T,
intptr_t (*memOffsetF)(),
const char* (*nameF)(),
uint32_t attrId
>
struct Attr {
	T value;
	Attr() : value() {}
	AttrDesc* attrDesc() {
		static AttrDesc desc;
		if(desc.attrId == 0) {
			desc.objTypeId = LuaID<ParentT>::value;
			desc.attrType = GetType<T>::value;
			desc.attrMemOffset = memOffsetF();
			desc.attrName = nameF();
			desc.attrId = attrId;
		}
		return &desc;
	}
	ParentT* parent() { return (ParentT*)(uintptr_t(this) - memOffsetF()); }
	operator T() { return value; }
	Attr& operator=(const T& v) {
		AttrUpdateInfo info;
		info.parent = parent()->thisWeakRef;
		info.attrDesc = attrDesc();
		pushAttrUpdateInfo(info);
	}
};

#define ATTR(parentType, type, name, attrId) \
static const char* _ ## name ## _getAttrName() { return #name; } \
static const char* _ ## name ## _getAttrMemOffset() { return __OLX_OFFSETOF(parentType, name); } \
Attr<parentType, type, \
&_ ## name ## _getAttrMemOffset, \
&_ ## name ## _getAttrName, \
attrId> name;

#endif
