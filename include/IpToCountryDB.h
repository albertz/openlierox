/*
	OpenLieroX

	reader for IpToCountry database
	
	code under LGPL
	by Albert Zeyer and Dark Charlie
*/

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
	int	GetProgress();
	bool Loaded();
	bool JustLoaded();
INTERNDATA_CLASS_END

#endif
