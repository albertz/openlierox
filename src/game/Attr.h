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
	ScriptVar_t defaultValue;

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
	const AttrDesc* attrDesc;
	ScriptVar_t oldValue;
};

void pushAttrUpdateInfo(const AttrUpdateInfo& info);

template <typename T, typename AttrDescT>
struct Attr {
	typedef T ValueType;
	typedef AttrDescT AttrDescType;
	T value;
	Attr() {
		// also inits attrDesc which is needed here
		value = (T) attrDesc()->defaultValue;
	}
	const AttrDescT* attrDesc() {
		static const AttrDescT desc;
		return &desc;
	}
	BaseObject* parent() { return (BaseObject*)(uintptr_t(this) - attrDesc()->attrMemOffset); }
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

template <typename T, typename AttrDescT>
struct AttrWithBasicOpts : public Attr<T, AttrDescT> {
	void operator++(int) { *this += 1; }
	void operator--(int) { *this -= 1; }
	AttrWithBasicOpts& operator+=(T i) { *this = this->value + i; return *this; }
	AttrWithBasicOpts& operator-=(T i) { *this = this->value - i; return *this; }
	AttrWithBasicOpts& operator=(const T& v) { Attr<T, AttrDescT>::operator=(v); return *this; }
};

template <typename T, typename AttrDescT>
struct AttrWithIntOpts : public Attr<T, AttrDescT> {
	void operator++(int) { *this += 1; }
	void operator--(int) { *this -= 1; }
	AttrWithIntOpts& operator+=(T i) { *this = this->value + i; return *this; }
	AttrWithIntOpts& operator-=(T i) { *this = this->value - i; return *this; }
	AttrWithIntOpts& operator%=(T i) { *this = this->value % i; return *this; }
	AttrWithIntOpts& operator=(const T& v) { Attr<T, AttrDescT>::operator=(v); return *this; }
};

template <typename T, typename AttrDescT>
struct AttrWithMaybeOpts {
	typedef Attr<T, AttrDescT> Type;
};

#define USE_OPTS_FOR(T, k) \
template <typename AttrDescT> \
struct AttrWithMaybeOpts<T,AttrDescT> { \
	typedef AttrWith ## k ## Opts<T,AttrDescT> Type; \
}

USE_OPTS_FOR(int, Int);
USE_OPTS_FOR(float, Basic);

#undef USE_OPTS_FOR


#define ATTR(parentType, type, name, id, attrDescSpecCode) \
struct _ ## name ## _AttrDesc : AttrDesc { \
	_ ## name ## _AttrDesc () { \
		objTypeId = LuaID<parentType>::value; \
		attrType = GetType<type>::value; \
		attrMemOffset = (intptr_t)__OLX_OFFSETOF(parentType, name); \
		attrName = #name ; \
		attrId = id; \
		defaultValue = ScriptVar_t(type()); \
		attrDescSpecCode; \
		registerAttrDesc(*this); \
	} \
}; \
typedef AttrWithMaybeOpts<type, _ ## name ## _AttrDesc>::Type name ## _Type; \
name ## _Type name;

#endif
