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
#include "GeoIPDatabase.h"

struct SDL_Surface;

typedef GeoRecord IpInfo;

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

extern char *IP_TO_COUNTRY_FILE;
extern  IpToCountryDB	*tIpToCountryDB;

#endif
