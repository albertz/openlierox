/*
 *  Settings.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#include "Settings.h"
#include "Options.h"
#include "CServer.h"
#include "IniReader.h"
#include "Debug.h"
#include "gusanos/luaapi/classes.h"
#include "game/Game.h"
#include "CClient.h"

Settings gameSettings;
FeatureSettingsLayer modSettings("Mod properties");
FeatureSettingsLayer gamePresetSettings("Settings preset");


void FeatureSettingsLayer::makeSet(bool v) {
	for(size_t i = 0; i < FeatureArrayLen; ++i)
		isSet[i] = v;
}


ScriptVar_t& FeatureSettingsLayer::set(FeatureIndex i) {
	if(!isSet[i]) {
		((FeatureSettings&)(*this))[i] = featureArray[i].unsetValue; // just to be sure; in case modSettings is init before featureArray, also important
		isSet[i] = true;
	}

	gameSettings.pushUpdateHint(i);

	return ((FeatureSettings&)(*this))[i];
}

void FeatureSettingsLayer::dump() const {
	notes << debug_name << " {" << endl;
	for(size_t i = 0; i < FeatureArrayLen; ++i)
		if(isSet[i])
			notes << " " << featureArray[i].name << " : " << (*this)[(FeatureIndex)i] << endl;
	notes << "}" << endl;
}



void Settings::layersInitStandard(bool withCustom) {
	layersClear();
	layers.push_back(&modSettings);
	layers.push_back(&gamePresetSettings);
	if(withCustom) {
		if(tLXOptions)
			layers.push_back(&tLXOptions->customSettings);
		else
			errors << "Settings::layersInitStandard: tLXOptions == NULL" << endl;
	}
}


ScriptVar_t Settings::hostGet(FeatureIndex i) {
	ScriptVar_t var = (*this)[i];
	Feature* f = &featureArray[i];
	if(f->getValueFct)
		var = f->getValueFct( var );
	
	return var;
}

bool Settings::olderClientsSupportSetting(Feature* f) {
	if( f->optionalForClient ) return true;
	return hostGet(f) == f->unsetValue;
}


bool FeatureSettingsLayer::loadFromConfig(const std::string& cfgfile, bool reset, std::map<std::string, std::string>* unknown) {
	if(reset) makeSet(false);
	
	IniReader ini(cfgfile);
	if(!ini.Parse()) return false;
	
	IniReader::Section& section = ini.m_sections["GameInfo"];
	for(IniReader::Section::iterator i = section.begin(); i != section.end(); ++i) {
		Feature* f = featureByName(i->first);
		if(f) {
			FeatureIndex fi = featureArrayIndex(f);
			if(!set(fi).fromString(i->second))
				notes << "loadFromConfig " << cfgfile << ": cannot understand: " << i->first << " = " << i->second << endl;
		}
		else {
			if(unknown)
				(*unknown)[i->first] = i->second;
		}
	}
	
	return true;
}

bool Settings::isRelevant(FeatureSettingsLayer* layer, FeatureIndex i) const {
	for(Layers::const_reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it) {
		if(layer == *it) return true;
		if((*it)->isSet[i]) return false;
	}
	return false; // layer not registered at all
}

bool Settings::isRelevant(FeatureSettingsLayer* layer, Feature* f) const {
	return isRelevant(layer, featureArrayIndex(f));
}

void Settings::dumpAllLayers() const {
	notes << "Settings (" << layers.size() << " layers) {" << endl;
	size_t num = 1;
	for(Layers::const_iterator i = layers.begin(); i != layers.end(); ++i, ++num) {
		notes << "Layer " << num << ": ";
		(*i)->dump();
	}
	notes << "}" << endl;
}


static ScriptVar_t Settings_attrGetValue(const BaseObject* obj, const AttrDesc* attrDesc) {
	const Settings* s = dynamic_cast<const Settings*>(obj);
	assert(s != NULL);
	assert(s == &gameSettings); // it's a singleton
	return s->attrGetValue(attrDesc);
}

static AttrExt& Settings_attrGetAttrExt(BaseObject* obj, const AttrDesc* attrDesc) {
	Settings* s = dynamic_cast<Settings*>(obj);
	assert(s != NULL);
	assert(s == &gameSettings); // it's a singleton
	return s->attrGetAttrExt(attrDesc);
}

Settings::AttrDescs::AttrDescs() {
	for(size_t i = 0; i < FeatureArrayLen; ++i) {
		attrDescs[i].objTypeId = LuaID<Settings>::value;
		attrDescs[i].attrId = featureArray[i].id;
		attrDescs[i].attrType = featureArray[i].valueType;
		attrDescs[i].attrName = featureArray[i].name;
		attrDescs[i].defaultValue = featureArray[i].unsetValue;
		attrDescs[i].isStatic = false;
		attrDescs[i].dynGetValue = Settings_attrGetValue;
		attrDescs[i].dynGetAttrExt = Settings_attrGetAttrExt;
		attrDescs[i].onUpdate = Game::onSettingsUpdate;
		registerAttrDesc(attrDescs[i]);
	}
}

Settings::Settings() {
	thisRef.classId = LuaID<Settings>::value;
	thisRef.objId = 1;
	for(size_t i = 0; i < FeatureArrayLen; ++i) {
		wrappers[i].i = (FeatureIndex)i;
		wrappers[i].s = this;
	}
}

bool Settings::AttrDescs::belongsToUs(const AttrDesc *attrDesc) {
	return attrDesc >= &attrDescs[0] && attrDesc < &attrDesc[FeatureArrayLen];
}

FeatureIndex Settings::AttrDescs::getIndex(const AttrDesc* attrDesc) {
	assert(belongsToUs(attrDesc));
	return FeatureIndex(attrDesc - &attrDescs[0]);
}

void Settings::pushUpdateHint(FeatureIndex i) {
	pushObjAttrUpdate(*this, &getAttrDescs().attrDescs[i]);
}

void Settings::pushUpdateHintAll() {
	for(size_t i = 0; i < FeatureArrayLen; ++i)
		pushUpdateHint((FeatureIndex)i);
}

ScriptVar_t Settings::attrGetValue(const AttrDesc* attrDesc) const {
	FeatureIndex i = getAttrDescs().getIndex(attrDesc);
	if(game.isServer() || game.state <= Game::S_Inactive)
		return (*this)[i];
	else
		return cClient->getGameLobby()[i];
}

AttrExt& Settings::attrGetAttrExt(const AttrDesc* attrDesc) {
	FeatureIndex i = getAttrDescs().getIndex(attrDesc);
	return attrExts[i];
}

REGISTER_CLASS(Settings, ClassId(-1))
