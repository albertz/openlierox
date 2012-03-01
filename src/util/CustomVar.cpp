/*
 *  CustomVar.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#include "CustomVar.h"
#include "CBytestream.h"
#include "Debug.h"
#include "game/Attr.h"

Result CustomVar::toBytestream(CBytestream *bs) {
	assert( thisRef.classId != ClassId(-1) );
	bs->writeInt16(thisRef.classId);

	return true;
}

Result CustomVar::fromBytestream(CBytestream *bs) {

	return true;
}

CustomVar::Ref CustomVar::FromBytestream( CBytestream* bs ) {
	ClassId classId = bs->readInt16();
	const ClassInfo* classInfo = getClassInfo(classId);
	if(classInfo == NULL) {
		errors << "CustomVar::FromBytestream: class ID " << classId << " unknown" << endl;
		return NULL;
	}

	CustomVar::Ref obj = (CustomVar*) classInfo->createInstance();
	if(!obj) {
		errors << "CustomVar::FromBytestream: couldn't create instance of " << classInfo->name << endl;
		return NULL;
	}

	if(NegResult r = obj->fromBytestream(bs)) {
		errors << "CustomVar::FromBytestream: error while reading " << classInfo->name << " instance: " << r.res.humanErrorMsg << endl;
		// continue anyway, maybe we have some useful data
	}

	return obj;
}
