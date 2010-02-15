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

	Version minVersion; // min supported version (<=beta8 is not updated automatically by the system) 
	// Old clients are kicked if feature version is greater that client version, no matter if feature is server-sided or optinal

	GameInfoGroup group;	// For grouping similar options in GUI

	AdvancedLevel advancedLevel;

	// TODO: make special type VarRange (which holds these hasmin/hasmax/min/max/signed)
	// TODO: move that to ScriptVarType_t
	Var minValue; // Min and max values are used in GUI to make sliders (only for float/int)
	Var maxValue; // Min and max values are used in GUI to make sliders (only for float/int)
	bool unsignedValue; // If the value is unsigned - ints and floats only
	
	bool serverSideOnly; // if true, all the following is just ignored
	bool optionalForClient; // Optional client-sided feature, like vision cone drawn for seekers, or SuicideDecreasesScore which required for precise damage calculation in scoreboard
	
	bool unsetIfOlderClients; // This flag will reset feature to unset value if an older client is connected

	typedef Var (GameServer::*GetValueFunction)( const Var& preset );
	GetValueFunction getValueFct; // if set, it uses the return value for hostGet
	
	bool SET; // Flag that marks end of global features list
	
	Feature() : SET(false) {}
	static Feature Unset() { return Feature(); }

	Feature(const std::string& n, const std::string& hn, const std::string& desc, bool unset, bool def, 
				Version ver, GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic, bool ssdo = false, bool opt = false, 
				bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_BOOL), unsetValue(Var(unset)), defaultValue(Var(def)), 
		minVersion(ver), group(g), advancedLevel(l), serverSideOnly(ssdo), optionalForClient(opt), 
		unsetIfOlderClients(u), getValueFct(f), SET(true) {}

	Feature(const std::string& n, const std::string& hn, const std::string& desc, int unset, int def, 
				Version ver, GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic, int minval = 0, int maxval = 0, bool ssdo = false, bool opt = false, 
				bool u = false, bool unsig = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_INT), unsetValue(Var(unset)), defaultValue(Var(def)), 
		minVersion(ver), group(g), advancedLevel(l), minValue(minval), maxValue(maxval), unsignedValue(unsig), serverSideOnly(ssdo), 
		optionalForClient(opt), unsetIfOlderClients(u), getValueFct(f), SET(true) {}

	Feature(const std::string& n, const std::string& hn, const std::string& desc, float unset, float def, 
				Version ver, GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic, float minval = 0.0f, float maxval = 0.0f, bool ssdo = false, bool opt = false, 
				bool u = false, bool unsig = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_FLOAT), unsetValue(Var(unset)), defaultValue(Var(def)), 
		minVersion(ver), group(g), advancedLevel(l), minValue(minval), maxValue(maxval), unsignedValue(unsig), serverSideOnly(ssdo), 
		optionalForClient(opt), unsetIfOlderClients(u), getValueFct(f), SET(true) {}

	Feature(const std::string& n, const std::string& hn, const std::string& desc, const char * unset, const char * def, 
				Version ver, GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic, bool ssdo = false, bool opt = false, 
				bool u = false, GetValueFunction f = NULL)
	: name(n), humanReadableName(hn), description(desc), valueType(SVT_STRING), unsetValue(Var(unset)), defaultValue(Var(def)), 
		minVersion(ver), group(g), advancedLevel(l), serverSideOnly(ssdo), optionalForClient(opt), 
		unsetIfOlderClients(u), getValueFct(f), SET(true) {}

};

extern Feature featureArray[];
inline int featureArrayLen() { int l = 0; for(Feature* f = featureArray; f->SET; ++f) ++l; return l; }
inline int clientSideFeatureCount() { int l = 0; for(Feature* f = featureArray; f->SET; ++f) if(!f->serverSideOnly) ++l; return l; }
Feature* featureByName(const std::string& name);

// Indexes of features in featureArray
// These indexes are only for local game use, not for network! (name is used there)
// (I am not that happy with this solution right now, I would like to put both these indexes together
//  with the actual declaration of the Feature. Though I don't know a way how to put both things together
//  in the header file.)
// WARNING: Keep this always synchronised with featureArray!
enum FeatureIndex {
	FT_GameSpeed = 0,
	FT_GameSpeedOnlyForProjs,
	FT_ScreenShaking,
	FT_FullAimAngle,
	FT_MiniMap,
	FT_SuicideDecreasesScore,
	FT_TeamkillDecreasesScore,
	FT_DeathDecreasesScore,
	FT_CountTeamkills,
	FT_AllowNegativeScore,
	FT_TeamInjure,
	FT_TeamHit,
	FT_SelfInjure,
	FT_SelfHit,
	FT_AllowEmptyGames,
	FT_HS_HideTime,		// Hide and Seek gamemode settings
	FT_HS_AlertTime,
	FT_HS_HiderVisionRange,
	FT_HS_HiderVisionRangeThroughWalls,
	FT_HS_SeekerVisionRange,
	FT_HS_SeekerVisionRangeThroughWalls,
	FT_HS_SeekerVisionAngle,
	FT_NewNetEngine,
	FT_FillWithBotsTo,
	FT_WormSpeedFactor,
	FT_WormDamageFactor,
	FT_WormShieldFactor,
	FT_InstantAirJump,
	FT_RelativeAirJump,
	FT_RelativeAirJumpDelay,
	FT_AllowWeaponsChange,
	FT_ImmediateStart,
	FT_DisableWpnsWhenEmpty,
	FT_WeaponCombos,
	FT_InfiniteMap,
	FT_WormFriction,
	FT_WormGroundFriction,
	FT_ProjFriction,
	FT_TeamScoreLimit,
	FT_SizeFactor,
	FT_CollideProjectiles,
	FT_CTF_AllowRopeForCarrier,
	FT_CTF_SpeedFactorForCarrier,
	FT_Race_Rounds,
	FT_Race_AllowWeapons,
	FT_Race_CheckPointRadius,
 
 	__FTI_BOTTOM
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
	std::map< std::string, Feature > list;
	Iterator< Feature & >::Ref iterator() { return GetIterator(list); }
	void set(const Feature& f) { list[ f.name ] = f; }
	void set(const std::string& name, const std::string& humanName, const ScriptVar_t& var, Feature::Type type) {
		Feature f;
		f.name = name;
		f.humanName = humanName;
		f.var = var;
		f.type = type;
		set(f);
	}
	const Feature * find( const std::string & name )
	{
		if( list.find( name ) == list.end() )
			return NULL;
		return &(list.find( name )->second);
	}
	void clear() { list.clear(); }
};

class FeatureSettings {
private:
	ScriptVar_t* settings;
public:
	FeatureSettings(); ~FeatureSettings();
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
