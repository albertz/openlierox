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
#include "CServerConnection.h"

void CustomVar::reset() {
	assert( thisRef.classId != ClassId(-1) );

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);
	foreach(a, attribs) {
		(*a)->set(this, (*a)->defaultValue);
	}
}

CustomVar* CustomVar::copy() const {
	// this copy() relies on the ClassInfo. otherwise it cannot work. provide your own in this case, it's virtual
	assert( thisRef.classId != ClassId(-1) );
	const ClassInfo* classInfo = getClassInfo(thisRef.classId);
	assert( classInfo != NULL );
	BaseObject* obj = classInfo->createInstance();
	assert( obj != NULL );
	CustomVar* v = dynamic_cast<CustomVar*>(obj);
	assert( v != NULL );
	v->copyFrom(*this);
	return v;
}

bool CustomVar::operator==(const CustomVar& v) const {
	if( thisRef.classId != v.thisRef.classId ) return false;
	assert( thisRef.classId != ClassId(-1) );

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);
	foreach(a, attribs) {
		ScriptVar_t value = (*a)->get(this);
		if(value != (*a)->get(&v)) return false;
	}

	return true;
}

bool CustomVar::operator<(const CustomVar& v) const {
	if( thisRef.classId != v.thisRef.classId )
		return thisRef.classId < v.thisRef.classId;
	assert( thisRef.classId != ClassId(-1) );

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);
	foreach(a, attribs) {
		ScriptVar_t value = (*a)->get(this);
		if(value != (*a)->get(&v))
			return value < (*a)->get(&v);
	}

	return true;
}

bool CustomVar::fromString(const std::string& str) {
	return false;
}

void CustomVar::copyFrom(const CustomVar& v) {
	assert( thisRef.classId != ClassId(-1) );
	assert( v.thisRef.classId != ClassId(-1) );
	assert( thisRef.classId == v.thisRef.classId );

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);
	foreach(a, attribs) {
		ScriptVar_t value = (*a)->get(&v);
		if(value == (*a)->get(this)) continue;
		(*a)->set(this, value);
	}
}

void CustomVar::fromScriptVar(const ScriptVar_t& v) {
	if(v.isCustomType()
			&& v.customVar()->thisRef.classId != ClassId(-1)
			&& v.customVar()->thisRef.classId == thisRef.classId
	)
		copyFrom(*v.customVar());
	else
		fromString(v.toString());
}

Result CustomVar::ToBytestream(CBytestream* bs, const CustomVar* diffTo) const {
	assert( thisRef.classId != ClassId(-1) );
	bs->writeInt16(thisRef.classId);
	return toBytestream(bs, diffTo);
}

Result CustomVar::toBytestream(CBytestream *bs, const CustomVar* diffTo) const {
	assert( thisRef.classId != ClassId(-1) );
	if(diffTo)
		assert( thisRef.classId == diffTo->thisRef.classId );

	std::vector<const AttrDesc*> attribs = getAttrDescs(thisRef.classId, true);

	size_t numChangesOld = 0;
	size_t numChangesDefault = 0;
	if(diffTo) {
		foreach(a, attribs) {
			ScriptVar_t value = (*a)->get(this);
			if(value != (*a)->defaultValue) numChangesDefault++;
			if(diffTo && value != (*a)->get(diffTo)) numChangesOld++;
		}
	}
	bool shouldUpdateAll = true;
	foreach(a, attribs) {
		if(isRegistered() && !(*a)->shouldUpdate(this)) {
			shouldUpdateAll = false;
			break;
		}
	}
	if(!shouldUpdateAll && !diffTo)
		errors << "CustomVar::toBytestream " << thisRef.description() << ": at least one attrib should not be updated but we have no diff to relate to" << endl;

	if(diffTo && (!shouldUpdateAll || (numChangesOld < numChangesDefault))) {
		bs->writeInt(CUSTOMVAR_STREAM_DiffToOld, 1);
		foreach(a, attribs) {
			ScriptVar_t value = (*a)->get(this);
			if(value == (*a)->get(diffTo)) continue;
			if(isRegistered() && !(*a)->shouldUpdate(this)) continue;
			bs->writeInt16((*a)->objTypeId);
			bs->writeInt16((*a)->attrId);
			bs->writeVar(value);
		}
		bs->writeInt16(ClassId(-1));
	}
	else {
		bs->writeInt(CUSTOMVAR_STREAM_DiffToDefault, 1);
		foreach(a, attribs) {
			ScriptVar_t value = (*a)->get(this);
			if(value == (*a)->defaultValue) continue;
			if(isRegistered() && !(*a)->shouldUpdate(this)) continue;
			bs->writeInt16((*a)->objTypeId);
			bs->writeInt16((*a)->attrId);
			bs->writeVar(value);
		}
		bs->writeInt16(ClassId(-1));
	}

	return true;
}

Result CustomVar::fromBytestream(CBytestream *bs, bool expectDiffToDefault) {
	assert( thisRef.classId != ClassId(-1) );

	{
		int streamType = bs->readInt(1);
		switch(streamType) {
		case CUSTOMVAR_STREAM_DiffToDefault: reset(); break;
		case CUSTOMVAR_STREAM_DiffToOld:
			if(expectDiffToDefault) {
				errors << "CustomVar::fromBytestream: got unexpected diffToOld data on " << thisRef.description() << endl;
				// continue anyway, there might be still something useful
			}
			/* nothing to do */ break;
		default:
			errors << "CustomVar::fromBytestream: got invalid streamType " << streamType << " for object " << thisRef.description() << endl;
			// It doesn't really make sense to continue.
			// This cannot be a bug, the stream is messed up.
			return "stream messed up. invalid streamtype";
		}
	}

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

		/*if(a.objTypeId == LuaID<wpnslot_t>::value) {
			notes << "CustomVar::fromBytestream: " << a.description() << ": -> " << var.toString() << ", " << attrDesc->get(this) << ", " << attrDesc->authorizedToWrite(this) << ", " << (AttrUpdateByClientScope::currentScope() ? AttrUpdateByClientScope::currentScope()->debugName() : "NULL") << ", " << (this->ownerClient() ? this->ownerClient()->debugName() : "NULL") << ", " << (this->parentObject() ? this->parentObject()->thisRef.description() : "NULL") << endl;
		}*/
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

	CustomVar::Ref obj = dynamic_cast<CustomVar*>(classInfo->createInstance());
	if(!obj) {
		errors << "CustomVar::FromBytestream: couldn't create instance of " << classInfo->name << endl;
		return NULL;
	}

	if(NegResult r = obj->fromBytestream(bs, true)) {
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
