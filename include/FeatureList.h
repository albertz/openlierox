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

class GameServer;

struct Feature {
	std::string name; // for config, network and other identification
	std::string humanReadableName;
	std::string description;
	typedef ScriptVarType_t VarType;
	typedef ScriptVar_t Var;
	VarType valueType; // for example: float for gamespeed, bool for ropelenchange
	Var unsetValue; // if the server does not provide this; for example: gamespeed=1; should always be like the behaviour without the feature to keep backward compatibility
	Var defaultValue; // for config, if not set before, in most cases the same as unsetValue
	// TODO: dont make it int! (make it a reference/pointer to the group or at least an enum)
	int group;	// For grouping similar options in GUI
	// TODO: move that to ScriptVarType_t
	float minValue; // Min and max values are used in GUI to make sliders (only for float/int)
	float maxValue; // Min and max values are used in GUI to make sliders (only for float/int)
	
	bool serverSideOnly; // if true, all the following is just ignored
	
	Version minVersion; // min supported version (<=beta8 is not updated automatically by the system) 
	bool unsetIfOlderClients; // if getValueFct is not set, it automatically uses the unsetValue for hostGet
	typedef Var (GameServer::*GetValueFunction)( const Var& preset );
	GetValueFunction getValueFct; // if set, it uses the return value for hostGet
	
	bool SET;
	
	Feature() : SET(false) {}
	static Feature Unset() { return Feature(); }
	Feature(const std::string& n, const std::string& hn, const std::string& desc, bool unset, bool def, Version ver, int g = -1, bool ssdo = false, bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_BOOL), unsetValue(Var(unset)), defaultValue(Var(def)), group(g), serverSideOnly(ssdo), minVersion(ver), unsetIfOlderClients(u), getValueFct(f), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, int unset, int def, Version ver, int g = -1, float minval = 0.0f, float maxval = 0.0f, bool ssdo = false, bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_INT), unsetValue(Var(unset)), defaultValue(Var(def)), group(g), minValue(minval), maxValue(maxval), serverSideOnly(ssdo), minVersion(ver), unsetIfOlderClients(u), getValueFct(f), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, float unset, float def, Version ver, int g = -1, float minval = 0.0f, float maxval = 0.0f, bool ssdo = false, bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_FLOAT), unsetValue(Var(unset)), defaultValue(Var(def)), group(g), minValue(minval), maxValue(maxval), serverSideOnly(ssdo), minVersion(ver), unsetIfOlderClients(u), getValueFct(f), SET(true) {}
	Feature(const std::string& n, const std::string& hn, const std::string& desc, const std::string& unset, const std::string& def, Version ver, int g = -1, bool ssdo = false, bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_STRING), unsetValue(Var(unset)), defaultValue(Var(def)), group(g), serverSideOnly(ssdo), minVersion(ver), unsetIfOlderClients(u), getValueFct(f), SET(true) {}

};

extern Feature featureArray[];
inline int featureArrayLen() { int l = 0; for(Feature* f = featureArray; f->SET; ++f) ++l; return l; }
inline int clientSideFeatureCount() { int l = 0; for(Feature* f = featureArray; f->SET ; ++f) if(!f->serverSideOnly) ++l; return l; }
Feature* featureByName(const std::string& name);

// Indexes of features in featureArray
// These indexes are only for local game use, not for network! (name is used there)
// (I am not that happy with this solution right now, I would like to put both these indexes together
//  with the actual declaration of the Feature. Though I don't know a way how to put both things together
//  in the header file.)
// WARNING: Keep this always synchronised with featureArray!
// WARNING: With current implementation (see FeatureSettings constr. and clientSideFeatureCount), serverSideOnly-features have to stay at end!
enum FeatureIndex {
	FT_GameSpeed = 0,
	FT_ForceScreenShaking,
	FT_SuicideDecreasesScore,
	FT_TeamInjure,
	FT_TeamHit,
	FT_SelfInjure,
	FT_SelfHit,
	FT_AllowEmptyGames,
	FT_CountTeamkills,	// Client should know this to calculate damage correctly in teamgames
	FT_HS_HideTime,		// Hide and Seek gamemode settings
	FT_HS_AlertTime,
	FT_HS_HiderVisionRange,
	FT_HS_HiderVisionRangeThroughWalls,
	FT_HS_SeekerVisionRange,
	FT_HS_SeekerVisionRangeThroughWalls,
	FT_HS_SeekerVisionAngle,
};

class FeatureCompatibleSettingList {
public:
	struct Feature {
		std::string name;
		std::string humanName;
		ScriptVar_t var;
		enum Type { FCSL_SUPPORTED, FCSL_JUSTUNKNOWN, FCSL_INCOMPATIBLE };
		Type type;
	};
	std::list<Feature> list;
	Iterator<Feature&>::Ref iterator() { return GetIterator(list); }
	void push_back(const Feature& f) { list.push_back(f); }
	void push_back(const std::string& name, const std::string& humanName, const ScriptVar_t& var, Feature::Type type) {
		Feature f;
		f.name = name;
		f.humanName = humanName;
		f.var = var;
		f.type = type;
		push_back(f);
	}
	void set(const std::string& name, const std::string& humanName, const ScriptVar_t& var, Feature::Type type);
	void clear() { list.clear(); }
};

class FeatureSettings {
private:
	ScriptVar_t* settings;
	unsigned long len;
public:
	FeatureSettings(bool alsoServerSide = true); ~FeatureSettings();
	FeatureSettings(const FeatureSettings& r) : settings(NULL) { (*this) = r; }
	FeatureSettings& operator=(const FeatureSettings& r);
	
	ScriptVar_t& operator[](FeatureIndex i) { return settings[i]; }
	ScriptVar_t& operator[](Feature* f) { return settings[f - &featureArray[0]]; }
	const ScriptVar_t& operator[](FeatureIndex i) const { return settings[i]; }
	const ScriptVar_t& operator[](Feature* f) const { return settings[f - &featureArray[0]]; }
	
	ScriptVar_t hostGet(FeatureIndex i);
	ScriptVar_t hostGet(Feature* f) { return hostGet(FeatureIndex(f - &featureArray[0])); }
	bool olderClientsSupportSetting(Feature* f);
};

#endif
