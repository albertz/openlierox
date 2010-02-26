/*
 *  Settings.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
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

struct FeatureSettingsLayer : FeatureSettings {
	bool isSet[FeatureArrayLen];
	FeatureSettingsLayer() { makeSet(false); }
	void makeSet(bool v = true) { for(size_t i = 0; i < FeatureArrayLen; ++i) isSet[i] = v; }
	
	ScriptVar_t& set(FeatureIndex i) { isSet[i] = true;	return (*this)[i]; }
	void copyTo(FeatureSettings& s) const;
	
	bool loadFromConfig(const std::string& cfgfile, bool reset = true, std::map<std::string, std::string>* unknown = NULL);
};

struct Settings {
	Settings() : overwrite(*this) {
		for(size_t i = 0; i < FeatureArrayLen; ++i) {
			wrappers[i].i = (FeatureIndex)i;
			wrappers[i].s = this;
		}
	}
	
	typedef std::vector<FeatureSettingsLayer*> Layers;
	Layers layers;
	void layersClear() { layers.clear(); }
	void layersInitStandard(); // settingpreset + customsettings

	const ScriptVar_t& operator[](FeatureIndex i) const {
		for(Layers::const_reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
			if((*it)->isSet[i])
				return (**it)[i];
		return featureArray[i].defaultValue;
	}

	const ScriptVar_t& operator[](Feature* f) const { return (*this)[FeatureIndex(f - &featureArray[0])]; }

	struct OverwriteWrapper {
		Settings& s; OverwriteWrapper(Settings& _s) : s(_s) {}
		ScriptVar_t& operator[](FeatureIndex i) {
			assert(!s.layers.empty());
			if(!s.layers.back()->isSet[i]) {
				(*s.layers.back())[i] = s[i];
				s.layers.back()->isSet[i] = true;
			}
			return (*s.layers.back())[i];	
		}
	};
	OverwriteWrapper overwrite;
	
	struct ScriptVarWrapper : _DynamicVar {
		FeatureIndex i; Settings* s;
		virtual ScriptVarType_t type() { return featureArray[i].valueType; }
		virtual ScriptVar_t asScriptVar() { return (*s)[i]; }
		virtual void fromScriptVar(const ScriptVar_t& v) { s->overwrite[i].fromScriptVar(v); }
	};
	ScriptVarWrapper wrappers[FeatureArrayLen];
		
	ScriptVar_t hostGet(FeatureIndex i);
	ScriptVar_t hostGet(Feature* f) { return hostGet(FeatureIndex(f - &featureArray[0])); }
	bool olderClientsSupportSetting(Feature* f);	
};

extern Settings gameSettings;
extern FeatureSettingsLayer gamePresetSettings; // for game settings preset

inline Feature* featureOfDynamicVar(_DynamicVar* var) {
	Settings::ScriptVarWrapper* wrapper = dynamic_cast<Settings::ScriptVarWrapper*> (var);
	if(wrapper) return &featureArray[wrapper->i];
	return NULL;
}

inline bool dynamicVarIsFeature(_DynamicVar* var, FeatureIndex i) {
	Settings::ScriptVarWrapper* wrapper = dynamic_cast<Settings::ScriptVarWrapper*> (var);
	if(wrapper) return wrapper->i == i;
	return false;
}

#endif

