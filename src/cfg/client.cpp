/*
 *  client.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under LGPL
 *
 */

#include "cfg/client.h"

Feature clientSettings[ClientSettingsArrayLen] = {
	Feature( "Raytracing", "Raytracing", "Raytracing enabled", false, false, Version(), GIG_Advanced, ALT_Dev ),
};

