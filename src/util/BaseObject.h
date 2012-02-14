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
#include "CodeAttributes.h"
#include "WeakRef.h"
#include "CScriptableVars.h"

struct AttrDesc;

struct AttrUpdateInfo {
	const AttrDesc* attrDesc;
	ScriptVar_t oldValue;
};

struct BaseObject : DontCopyTag {
	BaseObject();
	virtual ~BaseObject();
	
	std::vector<AttrUpdateInfo> attrUpdates;
	WeakRef<BaseObject> thisWeakRef;
};

#endif
