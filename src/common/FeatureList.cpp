/*
 *  FeatureList.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.12.08.
 *  code under LGPL
 *
 */


#include "FeatureList.h"
#include "Version.h"
#include "CServer.h"



// WARNING: Keep this always synchronised with FeatureIndex!
// Legend:	Name in options,		Human-readable-name,			Long description,	
//			Unset,	Default,		Min client Version,	Group,				[Min,]	[Max,]	[server-side only] (Min and Max are only for Int and Float)

Feature featureArray[] = {
	Feature("GameSpeed", 			"Game-speed multiplicator", 	"Game simulation speed is multiplicated by the given value.", 
			1.0f, 	1.0f,			OLXBetaVersion(7), 	GIG_Advanced, 		0.1f, 	10.0f ),
	Feature("ForceScreenShaking", 	"Force screen shaking", 		"Screen shaking will be activated for everybody.", 
			true, 	true, 			Version(), 			GIG_Other, 							true ),
	Feature("SuicideDecreasesScore", "Suicide/teamkill decreases score", "The kills count will be descreased by one after a suicide or teamkill.", 
			false, 	false, 			Version(), 			GIG_Score, 							true ),
	Feature("TeamInjure", 			"Damage team members", 			"If disabled, your bullets and projectiles don't damage other team members.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("TeamHit", 				"Hit team members", 			"If disabled, your bullets and projectiles will fly through your team members.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("SelfInjure", 			"Damage yourself", 				"If disabled, your bullets and projectiles don't damage you.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("SelfHit", 				"Hit yourself", 				"If disabled, your bullets and projectiles will fly through yourself.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("AllowEmptyGames", 		"Allow empty games", 			"If enabled, games with one or zero worms will not quit.", 
			false, 	false, 			Version(), 			GIG_Other, 							true),
	Feature("CountTeamkills", 		"Count teamkills", 				"When killing player from your team increase your kills", 
			false, 	false, 			Version(), 			GIG_Score, 							true),
	Feature("HS_HideTime", 			"Hiding time", 					"AbsTime at the start of the game for hiders to hide", 
			20.0f, 	20.0f, 			Version(), 			GIG_HideAndSeek,	0.0f,	100.0f,	true ),
	Feature("HS_AlertTime", 		"Alert time", 					"When player discovered but escapes the time for which it's still visible", 
			10.0f, 	10.0f, 			Version(), 			GIG_HideAndSeek, 	0.1f, 	100.0f,	true ),
	Feature("HS_HiderVision",	 	"Hider vision", 				"How far hider can see, in pixels (whole screen = 320 px)", 
			175, 	175, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_HiderVisionThroughWalls", "Hider vision thorough walls", "How far hider can see through walls, in pixels (whole screen = 320 px)", 
			75, 	75, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVision",		"Seeker vision", 				"How far seeker can see, in pixels (whole screen = 320 px)", 
			125, 	125, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVisionThroughWalls", "Seeker vision thorough walls", "How far seeker can see through walls, in pixels (whole screen = 320 px)", 
			0, 		0, 				Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVisionAngle",	"Seeker vision angle",			"The angle of seeker vision (180 = half-circle, 360 = full circle)", 
			360, 	360, 			Version(), 			GIG_HideAndSeek, 	0, 		360, 	true ),
	Feature("NewNetEngine", 		"New net engine",		"New net engine - not working yet!", 
			false, 	false, 			OLXBetaVersion(9),	GIG_Advanced ),

	Feature::Unset()
};

Feature* featureByName(const std::string& name) {
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		if( stringcaseequal(f->get()->name, name) )
			return f->get();
	}
	return NULL;
}

FeatureSettings::FeatureSettings(bool alsoServerSide) {
	len = alsoServerSide ? featureArrayLen() : clientSideFeatureCount();
	if(len == 0) {
		settings = NULL;
		return;
	}
	settings = new ScriptVar_t[len];
	foreach( Feature*, f, Array(featureArray,len) ) {
		(*this)[f->get()] = f->get()->defaultValue;
	}
}

FeatureSettings::~FeatureSettings() {
	if(settings) delete[] settings;
}

FeatureSettings& FeatureSettings::operator=(const FeatureSettings& r) {
	if(settings) delete[] settings;

	len = r.len;
	if(len == 0) {
		settings = NULL;
		return *this;
	}
	settings = new ScriptVar_t[len];
	foreach( Feature*, f, Array(featureArray,len) ) {
		(*this)[f->get()] = r[f->get()];		
	}
	
	return *this;
}

ScriptVar_t FeatureSettings::hostGet(FeatureIndex i) {
	ScriptVar_t var = (*this)[i];
	Feature* f = &featureArray[i];
	if(f->getValueFct)
		var = (cServer->*(f->getValueFct))( var );
	else if(f->unsetIfOlderClients) {
		if(cServer->clientsConnected_less(f->minVersion))
			var = f->unsetValue;
	}
			
	return var;
}

bool FeatureSettings::olderClientsSupportSetting(Feature* f) {
	if(f->serverSideOnly) return true;
	return hostGet(f) == f->unsetValue;
}

void FeatureCompatibleSettingList::set(const std::string& name, const std::string& humanName, const ScriptVar_t& var, Feature::Type type) {
	foreach( Feature&, f, list ) {
		if(f->get().name == name) {
			f->get().humanName = humanName;
			f->get().var = var;
			f->get().type = type;
			return;
		}
	}
	push_back(name, humanName, var, type);
}
