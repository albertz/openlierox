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
#include "WeakRef.h"
#include "CScriptableVars.h"
#include "game/ClassInfo.h"

typedef uint16_t ObjId;

struct AttrDesc;
struct BaseObject;
class CBytestream;

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

struct AttrUpdateInfo {
	const AttrDesc* attrDesc;
	ScriptVar_t oldValue;
};

struct BaseObject : DontCopyTag {
	typedef ::WeakRef<BaseObject> WeakRef;

	BaseObject();
	virtual ~BaseObject();
	
	std::vector<AttrUpdateInfo> attrUpdates;
	ObjRef thisRef;
};

#endif
