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
#include <set>
#include "gui/List.h"

struct ModInfo {
	bool valid;
	std::string name;
	std::string path;
	std::string type;
	std::string typeShort;
	ModInfo() : valid(false) {}
};

ModInfo infoForMod(const std::string& f, bool absolute = false);
std::string modName(const std::string& f);

struct GameSettingsPresetInfo {
	bool global;
	std::string name;
	std::string path;
	std::string description;
	
	std::pair<bool,std::string> compareData() const { return std::make_pair(global, name); }
	bool operator==(const GameSettingsPresetInfo& i) const { return compareData() == i.compareData(); }
	bool operator!=(const GameSettingsPresetInfo& i) const { return compareData() != i.compareData(); }
	bool operator<(const GameSettingsPresetInfo& i) const { return compareData() < i.compareData(); }
};

std::set<GameSettingsPresetInfo> presetsForMod(const std::string& modDir);
GuiListItem::Pt infoForSettingsPreset(const GameSettingsPresetInfo& preset);
GuiList::Pt dynamicPresetListForCurrentMod();


#endif

