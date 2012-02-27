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

class CBytestream;

struct AttrExt {
	bool updated;
	AttrExt() : updated(false) {}
};

struct AttrDesc {
	typedef uint16_t AttrId;

	ClassId objTypeId;
	ScriptVarType_t attrType;
	bool isStatic; // if true -> use memOffsets; otherwise, dyn funcs
	intptr_t attrMemOffset;
	intptr_t attrExtMemOffset;
	boost::function<ScriptVar_t (const BaseObject* base, const AttrDesc* attrDesc)> dynGetValue;
	boost::function<AttrExt& (BaseObject* base, const AttrDesc* attrDesc)> dynGetAttrExt;
	std::string attrName;
	AttrId attrId;
	ScriptVar_t defaultValue;

	bool serverside;
	boost::function<void(BaseObject* base, const AttrDesc* attrDesc, ScriptVar_t oldValue)> onUpdate;
	boost::function<void(BaseObject* base, const AttrDesc* attrDesc)> sync;
	
	AttrDesc()
	: objTypeId(0), attrType(SVT_INVALID), isStatic(true), attrMemOffset(0), attrExtMemOffset(0), attrId(0),
	  serverside(true) {}

	const void* getValuePtr(const BaseObject* base) const {
		assert(isStatic);
		return (void*)(uintptr_t(base) + attrMemOffset);
	}
	ScriptVarPtr_t getValueScriptPtr(BaseObject* base) const {
		return ScriptVarPtr_t(attrType, (void*)getValuePtr(base), defaultValue);
	}
	AttrExt& getAttrExt(BaseObject* base) const {
		if(isStatic)
			return *(AttrExt*)(uintptr_t(base) + attrMemOffset + attrExtMemOffset);
		else
			return dynGetAttrExt(base, this);
	}
	ScriptVar_t get(const BaseObject* base) const {
		if(isStatic)
			// the const-cast is safe because ScriptVarPtr_t.asScriptVar doesn't modify it
			return getValueScriptPtr((BaseObject*)base).asScriptVar();
		else
			return dynGetValue(base, this);
	}

	std::string description() const;
};

struct AttribRef {
	ClassId objTypeId;
	AttrDesc::AttrId attrId;

	AttribRef() : objTypeId(-1), attrId(-1) {}
	AttribRef(const AttrDesc* attrDesc);
	void writeToBs(CBytestream* bs) const;
	void readFromBs(CBytestream* bs);
	std::string description() const;
	const AttrDesc* getAttrDesc() const;

	bool operator==(const AttribRef& o) const {
		return objTypeId == o.objTypeId && attrId == o.attrId;
	}
	bool operator<(const AttribRef& o) const {
		if(objTypeId != o.objTypeId) return objTypeId < o.objTypeId;
		return attrId < o.attrId;
	}
};

struct ObjAttrRef {
	ObjRef obj;
	AttribRef attr;

	ObjAttrRef() {}
	ObjAttrRef(ObjRef o, const AttrDesc* attrDesc);
	void writeToBs(CBytestream* bs) const;
	void readFromBs(CBytestream* bs);
	std::string description() const;
	static ObjAttrRef LowerLimit(ObjRef o);
	static ObjAttrRef UpperLimit(ObjRef o);
	ScriptVar_t get() const;

	bool operator==(const ObjAttrRef& o) const {
		return obj == o.obj && attr == o.attr;
	}
	bool operator<(const ObjAttrRef& o) const {
		if(obj != o.obj) return obj < o.obj;
		return attr < o.attr;
	}
};

void registerAttrDesc(AttrDesc& attrDesc);
void pushObjAttrUpdate(BaseObject& obj);
void iterAttrUpdates(boost::function<void(BaseObject*, const AttrDesc* attrDesc, ScriptVar_t oldValue)> callback);


template <typename T, typename AttrDescT>
struct Attr {
	typedef T ValueType;
	typedef AttrDescT AttrDescType;
	T value;
	AttrExt ext;
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
	T& write() {
		if(parent()->attrUpdates.empty())
			pushObjAttrUpdate(*parent());
		if(!ext.updated || parent()->attrUpdates.empty()) {
			assert(&value == attrDesc()->getValuePtr(parent()));
			assert(&ext == &attrDesc()->getAttrExt(parent()));
			AttrUpdateInfo info;
			info.attrDesc = attrDesc();
			info.oldValue = ScriptVar_t(value);
			parent()->attrUpdates.push_back(info);
			ext.updated = true;
		}
		return value;
	}
	Attr& operator=(const T& v) {
		write() = v;
		return *this;
	}
};

template <typename T, typename AttrDescT>
struct AttrWithBasicOpts : public Attr<T, AttrDescT> {
	void operator++(int) { *this += 1; }
	void operator--(int) { *this -= 1; }
	template<typename T2> AttrWithBasicOpts& operator+=(T2 i) { *this = this->value + i; return *this; }
	template<typename T2> AttrWithBasicOpts& operator-=(T2 i) { *this = this->value - i; return *this; }
	template<typename T2> AttrWithBasicOpts& operator*=(T2 i) { *this = this->value * i; return *this; }
	template<typename T2> AttrWithBasicOpts& operator/=(T2 i) { *this = this->value / i; return *this; }
	template<typename T2> AttrWithBasicOpts& operator=(const T2& v) { Attr<T, AttrDescT>::operator=(v); return *this; }
	template<typename T2> T operator+(T2 o) const { return this->value + o; }
	template<typename T2> T operator-(T2 o) const { return this->value - o; }
	template<typename T2> T operator*(T2 o) const { return this->value * o; }
	template<typename T2> T operator/(T2 o) const { return this->value / o; }
	bool operator==(T o) const { return this->value == o; }
	bool operator!=(T o) const { return this->value != o; }
};

template <typename T, typename AttrDescT>
struct AttrWithIntOpts : public Attr<T, AttrDescT> {
	void operator++(int) { *this += 1; }
	void operator--(int) { *this -= 1; }
	template<typename T2> AttrWithIntOpts& operator+=(T2 i) { *this = this->value + i; return *this; }
	template<typename T2> AttrWithIntOpts& operator-=(T2 i) { *this = this->value - i; return *this; }
	template<typename T2> AttrWithIntOpts& operator*=(T2 i) { *this = this->value * i; return *this; }
	template<typename T2> AttrWithIntOpts& operator/=(T2 i) { *this = this->value / i; return *this; }
	template<typename T2> AttrWithIntOpts& operator%=(T2 i) { *this = this->value % i; return *this; }
	template<typename T2> AttrWithIntOpts& operator=(const T2& v) { Attr<T, AttrDescT>::operator=(v); return *this; }
	template<typename T2> T operator+(T2 o) const { return this->value + o; }
	template<typename T2> T operator-(T2 o) const { return this->value - o; }
	template<typename T2> T operator*(T2 o) const { return this->value * o; }
	template<typename T2> T operator/(T2 o) const { return this->value / o; }
	bool operator==(T o) const { return this->value == o; }
	bool operator!=(T o) const { return this->value != o; }
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

USE_OPTS_FOR(int32_t, Int);
USE_OPTS_FOR(uint64_t, Int);
USE_OPTS_FOR(float, Basic);
USE_OPTS_FOR(CVec, Basic);

#undef USE_OPTS_FOR


#define ATTR(parentType, type, name, id, attrDescSpecCode) \
struct _ ## parentType ## _AttrDesc ## id : AttrDesc { \
	_ ## parentType ## _AttrDesc ## id () { \
		objTypeId = LuaID<parentType>::value; \
		attrType = GetType<type>::value; \
		attrMemOffset = (intptr_t)__OLX_OFFSETOF(parentType, name); \
		struct Dummy { type x; AttrExt ext; }; \
		attrExtMemOffset = (intptr_t)__OLX_OFFSETOF(Dummy, ext); \
		attrName = #name ; \
		attrId = id; \
		defaultValue = ScriptVar_t(type()); \
		attrDescSpecCode; \
		registerAttrDesc(*this); \
	} \
}; \
typedef AttrWithMaybeOpts<type, _ ## parentType ## _AttrDesc ## id>::Type name ## _Type; \
name ## _Type name;

#endif
