//
//  Attr.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 20.01.12.
//  code under LGPL
//

#include <map>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include "Attr.h"
#include "util/macros.h"
#include "util/StaticVar.h"
#include "Debug.h"
#include "util/StringConv.h"
#include "gusanos/luaapi/classes.h"
#include "Game.h"
#include "GameState.h"
#include "CBytestream.h"
#include "OLXCommand.h"
#include "CClient.h"

typedef std::map<AttribRef, const AttrDesc*> AttrDescs;
static StaticVar<AttrDescs> attrDescs;

void AttrDesc::set(BaseObject* base, const ScriptVar_t& v, bool authorizedByServer) const {
	assert(isStatic); // not yet implemented otherwise... we would need another dynamic function
	if(!authorizedByServer && !authorizedToWrite(base)) return; // silently for now...
	pushObjAttrUpdate(*base, this);
	getValueScriptPtr(base).fromScriptVar(v);
}

bool AttrDesc::authorizedToWrite(const BaseObject* base) const {
	assert(base != NULL);
	if(authorizedToWriteExtra && !authorizedToWriteExtra(base, this)) return false;
	if(base->thisRef.objId == ObjId(-1)) return true; // not registered objects can always be written
	if(game.state <= Game::S_Inactive) return true;
	if(game.isServer()) return true;
	if(cClient->getServerVersion() < OLXBetaVersion(0,59,10)) return true; // old protocol, we just manage it manually
	if(!serverside && base->thisRef.ownThis()) return true;
	if(this == Game::state_Type::attrDesc()) return true; // small exception
	return false;
}

std::string AttrDesc::description() const {
	return std::string(LuaClassName(objTypeId)) + ":" + attrName;
}

AttribRef::AttribRef(const AttrDesc* attrDesc) {
	objTypeId = attrDesc->objTypeId;
	attrId = attrDesc->attrId;
}

void AttribRef::writeToBs(CBytestream* bs) const {
	bs->writeInt16(objTypeId);
	bs->writeInt16(attrId);
}

void AttribRef::readFromBs(CBytestream* bs) {
	objTypeId = bs->readInt16();
	attrId = bs->readInt16();
}

std::string AttribRef::description() const {
	const AttrDesc* attrDesc = getAttrDesc();
	if(attrDesc) return attrDesc->attrName;
	else return "<unknown attr " + to_string(objTypeId) + ":" + to_string(attrId) + ">";
}

AttribRef AttribRef::LowerLimit(ClassId c) {
	AttribRef r;
	r.objTypeId = c;
	r.attrId = 0;
	return r;
}

AttribRef AttribRef::UpperLimit(ClassId c) {
	AttribRef r;
	r.objTypeId = c;
	r.attrId = AttrDesc::AttrId(-1);
	return r;
}

const AttrDesc* AttribRef::getAttrDesc() const {
	AttrDescs::iterator it = attrDescs->find(*this);
	if(it != attrDescs->end()) return it->second;
	return NULL;
}

ObjAttrRef::ObjAttrRef(ObjRef o, const AttrDesc* attrDesc) {
	obj = o;
	attr = AttribRef(attrDesc);
}

void ObjAttrRef::writeToBs(CBytestream* bs) const {
	obj.writeToBs(bs);
	attr.writeToBs(bs);
}

void ObjAttrRef::readFromBs(CBytestream* bs) {
	obj.readFromBs(bs);
	attr.readFromBs(bs);
}

std::string ObjAttrRef::description() const {
	return "<" + obj.description() + "> " + attr.description();
}

ObjAttrRef ObjAttrRef::LowerLimit(ObjRef o) {
	ObjAttrRef r;
	r.obj = o;
	r.attr.objTypeId = 0;
	r.attr.attrId = 0;
	return r;
}

ObjAttrRef ObjAttrRef::UpperLimit(ObjRef o) {
	ObjAttrRef r;
	r.obj = o;
	r.attr.objTypeId = ClassId(-1);
	r.attr.attrId = AttrDesc::AttrId(-1);
	return r;
}

ScriptVar_t ObjAttrRef::get() const {
	const BaseObject* oPt = obj.obj.get();
	assert(oPt != NULL); // or should we return the attr default?
	const AttrDesc* attrDesc = attr.getAttrDesc();
	assert(attrDesc != NULL);
	return attrDesc->get(oPt);
}


void registerAttrDesc(AttrDesc& attrDesc) {
	attrDescs.get()[AttribRef(&attrDesc)] = &attrDesc;
}

void iterAttrDescs(ClassId classId, bool withSuperClasses, boost::function<void(const AttrDesc* attrDesc)> callback) {
	AttrDescs::iterator itStart = attrDescs->lower_bound(AttribRef::LowerLimit(classId));
	AttrDescs::iterator itEnd = attrDescs->upper_bound(AttribRef::UpperLimit(classId));
	for(AttrDescs::iterator it = itStart; it != itEnd; ++it)
		callback(it->second);

	if(withSuperClasses) {
		const ClassInfo* classInfo = getClassInfo(classId);
		assert(classInfo != NULL);
		if(classInfo->superClassId != ClassId(-1))
			iterAttrDescs(classInfo->superClassId, withSuperClasses, callback);
	}
}

static void _addAttrDesc(std::vector<const AttrDesc*>& vec, const AttrDesc* attrDesc) {
	vec.push_back(attrDesc);
}

std::vector<const AttrDesc*> getAttrDescs(ClassId classId, bool withSuperClasses) {
	using namespace boost;
	std::vector<const AttrDesc*> vec;
	function<void(const AttrDesc* attrDesc)> callback = bind(_addAttrDesc, ref(vec), _1);
	iterAttrDescs(classId, withSuperClasses, callback);
	return vec;
}

static std::vector< WeakRef<BaseObject> > objUpdates;

static void pushObjAttrUpdate(BaseObject& obj) {
	objUpdates.push_back(obj.thisRef.obj);
}

void pushObjAttrUpdate(BaseObject& obj, const AttrDesc* attrDesc) {
	if(obj.attrUpdates.empty())
		pushObjAttrUpdate(obj);
	AttrExt& ext = attrDesc->getAttrExt(&obj);
	if(!ext.updated || obj.attrUpdates.empty()) {
		AttrUpdateInfo info;
		info.attrDesc = attrDesc;
		info.oldValue = attrDesc->get(&obj);
		obj.attrUpdates.push_back(info);
		ext.updated = true;
	}
}

static void handleAttrUpdateLogging(BaseObject* oPt, const AttrDesc* attrDesc, ScriptVar_t oldValue) {
	// For now, just to console.
	// For some, no debug msg, too annoying.
	if(attrDesc->objTypeId == LuaID<CGameObject>::value && attrDesc->attrName == "vPos") return;
	if(attrDesc->objTypeId == LuaID<CGameObject>::value && attrDesc->attrName == "vVelocity") return;
	if(attrDesc->objTypeId == LuaID<Game>::value && attrDesc->attrName == "serverFrame") return;
	if(attrDesc->objTypeId == LuaID<CWorm>::value && attrDesc->attrName == "tState") return;
	if(attrDesc->objTypeId == LuaID<CWorm>::value && attrDesc->attrName == "iAFK") return;
	if(attrDesc->objTypeId == LuaID<CWorm>::value && attrDesc->attrName == "sAFKMessage") return;
	if(attrDesc->objTypeId == LuaID<CWorm>::value && attrDesc->attrName == "iCurrentWeapon") return;
	if(!oPt->thisRef)
		return; // obj not really registered
	notes << "<" << oPt->thisRef.description() << "> " << attrDesc->description() << ": update " << oldValue.toString() << " -> " << attrDesc->get(oPt).toString() << endl;
}

void iterAttrUpdates(boost::function<void(BaseObject*, const AttrDesc* attrDesc, ScriptVar_t oldValue)> callback) {
	foreach(o, objUpdates) {
		BaseObject* oPt = o->get();
		if(oPt == NULL) continue;

		foreach(u, oPt->attrUpdates) {
			const AttrDesc* const attrDesc = u->attrDesc;
			ScriptVar_t& oldValue = u->oldValue;

			attrDesc->getAttrExt(oPt).updated = false;
			if(oldValue == attrDesc->get(oPt)) continue;

			if(oPt->thisRef) // if registered
				game.gameStateUpdates->pushObjAttrUpdate(ObjAttrRef(oPt->thisRef, attrDesc));

			if(callback)
				callback(oPt, attrDesc, oldValue);
			if(attrDesc->onUpdate)
				attrDesc->onUpdate(oPt, attrDesc, oldValue);

			handleAttrUpdateLogging(oPt, attrDesc, oldValue);
		}

		oPt->attrUpdates.clear();
	}

	objUpdates.clear();
}


void dumpObject(const BaseObject* obj, CmdLineIntf* cliOut) {
	assert(obj != NULL);
	assert(obj->thisRef.classId != ClassId(-1));
	if(cliOut == NULL) cliOut = &stdoutCLI();

	cliOut->writeMsg(obj->thisRef.description() + " = {");
	std::vector<const AttrDesc*> attrDescs = getAttrDescs(obj->thisRef.classId, true);
	foreach(a, attrDescs) {
		cliOut->writeMsg("  " + (*a)->attrName + ": " + (*a)->get(obj).toString());
	}
	cliOut->writeMsg("}");
}

