/*
 *  FeatureList.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.12.08.
 *	code under LGPL
 *
 */

#ifndef __FEATURELIST_H__
#define __FEATURELIST_H__

#include <string>
#include <set>
#include <map>
#include "Iterator.h"
#include "CScriptableVars.h"
#include "Version.h"

struct Feature {
	std::string name; // for config, network and other identification
	std::string humanReadableName;
	std::string description;
	CScriptableVars::ScriptVarType_t valueType; // for example: float for gamespeed, bool for ropelenchange
	CScriptableVars::ScriptVar_t unsetValue; // if the server does not provide this; for example: gamespeed=1
	CScriptableVars::ScriptVar_t defaultValue; // for config, if not set before, in most cases the same as unsetValue
	Version minVersion;
	bool UNSET;
	
	Feature() : UNSET(true) {}
	static Feature Unset() { return Feature(); }
};

extern Feature featureArray[];
inline int featureArrayLen() { int l = 0; for(Feature* f = featureArray; !f->UNSET; ++f) ++l; return l; }

struct FeatureSettings {
	CScriptableVars::ScriptVar_t* settings;
	FeatureSettings();
	~FeatureSettings() { if(settings) delete[] settings; }
	CScriptableVars::ScriptVar_t& operator[](Feature* f) {
		return settings[f - &featureArray[0]];
	}
};

typedef CScriptableVars::ScriptVar_t FeatureConfigurations[];

#endif
