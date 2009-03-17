/*
	OpenLieroX

	reader for IpToCountry database
	
	code under LGPL
	by Albert Zeyer and Dark Charlie
*/

#ifndef __IPTOCOUNTRY_H__
#define	__IPTOCOUNTRY_H__

#include <string>
#include "SmartPointer.h"
#include "InternDataClass.h"

struct SDL_Surface;

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
	SmartPointer<SDL_Surface> GetCountryFlag(const std::string& shortcut);
	int	GetProgress();
	bool Loaded();
INTERNDATA_CLASS_END

extern  IpToCountryDB	*tIpToCountryDB;

#endif
