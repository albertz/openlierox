/*
 *  Settings.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#include "Settings.h"
#include "Options.h"
#include "CServer.h"
#include "IniReader.h"

Settings gameSettings;
FeatureSettingsLayer gamePresetSettings;


void FeatureSettingsLayer::copyTo(FeatureSettings& s) const {
	for(size_t i = 0; i < FeatureArrayLen; ++i)
		if(isSet[i])
			s[(FeatureIndex)i] = (*this)[(FeatureIndex)i];
}

void Settings::layersInitStandard() {
	layersClear();
	layers.push_back(&gamePresetSettings);
	if(tLXOptions)
		layers.push_back(&tLXOptions->customSettings);
}


ScriptVar_t Settings::hostGet(FeatureIndex i) {
	ScriptVar_t var = (*this)[i];
	Feature* f = &featureArray[i];
	if(f->getValueFct)
		var = (cServer->*(f->getValueFct))( var );
	
	return var;
}

bool Settings::olderClientsSupportSetting(Feature* f) {
	if( f->optionalForClient ) return true;
	return hostGet(f) == f->unsetValue;
}


bool FeatureSettingsLayer::loadFromConfig(const std::string& cfgfile, bool reset, std::map<std::string, std::string>* unknown) {
	if(reset) makeSet(false);
	
	IniReader ini(cfgfile);
	if(!ini.Parse()) return false;
	
	IniReader::Section& section = ini.m_sections["GameInfo"];
	for(IniReader::Section::iterator i = section.begin(); i != section.end(); ++i) {
		Feature* f = featureByName(i->first);
		if(f) {
			FeatureIndex fi = featureArrayIndex(f);
			isSet[fi] = true;
			if(!(*this)[fi].fromString(i->second))
				notes << "loadFromConfig " << cfgfile << ": cannot understand: " << i->first << " = " << i->second << endl;
		}
		else {
			if(unknown)
				(*unknown)[i->first] = i->second;
		}
	}
	
	return true;
}
