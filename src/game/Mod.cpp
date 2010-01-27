/*
 *  Mod.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#include "game/Mod.h"
#include "CGameScript.h"
#include "FindFile.h"

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
