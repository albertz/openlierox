/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __IPTOCOUNTRY_H__
#define	__IPTOCOUNTRY_H__

#include <string>
#include "Utils.h"

struct IpInfo {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
};

INTERNDATA_CLASS_BEGIN(IpToCountryDB)
public:
	IpToCountryDB(const std::string& dbfile);
	void LoadDBFile(const std::string& dbfile);
	IpInfo GetInfoAboutIP(const std::string& Address);
INTERNDATA_CLASS_END

#endif
