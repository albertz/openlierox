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
#include <string>
#include "Color.h"
#include "SmartPointer.h"

struct profile_t;
class CBytestream;
class CWorm;
struct WormType;

struct WormJoinInfo {
	WormJoinInfo() : iTeam(0), m_type(NULL) {}
	void loadFromProfile(const SmartPointer<profile_t>& p);	
	void readInfo(CBytestream *bs);
	static bool	skipInfo(CBytestream *bs);
	void applyTo(CWorm* worm) const;
	
	std::string sName;
	int iTeam;
	WormType* m_type;
	std::string skinFilename;
	Color skinColor;
};

WormJoinInfo wormJoinInfoFromProfile(const SmartPointer<profile_t>& prof);
WormJoinInfo wormJoinInfoFromWorm(CWorm* w);

struct ClientConnectionRequestInfo {
	void initFromGame();
	std::vector<SmartPointer<profile_t> > worms;
};

#endif
