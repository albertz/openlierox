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
};

class FeatureList {
private:
	std::set<Feature*> m_features;
	static FeatureList* m_instance;
public:
	static void Init();
	static FeatureList* Instance() { return m_instance; }
	std::set<Feature*>& features() { return m_features; }
	FeatureList();
	~FeatureList();
};

typedef std::map<Feature*, CScriptableVars::ScriptVar_t> FeatureConfigurations;

#endif
