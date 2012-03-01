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
#include "util/macros.h"

Result CustomVar::toBytestream(CBytestream *bs) {
	assert( thisRef.classId != ClassId(-1) );
	bs->writeInt16(thisRef.classId);

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);
	foreach(a, attribs) {
		ScriptVar_t value = (*a)->get(this);
		if(value == (*a)->defaultValue) continue;
		bs->writeInt16((*a)->objTypeId);
		bs->writeInt16((*a)->attrId);
		bs->writeVar(value);
	}
	bs->writeInt16(ClassId(-1));

	return true;
}

Result CustomVar::fromBytestream(CBytestream *bs) {
	assert( thisRef.classId != ClassId(-1) );

	Result r = true;
	while(true) {
		AttribRef a;
		a.objTypeId = bs->readInt16();
		if(a.objTypeId == ClassId(-1)) break;
		a.attrId = bs->readInt16();
		const AttrDesc* attrDesc = a.getAttrDesc();
		if(attrDesc == NULL) {
			errors << "CustomVar::fromBytestream: unknown attrib " << a.objTypeId << ":" << a.attrId << " for object " << thisRef.description() << endl;
			// try to continue
			r = "unknown attrib";
			bs->SkipVar();
			continue;
		}

		const ClassInfo* classInfo = getClassInfo(a.objTypeId);
		if(classInfo == NULL) {
			errors << "CustomVar::fromBytestream: unknown class for attrib " << a.description() << " for object " << thisRef.description() << endl;
			// try to continue
			r = "unknown class";
			bs->SkipVar();
			continue;
		}

		if(!classInfo->isTypeOf(a.objTypeId)) {
			errors << "CustomVar::fromBytestream: attrib " << a.description() << " does not belong to class " << classInfo->name << " for object " << thisRef.description() << endl;
			// try to continue
			r = "invalid attrib for class";
			bs->SkipVar();
			continue;
		}

		ScriptVar_t var;
		bs->readVar(var);
		attrDesc->set(this, var);
	}

	return r;
}

CustomVar::Ref CustomVar::FromBytestream( CBytestream* bs ) {
	ClassId classId = bs->readInt16();
	const ClassInfo* classInfo = getClassInfo(classId);
	if(classInfo == NULL) {
		errors << "CustomVar::FromBytestream: class ID " << classId << " unknown" << endl;
		return NULL;
	}

	if(!classInfo->isTypeOf(LuaID<CustomVar>::value)) {
		errors << "CustomVar::FromBytestream: class " << classInfo->name << " is not a CustomVar" << endl;
		return NULL;
	}

	if(classInfo->id == LuaID<CustomVar>::value) {
		errors << "CustomVar::FromBytestream: class " << classInfo->name << " is abstract" << endl;
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

// We cannot use the REGISTER_CLASS macro because we cannot instantiate it.
static bool registerClass_CustomVar() {
	ClassInfo i;
	i.id = LuaID<CustomVar>::value;
	i.name = "CustomVar";
	i.memSize = sizeof(CustomVar);
	registerClass(i);
	return true;
}
static bool registerClass_CustomVar_init = registerClass_CustomVar();
