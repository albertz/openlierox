/*
	OpenLieroX

	reader for IpToCountry database

	code under LGPL
	by Albert Zeyer and Dark Charlie
*/


#include "IpToCountryDB.h"
#include <GeoIPCity.h>
#include "GfxPrimitives.h"
#include "Unicode.h"

IpToCountryDB::IpToCountryDB(const std::string& dbfile) : m_geoIP(NULL) { LoadDBFile(dbfile); }

void IpToCountryDB::LoadDBFile(const std::string& dbfile)
{
	if (m_geoIP)  {
		GeoIP_delete(m_geoIP);
		m_geoIP = NULL;
	}

	// Open
	m_file = dbfile;
	m_geoIP = GeoIP_open(m_file.c_str(), GEOIP_STANDARD);
	GeoIP_set_charset(m_geoIP, GEOIP_CHARSET_UTF8);
	if (!m_geoIP)  {
		errors << "Could not open Geo IP to Country database" << endl;
		return;
	}
}

IpInfo IpToCountryDB::GetInfoAboutIP(const std::string& address)
{
	IpInfo res = {"Unknown", "Unknown", "UNK", "Unknown"};
	if (!m_geoIP)
		return res;

	if (address.find("127.0.0.1") == 0)  {
		res.Country = "Home";
		res.City = "Home City";
		res.Region = "Home Region";
		return res;
	}

	std::string pure_addr = address;
	size_t pos = pure_addr.find(':');
	if (pos != std::string::npos)
		pure_addr.erase(pos);

	GeoIPRecord *rec = GeoIP_record_by_addr(m_geoIP, pure_addr.c_str());
	if (rec == NULL || !rec->country_name || !rec->country_name[0])
		return res;

	res.Country = rec->country_name;
	res.Continent = rec->continent_code;
	res.CountryShortcut = rec->country_code;
	if (rec->city)
		res.City = rec->city;
	if (rec->region)
		res.Region = rec->region;

	GeoIPRecord_delete(rec);

	return res;
}

SmartPointer<SDL_Surface> IpToCountryDB::GetCountryFlag(const std::string& shortcut)
{
	return LoadGameImage("data/flags/" + shortcut + ".png", true);
}

IpToCountryDB::~IpToCountryDB()
{
	if (m_geoIP)  {
		GeoIP_delete(m_geoIP);
		m_geoIP = NULL;
	}
}
