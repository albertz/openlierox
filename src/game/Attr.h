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
#include "util/BaseObject.h"

struct AttrDesc {
	uint32_t objTypeId;
	ScriptVarType_t attrType;
	intptr_t attrMemOffset;
	std::string attrName;
	uint32_t attrId;
	
	bool serverside;
	boost::function<bool(void* base, AttrDesc* attrDesc, ScriptVar_t oldValue)> onUpdate;
	boost::function<void(void* base, AttrDesc* attrDesc)> sync;
	
	AttrDesc()
	: objTypeId(0), attrType(SVT_INVALID), attrMemOffset(0), attrId(0) {}

	uintptr_t getPtr(void* base) { return uintptr_t(base) + attrMemOffset; }
	template<typename T> T& get(void* base) { return *(T*)getPtr(base); }
};

void registerAttrDesc(AttrDesc& attrDesc);

struct AttrUpdateInfo {
	WeakRef<BaseObject> parent;
	AttrDesc* attrDesc;
	ScriptVar_t oldValue;
};

void pushAttrUpdateInfo(const AttrUpdateInfo& info);

template <
typename ParentT, typename T,
typename AttrDescT
>
struct Attr {
	T value;
	Attr() : value() { /* to init attrDesc */ attrDesc(); }
	AttrDescT* attrDesc() {
		static AttrDescT desc;
		return &desc;
	}
	ParentT* parent() { return (ParentT*)(uintptr_t(this) - attrDesc()->attrMemOffset); }
	T get() const { return value; }
	operator T() const { return get(); }
	Attr& operator=(const T& v) {
		AttrUpdateInfo info;
		info.parent = parent()->thisWeakRef;
		info.attrDesc = attrDesc();
		info.oldValue = ScriptVar_t(value);
		pushAttrUpdateInfo(info);
		value = v;
		return *this;
	}
};

#define ATTR(parentType, type, name, id, attrDescSpecCode) \
struct _ ## name ## _AttrDesc : AttrDesc { \
	_ ## name ## _AttrDesc () { \
		objTypeId = LuaID<parentType>::value; \
		attrType = GetType<type>::value; \
		attrMemOffset = (intptr_t)__OLX_OFFSETOF(parentType, name); \
		attrName = #name ; \
		attrId = id; \
		attrDescSpecCode; \
		registerAttrDesc(*this); \
	} \
}; \
Attr<parentType, type, _ ## name ## _AttrDesc> name;

#endif
