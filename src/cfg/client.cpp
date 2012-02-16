/*
 *  client.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under LGPL
 *
 */

#include "cfg/client.h"

Feature ClientSettingsArray[ClientSettingsArrayLen] = {
	Feature( -1, "Video.Raytracing", "Raytracing", "Raytracing enabled. It's an advanced gfx engine for OLX.", false, false, OLXBetaVersion(0,59,9), GIG_Advanced, ALT_Dev ),
};

ClientSettings clientSettings;
