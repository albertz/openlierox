//
//  EngineSettings.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 03.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_EngineSettings_h
#define OpenLieroX_EngineSettings_h

#include "Debug.h"
#include "FeatureList.h"
#include "Game.h"
#include "util/macros.h"
#include "Settings.h"

/*
  EngineSettings is used by the client (CClient) to store the current
  game settings. It is set by the server over network.
  In case we are the server, it is all wrapped to gameSettings
  and writing to it is disallowed (because server-code should directly
  modify gameSettings).
  */

struct EngineSettings {
private:
	ScriptVar_t settings[FeatureArrayLen];
public:
	EngineSettings() {
		for(size_t i = 0; i < FeatureArrayLen; ++i)
			settings[i] = featureArray[i].unsetValue;
	}

	const ScriptVar_t& operator[](FeatureIndex i) const {
		if(game.isClient()) return settings[i];
		else return gameSettings[i];
	}
	const ScriptVar_t& operator[](Feature* f) const { return (*this)[FeatureIndex(f - &featureArray[0])]; }

	struct OverwriteVarWrapper {
		EngineSettings& s;
		FeatureIndex i;
		OverwriteVarWrapper(EngineSettings& _s, FeatureIndex _i) : s(_s), i(_i) {}
		template<typename T> OverwriteVarWrapper& operator=(const T& val) {
			if(game.isClient())
				s.settings[i] = val;
			else {
				if(gameSettings[i] != ScriptVar_t(val))
					errors << "EngineSettings: overwrite "
					<< featureArray[i].name << " mismatch. old="
					<< gameSettings[i].toString() << ", new=" << ScriptVar_t(val).toString() << endl;
			}
			return *this;
		}
	};
	struct OverwriteWrapper {
		OverwriteVarWrapper operator[](FeatureIndex i) {
			EngineSettings& s = *__OLX_BASETHIS(EngineSettings, overwrite);
			return OverwriteVarWrapper(s, i);
		}
		OverwriteVarWrapper operator[](Feature* f) { return (*this)[FeatureIndex(f - &featureArray[0])]; }
	};
	OverwriteWrapper overwrite;

	ScriptVar_t& write(FeatureIndex i) {
		assert(game.isClient());
		return settings[i];
	}
	ScriptVar_t& write(Feature* f) { return write(FeatureIndex(f - &featureArray[0])); }

};

#endif
