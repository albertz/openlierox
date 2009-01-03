/*
 *  FeatureList.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.12.08.
 *  code under LGPL
 *
 */

#include <iostream>
#include "FeatureList.h"
#include "Version.h"
#include "CServer.h"

using namespace std;

// WARNING: Keep this always synchronised with FeatureIndex!
Feature featureArray[] = {
	Feature("GameSpeed", "Game-speed multiplicator", "Game simulation speed is multiplicated by the given value.", 1.0f, 1.0f, OLXBetaVersion(7), 0.1f, 5.0f ),
	Feature("ForceScreenShaking", "Force screen shaking", "Screen shaking will be activated for everybody.", true, true, OLXBetaVersion(9), true ),
	Feature("SuicideDecreasesScore", "Suicide/teamkill decreases score", "The kills count will be descreased by one after a suicide or teamkill.", false, false, OLXBetaVersion(9) ),
	Feature("TeamInjure", "Damage team members", "If disabled, your bullets and projectiles don't damage other team members.", true, true, OLXBetaVersion(9) ),
	Feature("TeamHit", "Hit team members", "If disabled, your bullets and projectiles will fly through your team members.", true, true, OLXBetaVersion(9) ),
	Feature("SelfInjure", "Damage yourself", "If disabled, your bullets and projectiles don't damage you.", true, true, OLXBetaVersion(9) ),
	Feature("SelfHit", "Hit yourself", "If disabled, your bullets and projectiles will fly through yourself.", true, true, OLXBetaVersion(9) ),
	
	Feature("AllowEmptyGames", "Allow empty games", "If enabled, games with one or zero worms will not quit.", false, false, OLXBetaVersion(9), true),

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
