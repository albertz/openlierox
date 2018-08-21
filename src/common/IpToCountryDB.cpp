/*
	OpenLieroX

	reader for IpToCountry database

	code under LGPL
	by Albert Zeyer and Dark Charlie
*/


#include "IpToCountryDB.h"
#include "GfxPrimitives.h"
#include "Unicode.h"
#include "GeoIPDatabase.h"
#include "Networking.h"

const char *IP_TO_COUNTRY_FILE = "GeoIP.dat";

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
	IpInfo res;
	if (!m_database || !m_database->loaded())
		return res;

	// Home
	if (address.find("127.0.0.1") == 0)  {
		res.countryName = "Home";
		res.city = "Home City";
		res.region = "Home Region";
		res.continent = "Earth";
		return res;
	}

	// LAN
	if (address.find("10.0.") == 0 || address.find("192.168.") == 0)  {
		res.countryName = "Local Area Network";
		res.city = "Local City";
		res.region = "Local Area Network";
		return res;
	}

	// IPv6
	if (IsNetAddrV6(address))  {
		res.countryName = "IPv6 Network";
		res.countryCode = "v6";
		res.city = "IPv6 Network";
		res.region = "IPv6 Network";
		return res;
	}

	GeoRecord rec = m_database->lookup(address);
	if (rec.countryCode == "--" || rec.countryCode == "UN")  // Unknown
		return res;

	res = rec;

	return res;
}

SmartPointer<SDL_Surface> IpToCountryDB::GetCountryFlag(const std::string& shortcut)
{
	return LoadGameImage("data/flags/" + shortcut + ".png", true);
}

////////////////////
// Calculate distance (in kilometers) between the two places
float IpToCountryDB::GetDistance(const IpInfo &place1, const IpInfo &place2)
{
	// Use Haversine formula
	static const double earth_radius = 6371.0;
	double dlat = DEG2RAD(place2.latitude - place1.latitude);
	double dlong = DEG2RAD(place2.longitude - place1.longitude);
	double a = SQR(sin(dlat/2)) + cos(DEG2RAD(place1.latitude)) * cos(DEG2RAD(place2.latitude)) * SQR(sin(dlong/2));
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return (float)(earth_radius * c);
}

IpToCountryDB::~IpToCountryDB()
{
	if (m_database)  {
		delete m_database;
		m_database = NULL;
	}
}
