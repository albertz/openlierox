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
	typedef CScriptableVars::ScriptVarType_t VarType;
	typedef CScriptableVars::ScriptVar_t Var;
	VarType valueType; // for example: float for gamespeed, bool for ropelenchange
	Var unsetValue; // if the server does not provide this; for example: gamespeed=1; should always be like the behaviour without the feature to keep backward compatibility
	Var defaultValue; // for config, if not set before, in most cases the same as unsetValue
	Version minVersion; // if different from unsetValue
	bool SET;
	
	Feature() : SET(false) {}
	static Feature Unset() { return Feature(); }
	Feature(const std::string& n, const std::string& hn, const std::string& desc, bool unset, bool def, Version ver)
	: name(n), humanReadableName(hn), description(desc), valueType(CScriptableVars::SVT_BOOL), unsetValue(Var(unset)), defaultValue(Var(def)), minVersion(ver), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, int unset, int def, Version ver)
	: name(n), humanReadableName(hn), description(desc), valueType(CScriptableVars::SVT_INT), unsetValue(Var(unset)), defaultValue(Var(def)), minVersion(ver), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, float unset, float def, Version ver)
	: name(n), humanReadableName(hn), description(desc), valueType(CScriptableVars::SVT_FLOAT), unsetValue(Var(unset)), defaultValue(Var(def)), minVersion(ver), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, const std::string& unset, const std::string& def, Version ver)
	: name(n), humanReadableName(hn), description(desc), valueType(CScriptableVars::SVT_STRING), unsetValue(Var(unset)), defaultValue(Var(def)), minVersion(ver), SET(true) {}

};

extern Feature featureArray[];
inline int featureArrayLen() { int l = 0; for(Feature* f = featureArray; f->SET; ++f) ++l; return l; }

// Indexes of features in featureArray
// These indexes are only for local game use, not for network! (name is used there)
// (I am not that happy with this solution right now, I would like to put both these indexes together
//  with the actual declaration of the Feature. Though I don't know a way how to put both things together
//  in the header file.)
enum FeatureIndex {
	FT_FORCESCREENSHAKING = 0
};

class FeatureSettings {
private:
	CScriptableVars::ScriptVar_t* settings;
public:
	FeatureSettings(); ~FeatureSettings();
	FeatureSettings(const FeatureSettings& r) : settings(NULL) { (*this) = r; }
	FeatureSettings& operator=(const FeatureSettings& r);
	CScriptableVars::ScriptVar_t& operator[](FeatureIndex i) { return settings[i]; }
	CScriptableVars::ScriptVar_t& operator[](Feature* f) { return settings[f - &featureArray[0]]; }
	const CScriptableVars::ScriptVar_t& operator[](FeatureIndex i) const { return settings[i]; }
	const CScriptableVars::ScriptVar_t& operator[](Feature* f) const { return settings[f - &featureArray[0]]; }
};

#endif
