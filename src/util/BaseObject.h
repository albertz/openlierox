//
//  BaseObject.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_BaseObject_h
#define OpenLieroX_BaseObject_h

#include "CodeAttributes.h"
#include "WeakRef.h"

struct BaseObject : DontCopyTag {
	BaseObject();
	virtual ~BaseObject();
	
	WeakRef<BaseObject> thisWeakRef;	
};

#endif
