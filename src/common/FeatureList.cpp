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
	Feature("ForceScreenShaking", "force screen shaking", "Screen shaking will be activated for everybody.", true, true, OLXBetaVersion(9) ),
	Feature("SuicideDecreasesScore", "suicide decreases score", "The kills count will be descreased by one after a suicide.", false, false, OLXBetaVersion(9) ),
	Feature::Unset()
};

Feature* featureByName(const std::string& name) {
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		if( stringcaseequal(f->get()->name, name) )
			return f->get();
	}
	return NULL;
}

FeatureSettings::FeatureSettings() {
	unsigned long len = featureArrayLen();
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

	unsigned long len = featureArrayLen();
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
	return hostGet(f) == f->unsetValue;
}
