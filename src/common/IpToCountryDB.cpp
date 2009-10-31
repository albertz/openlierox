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
#include "GeoIPDatabase.h"

char *IP_TO_COUNTRY_FILE = "GeoIP.dat";

IpToCountryDB::IpToCountryDB(const std::string& dbfile) : m_database(NULL) { LoadDBFile(dbfile); }

void IpToCountryDB::LoadDBFile(const std::string& dbfile)
{
	if (m_database)  {
		delete m_database;
		m_database = NULL;
	}

	// Test: database
	m_database = new GeoIPDatabase();
	if (!m_database->load(dbfile))
		errors << "Error when loading GeoIP database" << endl;
}

IpInfo IpToCountryDB::GetInfoAboutIP(const std::string& address)
{
	IpInfo res = {"Unknown", "Unknown", "UNK", "Unknown"};
	if (!m_database || !m_database->loaded())
		return res;

	// Home
	if (address.find("127.0.0.1") == 0)  {
		res.Country = "Home";
		res.City = "Home City";
		res.Region = "Home Region";
		return res;
	}

	// LAN
	if (address.find("10.0.") == 0 || address.find("192.168.") == 0)  {
		res.Country = "Local Area Network";
		res.City = "Local City";
		res.Region = "Local Area Network";
		return res;
	}

	GeoRecord rec = m_database->lookup(address);
	if (rec.countryCode == "--" || rec.countryCode == "UN")  // Unknown
		return res;

	res.Country = rec.countryName;
	res.Continent = rec.continentCode;
	res.CountryShortcut = rec.countryCode;
	res.City = rec.city;
	res.Region = rec.region;

	return res;
}

SmartPointer<SDL_Surface> IpToCountryDB::GetCountryFlag(const std::string& shortcut)
{
	return LoadGameImage("data/flags/" + shortcut + ".png", true);
}

IpToCountryDB::~IpToCountryDB()
{
	if (m_database)  {
		delete m_database;
		m_database = NULL;
	}
}
