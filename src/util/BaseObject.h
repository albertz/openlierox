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

struct AttrDesc;

struct AttrUpdateInfo {
	const AttrDesc* attrDesc;
	ScriptVar_t oldValue;
};

struct BaseObject : DontCopyTag {
	typedef uint16_t ObjId;
	typedef ::WeakRef<BaseObject> WeakRef;

	BaseObject();
	virtual ~BaseObject();
	
	ObjId uniqueObjId;
	std::vector<AttrUpdateInfo> attrUpdates;
	WeakRef thisWeakRef;
};

#endif
