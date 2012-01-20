//
//  BaseObject.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include "BaseObject.h"

BaseObject::BaseObject() {
	thisWeakRef.set(this);
}

BaseObject::~BaseObject() {
	thisWeakRef.overwriteShared(NULL);
}
