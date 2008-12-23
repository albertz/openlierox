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


using namespace std;

Feature featureArray[] = { Feature::Unset() };

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
