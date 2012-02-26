//
//  BaseObject.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include "BaseObject.h"

BaseObject::BaseObject() {
	thisRef.obj.set(this);
}

BaseObject::~BaseObject() {
	thisRef.obj.overwriteShared(NULL);
}
