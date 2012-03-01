/*
 *  Settings.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

/*
 These settings classes are about the game specific settings - esp. everything from the FeatureList.
 All available game settings are exactly (and only) those in the FeatureList.
 
 The OLX game settings system is layer-based. The Settings class searches from the top layer to the bottom,
 until it finds a layer with the specific setting set. If it doesn't find any, it returns the default.
 
 In a standard game, these are the layers (from bottom to top):
   * modSettings (covers everything what is read from LX56 gamescripts or what is set by Gusanos mod)
   * gamePresetSettings (mod/gamesettings.cfg + F[FT_SettingsPreset] (*.gamesettings))
   * tLXOptions->customSettings
 
 The server tells the client about these settings as usual.
 (Some special handling is done about lx56modSettings for old clients but that doesn't matter.)
 
 The client doesn't use layers. It only uses cClient->getGameLobby().
 */

#ifndef __OLX_GAME_SETTINGS_H__
#define __OLX_GAME_SETTINGS_H__

// stupid MSVC
#ifdef _MSVC
#pragma warning(disable: 4355)  // Warning: this used in base member initializer list
#endif

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include "FeatureList.h"
#include "CodeAttributes.h"
#include "util/macros.h"
#include "util/BaseObject.h"
#include "game/Attr.h"

struct FeatureSettingsLayer : private FeatureSettings {
	std::string debug_name;
	bool isSet[FeatureArrayLen];
	FeatureSettingsLayer(const std::string& debug_name_) : debug_name(debug_name_) { makeSet(false); }
	void makeSet(bool v = true);

	const ScriptVar_t& operator[](FeatureIndex i) const { return ((FeatureSettings&)(*this))[i]; }
	const ScriptVar_t& operator[](Feature* f) const { return (*this)[featureArrayIndex(f)]; }

	ScriptVar_t& set(FeatureIndex i);

	template<typename T>
	void copyTo(T& s) const {
		for(size_t i = 0; i < FeatureArrayLen; ++i)
			if(isSet[i])
				s[(FeatureIndex)i] = (ScriptVar_t)(*this)[(FeatureIndex)i];
	}
	void dump() const; // to notes
	
	bool loadFromConfig(const std::string& cfgfile, bool reset, std::map<std::string, std::string>* unknown = NULL);
};

struct Settings : BaseObject {
	Settings();
	
	typedef std::vector<FeatureSettingsLayer*> Layers;
	Layers layers;
	void layersClear() { layers.clear(); }
	void layersInitStandard(bool withCustom); // settingpreset + customsettings

	FeatureSettingsLayer* layerFor(FeatureIndex i) const {
		for(Layers::const_reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
			if((*it)->isSet[i])
				return *it;
		return NULL;
	}

	const ScriptVar_t& operator[](FeatureIndex i) const {
		if(FeatureSettingsLayer* s = layerFor(i))
			return (*s)[i];
		return featureArray[i].defaultValue;
	}

	const ScriptVar_t& operator[](Feature* f) const { return (*this)[FeatureIndex(f - &featureArray[0])]; }
	
	struct OverwriteWrapper {
		ScriptVar_t& operator[](FeatureIndex i) {
			Settings& s = *__OLX_BASETHIS(Settings, overwrite);
			assert(!s.layers.empty());
			if(!s.layers.back()->isSet[i])
				s.layers.back()->set(i) = s[i];
			return s.layers.back()->set(i);
		}
	};
	OverwriteWrapper overwrite;
	
	// used for CScriptableVars::RegisterVars
	struct ScriptVarWrapper : _DynamicVar {
		FeatureIndex i; Settings* s;
		virtual ScriptVarType_t type() { return featureArray[i].valueType; }
		virtual ScriptVar_t asScriptVar() { return (*s)[i]; }
		virtual void fromScriptVar(const ScriptVar_t& v) { s->overwrite[i].fromScriptVar(v); }
		virtual const AttrDesc* getAttrDesc() const { return &Settings::getAttrDescs().attrDescs[i]; }
	};
	ScriptVarWrapper wrappers[FeatureArrayLen];
	
	void dumpAllLayers() const; // to notes
	ScriptVar_t hostGet(FeatureIndex i);
	ScriptVar_t hostGet(Feature* f) { return hostGet(FeatureIndex(f - &featureArray[0])); }
	bool olderClientsSupportSetting(Feature* f);

	struct AttrDescs {
		AttrDesc attrDescs[FeatureArrayLen];
		AttrDescs();
		bool belongsToUs(const AttrDesc* attrDesc);
		FeatureIndex getIndex(const AttrDesc* attrDesc);
	};
	static AttrDescs& getAttrDescs() {
		static AttrDescs attrDescs;
		return attrDescs;
	}
	AttrExt attrExts[FeatureArrayLen];

	void pushUpdateHintAll();
	void pushUpdateHint(FeatureIndex i);
	ScriptVar_t attrGetValue(const AttrDesc* attrDesc) const;
	AttrExt& attrGetAttrExt(const AttrDesc* attrDesc);
};

extern Settings gameSettings;
extern FeatureSettingsLayer modSettings; // in case of LX56 mod: for LX56 mod settings (WormGravity, etc); otherwise Gusanos settings
extern FeatureSettingsLayer gamePresetSettings; // for game settings preset

INLINE Feature* featureOfDynamicVar(_DynamicVar* var) {
	Settings::ScriptVarWrapper* wrapper = dynamic_cast<Settings::ScriptVarWrapper*> (var);
	if(wrapper) return &featureArray[wrapper->i];
	return NULL;
}

INLINE bool dynamicVarIsFeature(_DynamicVar* var, FeatureIndex i) {
	Settings::ScriptVarWrapper* wrapper = dynamic_cast<Settings::ScriptVarWrapper*> (var);
	if(wrapper) return wrapper->i == i;
	return false;
}

#endif

