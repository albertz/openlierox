/*
 *  client.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_CFG_CLIENT_H__
#define __OLX_CFG_CLIENT_H__

#include "FeatureList.h"

enum ClientSettingIndex {
	CS_Raytracing,
	
	__CS_BOTTOM
};

static const size_t ClientSettingsArrayLen = __CS_BOTTOM;

extern Feature clientSettings[];

#endif
