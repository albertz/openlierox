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
#include "FindFile.h"

static bool isGusanosMod(const std::string& f) {
	// there is no better way to check that
	return IsDirectory(f + "/objects");
}

ModInfo infoForMod(const std::string& f, bool absolute) {	
	if(IsDirectory(f, absolute)) {
		{
			std::string name;
			if(CGameScript::CheckFile(f, name, absolute)) {
				ModInfo info;
				info.valid = true;
				info.name = name;
				info.path = GetBaseFilename(f);
				info.type = "LieroX mod";
				info.typeShort = "LX";
				return info;
			}
		}
	
		{
			if(isGusanosMod(GetBaseFilename(f))) {
				ModInfo info;
				info.valid = true;
				info.name = GetBaseFilename(f);
				info.path = info.name;
				info.type = "Gusanos mod";
				info.typeShort = "Gus";
				return info;
			}			
		}
	}
	
	return ModInfo();
}

std::string modName(const std::string& f) {
	ModInfo info = infoForMod(f);
	if(info.valid)
		return info.name;
	return "";
}
