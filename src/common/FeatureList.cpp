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

using namespace std;

Feature featureArray[] = {
	Feature("ForceScreenShaking", "force screen shaking", "Screen shaking will be activated for everybody.", true, true, OLXBetaVersion(9) ),
	Feature::Unset()
};

FeatureSettings::FeatureSettings() {
	unsigned long len = featureArrayLen();
	if(len == 0) {
		settings = NULL;
		return;
	}
	settings = new CScriptableVars::ScriptVar_t[len];
	foreach( Feature*, f, Array(featureArray,len) ) {
		(*this)[f->get()].type = f->get()->valueType;
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
	settings = new CScriptableVars::ScriptVar_t[len];
	foreach( Feature*, f, Array(featureArray,len) ) {
		(*this)[f->get()] = r[f->get()];		
	}
	
	return *this;
}
