/*
 *  Mod.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#include <boost/bind.hpp>
#include "Mod.h"
#include "CGameScript.h"
#include "FindFile.h"
#include "Options.h"
#include "util/macros.h"
#include "ConfigHandler.h"
#include "DeprecatedGUI/CCombobox.h"
#include "gui/List.h"

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

static GameSettingsPresetInfo settingsInfo(const std::string& path, bool global) {
	GameSettingsPresetInfo info;
	info.global = global;
	if(!ReadString(path, "GameSettings", "Name", info.name, ""))
		info.name = GetBaseFilenameWithoutExt(path);
	ReadString(path, "GameSettings", "Description", info.description, "");
	info.path = path;
	return info;
}

std::set<GameSettingsPresetInfo> presetsForMod(const std::string& modDir) {
	std::set<GameSettingsPresetInfo> presets;
	for(Iterator<std::string>::Ref it = FileListIter(".", false, FM_REG, "*.gamesettings"); it->isValid(); it->next())
		presets.insert(settingsInfo("./" + GetBaseFilename(it->get()), true));
	
	for(Iterator<std::string>::Ref it = FileListIter(modDir, false, FM_REG, "*.gamesettings"); it->isValid(); it->next())
		presets.insert(settingsInfo(modDir + "/" + GetBaseFilename(it->get()), false));

	return presets;
}

GuiListItem::Pt infoForSettingsPreset(const GameSettingsPresetInfo& preset) {
	struct Item : GuiListItem {
		Item(const GameSettingsPresetInfo& p) : preset(p) {}
		GameSettingsPresetInfo preset;
		
		virtual std::string caption() { return preset.name; }
		virtual std::string tooltip() { return preset.description; }		
		virtual std::string index() { return preset.path; }		
	};
	
	return new Item(preset);
}

static std::list<GuiListItem::Pt> getCurrentSettingsPresetList() {
	std::set<GameSettingsPresetInfo> presets = presetsForMod(tLXOptions->tGameInfo.sModDir);
	std::list<GuiListItem::Pt> presetList;
	foreach(i, presets)
		presetList.push_back(infoForSettingsPreset(*i));
	return presetList;
}

GuiList::Pt dynamicPresetListForCurrentMod() {
	return dynamicGuiList(boost::bind(&getCurrentSettingsPresetList));
}

void setupModGameSettingsPresetComboboxes(DeprecatedGUI::CCombobox* modList, DeprecatedGUI::CCombobox* presetList) {
	return; // TODO: ???
	presetList->setListBackend( dynamicPresetListForCurrentMod() );
	modList->OnChangeSelection.connect( boost::bind(&DeprecatedGUI::CCombobox::updateFromListBackend, presetList) );
	presetList->updateFromListBackend(); // also update right now
}


