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

ModInfo infoForMod(const std::string& f, bool absolute) {
	ModInfo info;
	
	{
		std::string name;
		if(CGameScript::CheckFile(f, name, absolute)) {
			info.valid = true;
			info.name = name;
			info.path = GetBaseFilename(f);
			info.type = "LieroX mod";
			info.typeShort = "LX";
			return info;
		}
	}
	
	// TODO gus
	
	return info;
}

std::string modName(const std::string& f) {
	ModInfo info = infoForMod(f);
	if(info.valid)
		return info.name;
	return "";
}
