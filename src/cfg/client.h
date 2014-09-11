/*
 *  client.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under LGPL
 *
 */

/*
 These Client Settings are for all pure client settings. 
 Most of the stuff in our Options class should be moved here sooner or later.
 
 It uses the same Feature/FeatureSettings structures as for the other game settings.
 */

#ifndef __OLX_CFG_CLIENT_H__
#define __OLX_CFG_CLIENT_H__

#include "FeatureList.h"

enum ClientSettingIndex {
	__CS_BOTTOM
};

static const size_t ClientSettingsArrayLen = __CS_BOTTOM;
extern Feature ClientSettingsArray[ClientSettingsArrayLen];

typedef _FeatureSettings<size_t, ClientSettingsArray, ClientSettingsArrayLen> ClientSettings;
extern ClientSettings clientSettings;

#endif
