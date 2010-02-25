/*
 *  SettingsPreset.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_GAME_SETTINGSPRESET_H__
#define __OLX_GAME_SETTINGSPRESET_H__

#include <string>
#include <set>
#include "gui/List.h"
#include "util/CustomVar.h"

struct GameSettingsPresetInfo : CustomVar {
	GameSettingsPresetInfo() : global(true) {}
	static GameSettingsPresetInfo Default();
	
	bool global;
	std::string name;
	std::string path;
	std::string description;
	
	std::pair<bool,std::string> compareData() const { return std::make_pair(global, name); }
	bool operator==(const GameSettingsPresetInfo& i) const { return compareData() == i.compareData(); }
	bool operator!=(const GameSettingsPresetInfo& i) const { return compareData() != i.compareData(); }
	bool operator<(const GameSettingsPresetInfo& i) const { return compareData() < i.compareData(); }

	virtual CustomVar* copy() const { return new GameSettingsPresetInfo(*this); }
	virtual bool operator==(const CustomVar& o) const;
	virtual bool operator<(const CustomVar& o) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

std::set<GameSettingsPresetInfo> presetsForMod(const std::string& modDir);
GuiListItem::Pt infoForSettingsPreset(const GameSettingsPresetInfo& preset);
GuiList::Pt dynamicPresetListForCurrentMod();

namespace DeprecatedGUI { class CCombobox; }
void setupModGameSettingsPresetComboboxes(DeprecatedGUI::CCombobox* modList, DeprecatedGUI::CCombobox* presetList);

#endif
