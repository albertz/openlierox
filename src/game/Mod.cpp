/*
 *  Mod.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#include "Mod.h"
#include "CGameScript.h"
#include "StringUtils.h"

ModInfo infoForMod(const std::string& f, bool absolute) {	
	std::string name;
	ModInfo info;
	CGameScript::CheckFile(f, name, absolute, &info);	
	return info;
}

std::string modName(const std::string& f) {
	ModInfo info = infoForMod(f);
	if(info.valid)
		return info.name;
	return "";
}

ModInfo::ModInfo() : valid(false) {
	thisRef.classId = LuaID<ModInfo>::value;
}

bool ModInfo::operator==(const CustomVar& o) const {
	const ModInfo* oi = dynamic_cast<const ModInfo*> (&o);
	return oi && stringcaseequal(path, oi->path);
}

bool ModInfo::operator<(const CustomVar& o) const {
	const ModInfo* oi = dynamic_cast<const ModInfo*> (&o);
	if(oi) return stringcasecmp(path, oi->path) < 0;
	return this < &o;	
}

std::string ModInfo::toString() const {
	return path;
}

bool ModInfo::fromString(const std::string & str) {
	*this = infoForMod(str, false);
	return true;
}

REGISTER_CLASS(ModInfo, LuaID<CustomVar>::value)
