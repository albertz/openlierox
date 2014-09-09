//
//  BaseObject.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include "BaseObject.h"
#include "CBytestream.h"
#include "game/Attr.h"
#include "game/Game.h"
#include "game/CWorm.h"
#include "CServer.h"

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

BaseObject::BaseObject() : deleted(false) {
	thisRef.obj.set(this);
}

BaseObject::BaseObject(const BaseObject& o) : deleted(false) {
	thisRef.obj.set(this);
	(*this) = o;
}

BaseObject& BaseObject::operator=(const BaseObject& o) {
	if(thisRef.classId == ClassId(-1))
		// Note: This classId assignment is a bit risky
		// because we cannot assure at this point that it is
		// correct because op= might be called at an early
		// point.
		thisRef.classId = o.thisRef.classId;
	else
		assert(thisRef.classId == o.thisRef.classId);
	if(o.thisRef.objId != ObjId(-1))
		// The reason this is an error is because we cannot really know if
		// the copy is really a takeover (in that case, we want to copy the
		// objId/WeakRef and invalidate it on the source) or a real copy
		// (in that case, we have a new objId/WeakRef).
		// If there is ever a need for a sane way, there should be two
		// distinct functions for both cases.
		// Right now, we only want to allow copying objects which are not
		// registered by an objId. E.g. CustomVar objects.
		errors << "BaseObject copy: the source has a specific objId" << endl;
	return *this;
}

BaseObject::~BaseObject() {
	thisRef.obj.overwriteShared(NULL);
}

BaseObject* BaseObject::parentObject() const {
	return NULL;
}

bool BaseObject::weOwnThis() const {
	if(parentObject()) return parentObject()->weOwnThis();
	if(thisRef.objId == ObjId(-1)) return true; // not registered objects without parent are always owned
	if(game.isServer()) {
		CServerConnection* cl = ownerClient();
		if(cl) return cl == cServer->localClientConnection();
	}
	return game.isServer();
}

CServerConnection* BaseObject::ownerClient() const {
	if(parentObject()) return parentObject()->ownerClient();
	if(game.state <= Game::S_Inactive) return NULL;
	if(game.isClient()) return NULL;
	assert(cServer != NULL);
	assert(cServer->isServerRunning());
	return cServer->localClientConnection();
}

// Note: Currently, we do the registering (objId assignment) manually.
// (Atm only fixed (game|settings).objId == 1 and worm.objId == wormId.)
bool BaseObject::isRegistered() const {
	if(thisRef.objId != ObjId(-1)) return true;
	if(parentObject()) return parentObject()->isRegistered();
	return false;
}

std::string BaseObject::toString() const {
	return "<" + thisRef.description() + ">";
}

Result BaseObject::getAttrib(const ScriptVar_t& key, ScriptVar_t& value) const {
	if(key.type != SVT_STRING)
		return "only string-typed keys accepted for BaseObject";

	if(thisRef.classId == ClassId(-1))
		return "no classId assoziated";

	const AttrDesc* attrDesc = findAttrDescByName(key, thisRef.classId, true);
	if(!attrDesc)
		return "no attrib '" + std::string(key) + "'";

	value = attrDesc->get(this);
	return true;
}

Result BaseObject::setAttrib(const ScriptVar_t& key, const ScriptVar_t& value) {
	if(key.type != SVT_STRING)
		return "only string-typed keys accepted for BaseObject";

	if(thisRef.classId == ClassId(-1))
		return "no classId assoziated";

	const AttrDesc* attrDesc = findAttrDescByName(key, thisRef.classId, true);
	if(!attrDesc)
		return "no attrib '" + std::string(key) + "'";

	attrDesc->set(this, value);
	return true;
}

// We cannot use the REGISTER_CLASS macro because we cannot instantiate it.
static bool registerClass_BaseObject() {
	ClassInfo i;
	i.id = LuaID<BaseObject>::value;
	i.name = "BaseObject";
	i.memSize = sizeof(BaseObject);
	registerClass(i);
	return true;
}
static bool registerClass_BaseObject_init = registerClass_BaseObject();
