/*
	OpenLieroX

	reader for IpToCountry database
	
	code under LGPL
	by Albert Zeyer and Dark Charlie
*/

#ifndef __IPTOCOUNTRY_H__
#define	__IPTOCOUNTRY_H__

#include <string>
#define _WSPIAPI_H_
#define _WS2TCPIP_H_
#define _WINSOCK2API_
#include <GeoIP.h>
#include "SmartPointer.h"
#include "InternDataClass.h"

struct SDL_Surface;
class GeoIPDatabase;

struct IpInfo {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
	std::string		City;
	std::string		Region;
};

INTERNDATA_CLASS_BEGIN(IpToCountryDB)
private:
	GeoIPDatabase *m_database;
public:
	IpToCountryDB(const std::string& dbfile);
	void LoadDBFile(const std::string& dbfile);
	IpInfo GetInfoAboutIP(const std::string& Address);
	SmartPointer<SDL_Surface> GetCountryFlag(const std::string& shortcut);
	int	GetProgress() { return 100; }
	bool Loaded()  { return true; }
INTERNDATA_CLASS_END

extern  IpToCountryDB	*tIpToCountryDB;

#endif
