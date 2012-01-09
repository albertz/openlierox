//
//  ClientConnectionRequestInfo.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 08.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_ClientConnectionRequestInfo_h
#define OpenLieroX_ClientConnectionRequestInfo_h

#include <vector>
#include "ProfileSystem.h"

struct ClientConnectionRequestInfo {
	std::vector<profile_t> worms;
};

#endif
