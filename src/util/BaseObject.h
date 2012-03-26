//
//  BaseObject.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_BaseObject_h
#define OpenLieroX_BaseObject_h

#include <vector>
#include <stdint.h>
#include "CodeAttributes.h"
#include "util/Result.h"
#include "WeakRef.h"
#include "game/ClassInfo.h"
#include "gusanos/luaapi/types.h"

typedef uint16_t ObjId;

struct AttrDesc;
class BaseObject;
class CBytestream;
class CServerConnection;

struct ObjRef {
	ClassId classId;
	ObjId objId;
	WeakRef<BaseObject> obj;

	ObjRef() : classId(-1), objId(-1) {}
	void writeToBs(CBytestream* bs) const;
	void readFromBs(CBytestream* bs);
	std::string description() const;
	operator bool() const {
		return classId != ClassId(-1) && objId != ObjId(-1);
	}
	bool operator==(const ObjRef& o) const {
		return classId == o.classId && objId == o.objId;
	}
	bool operator!=(const ObjRef& o) const { return !(*this == o); }
	bool operator<(const ObjRef& o) const {
		if(classId != o.classId) return classId < o.classId;
		return objId < o.objId;
	}
};

struct AttrUpdateInfo;
class ScriptVar_t;

class BaseObject {
public:
	typedef ::WeakRef<BaseObject> WeakRef;

	BaseObject();
	BaseObject(const BaseObject& o);
	BaseObject& operator=(const BaseObject& o);
	virtual ~BaseObject();

	virtual BaseObject* parentObject() const; // if defined, ownage-properties are overtaken
	virtual bool weOwnThis() const;
	virtual CServerConnection* ownerClient() const;

	virtual std::string toString() const;
	virtual Result getAttrib(const ScriptVar_t& key, ScriptVar_t& value) const;
	virtual Result setAttrib(const ScriptVar_t& key, const ScriptVar_t& value);

	std::vector<AttrUpdateInfo> attrUpdates;
	ObjRef thisRef;
	bool isRegistered() const;

	static LuaReference metaTable;
	static void initMetaTable(LuaContext& ctx);
	virtual LuaReference getMetaTable() const { return metaTable; }

	void pushLuaReference(LuaContext& context);
	LuaReferenceLazy getLuaReference();

	bool deleted;

	virtual void finalize() {}
	virtual void deleteThis();

private:
	LuaReference luaReference;
};

template<class T>
INLINE void luaDelete(T* p)
{
	if(p)
		p->deleteThis();
}

#endif
