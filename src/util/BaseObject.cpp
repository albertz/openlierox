//
//  BaseObject.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include "BaseObject.h"
#include "CBytestream.h"

void ObjRef::writeToBs(CBytestream* bs) const {
	bs->writeInt16(classId);
	bs->writeInt16(objId);
}

void ObjRef::readFromBs(CBytestream* bs) {
	classId = bs->readInt16();
	objId = bs->readInt16();
}

std::string ObjRef::description() const {
	const ClassInfo* classInfo = getClassInfo(classId);
	std::string r;
	if(classInfo) r += classInfo->name;
	else r += "<unknown class " + to_string(classId) + ">";
	r += ":";
	r += to_string(objId);
	return r;
}

BaseObject::BaseObject() {
	thisRef.obj.set(this);
}

BaseObject::~BaseObject() {
	thisRef.obj.overwriteShared(NULL);
}
