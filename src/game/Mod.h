/*
 *  Mod.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#ifndef __OLX_MOD_H__
#define __OLX_MOD_H__

#include <string>

struct ModLoader;

struct ModInfo {
	bool valid;
	std::string name;
	std::string path;
	std::string type;
	std::string typeShort;
	ModLoader* loader;
	ModInfo() : valid(false), loader(NULL) {}
};

ModInfo infoForMod(const std::string& f, bool absolute = false);
std::string modName(const std::string& f);


#endif

