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
#include "util/CustomVar.h"
#include "game/Attr.h"

struct ModInfo : CustomVar {
	ModInfo();
	static ModInfo ByPath(const std::string& _p) { ModInfo info; info.valid = true; info.path = _p; return info; }
	bool valid;
	ATTR(ModInfo, std::string, name, 1, {})
	ATTR(ModInfo, std::string, path, 2, {})
	ATTR(ModInfo, std::string, typeShort, 3, {})
	std::string type;
	
	virtual CustomVar* copy() const { return new ModInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

ModInfo infoForMod(const std::string& f, bool absolute = false);
std::string modName(const std::string& f);

#endif

