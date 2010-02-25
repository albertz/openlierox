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

struct ModInfo : CustomVar {
	ModInfo() : valid(false) {}
	ModInfo(const std::string& _path) : valid(true), path(_path) {}
	bool valid;
	std::string name;
	std::string path;
	std::string type;
	std::string typeShort;
	
	virtual CustomVar* copy() const { return new ModInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

ModInfo infoForMod(const std::string& f, bool absolute = false);
std::string modName(const std::string& f);

#endif

