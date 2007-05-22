/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

/*#include <map>
#include <vector>*/

#include "StringUtils.h"
#include "FindFile.h"
#include "IpToCountryDB.h"

// TODO: enhance cache, use hashmap
// TODO: merge these two classes // NO, don't do this, this makes no sence

/*class CountryCvsReader {
public:
	std::ifstream* file;
	Uint32 myIP;
	
	// reading states
	bool inquote;
	bool waitforkomma;
	bool ignoreline;
	
	// collected data
	int tindex;
	std::string token;	
	
	bool hasresult;
	IpInfo result;
	
	CountryCvsReader() : inquote(false), waitforkomma(false), ignoreline(false), tindex(0), hasresult(false) {}
	
	void endToken() {
		ignoreline = false;
		waitforkomma = false;
		inquote = false;	
		
		switch(tindex) {
		case 0:
			// range check
			ignoreline = !(myIP >= from_string<uint>(token));
			break;
			
		case 1:
			// range check
			ignoreline = !(myIP <= from_string<uint>(token));
			break;
		
		case 2:
			if (token == "IANA")
				result.Continent = "Local Network";
			else if (token == "ARIN")
				result.Continent = "North America";
			else if (token == "LACNIC")
				result.Continent = "South America";
			else if (token == "AFRINIC")
				result.Continent = "Africa";
			else if (token == "RIPE")
				result.Continent = "Europe";
			else if (token == "APNIC")
				result.Continent = "Asia";
			else
				result.Continent = "Outer space";
			break;
		case 3:
		case 5:
			// ignore
			break;

		case 4: 
			result.CountryShortcut = token;
			break;
			
		case 6:
			result.Country = token;
			ucfirst(result.Country);
			// Small hack, Australia is considered as asia by the database
			if (result.Country == "Australia")
				result.Continent = "Australia";
			hasresult = true;
			ignoreline = true;
			break;
		
		default:
			ignoreline = true;
		}
		
		token = "";
		tindex++;
	}
	
	IpInfo readAndReturnInfo() {
		char nextch;
		while(!file->eof() && !hasresult) {
			file->get(nextch);
			switch(nextch) {
			case 0:
				break;
				
			case ' ':
			case 9: // TAB
				if(ignoreline) break;
				if(waitforkomma) break;
				if(inquote) token += nextch;
				break;
							
			case '\"':
				if(ignoreline) break;
				if(waitforkomma) break;
				if(inquote) {
					endToken();
					waitforkomma = true;
				} else {
					inquote = true;
				}
				break;
				
			case 10: // LF (newline)
			case 13: // CR
				if(!ignoreline && !waitforkomma) endToken();
				ignoreline = false;
				waitforkomma = false;
				inquote = false;
				tindex = 0;
				break;
				
			case '#': // comment marking
				if(ignoreline) break;
				if(inquote)
					token += nextch;
				else {
					if(!waitforkomma) endToken();
					ignoreline = true;
				}
				break;
				
			case ',': // new token marking
				if(ignoreline) break;				
				if(waitforkomma) {
					waitforkomma = false;
					break;
				}
				if(inquote)
					token += nextch;
				else
					endToken();
				break;
				
			default:
				if(!ignoreline && !waitforkomma) token += nextch;
			}
		}
		
		return result;
	}


};

// TODO: use hashmap instead of map
typedef std::map<std::string, IpInfo> ipcache_t;

class IpToCountryData {
public:
	std::string		sFile;
	ipcache_t		tIPCache;
	std::ifstream	*tDatabase;
	CountryCvsReader	*tReader;
	
	void loadFile(const std::string& f) {
		sFile = f;
		if(tDatabase) {
			tDatabase->close();
			delete tDatabase;
		}
		tDatabase = OpenGameFileR(f);		
	}
	
	IpToCountryData() {
		tDatabase = NULL;
		tReader = new CountryCvsReader;
	}
	
	~IpToCountryData() {
		if(tDatabase)  {
			tDatabase->close();
			delete tDatabase;
		}
		tDatabase = NULL;
		if(tReader)
			delete tReader;
		tReader = NULL;
		tIPCache.clear();
	}
	
};

DECLARE_INTERNDATA_CLASS(IpToCountryDB, IpToCountryData);


IpToCountryDB::IpToCountryDB(const std::string &dbfile) {
	init(); // needed INTERNCLASS-init function
	LoadDBFile(dbfile);
}

void IpToCountryDB::LoadDBFile(const std::string& dbfile) {
	IpToCountryDBData(this)->loadFile(dbfile);
}


IpInfo IpToCountryDB::GetInfoAboutIP(const std::string& Address)
{
	static IpInfo Result;
	Result.Continent = "";
	Result.Country = "";
	Result.CountryShortcut = "";

	CountryCvsReader* reader = IpToCountryDBData(this)->tReader;

	// Don't check against local IP
	if (Address.find("127.0.0.1") != std::string::npos) {
		Result.Continent = "Home";
		Result.Country = "Home";
		return Result;
	}

	const std::vector<std::string>& ip_e = explode(Address,".");
	if (ip_e.size() != 4)  {
		Result.Continent = "Hackerland";
		Result.Country = "Hackerland";
		return Result;
	}

	// If we've already read
	ipcache_t::iterator it = IpToCountryDBData(this)->tIPCache.find(Address);
	if (it != IpToCountryDBData(this)->tIPCache.end())  {
		return it->second;
	}

	// Convert the IP to the numeric representation
	reader->myIP = from_string<int>(ip_e[0]) * 16777216 + from_string<int>(ip_e[1]) * 65536 + from_string<int>(ip_e[2]) * 256 + from_string<int>(ip_e[3]);

	// Open the database
	IpToCountryDBData(this)->tDatabase->seekg(0);  // To the beginning
	reader->file = IpToCountryDBData(this)->tDatabase;
	if(!reader->file)  {
		Result.Continent = "invisibland";
		Result.Country = "invisibland";
		return Result;
	}

	Result = reader->readAndReturnInfo();
	if(Result.Country == "")  {
		Result.Continent = "unknown continent";
		Result.Country = "unknown country";
	}

	// Add to cache
	IpToCountryDBData(this)->tIPCache[Address] = Result;
		
	
	return Result;	
}
*/

IpToCountryDB::IpToCountryDB(const std::string& dbfile)
{
	fDatabase.open(dbfile.c_str());
}

IpToCountryDB::~IpToCountryDB()
{
	fDatabase.close();
}

IpInfo IpToCountryDB::GetInfoAboutIP(const std::string& Address)
{
	static IpInfo Result;
	Result.Continent = "";
	Result.Country = "";
	Result.CountryShortcut = "";

	// Local IP
	if (Address.find("127.0.0.1") != std::string::npos)  {
		Result.Continent = "Home";
		Result.Country = "Home";
		return Result;
	}

	// Convert the IP
	const std::vector<std::string>& ip_e = explode(Address,".");
	if (ip_e.size() != 4)  {
		Result.Continent = "Hackerland";
		Result.Country = "Hackerland";
		return Result;
	}
	unsigned int Ip = from_string<int>(ip_e[0]) * 16777216 + from_string<int>(ip_e[1]) * 65536 + from_string<int>(ip_e[2]) * 256 + from_string<int>(ip_e[3]);

	// Check the IP cache
	IpCache::iterator it1 = tIpCache.find(Ip);
	if (it1 != tIpCache.end())  {
		return it1->second;
	}

	// Check the DB cache
	DBCache::iterator it2 = tDBCache.begin();
	for (;it2 != tDBCache.end();it2++)  {
		if (it2->RangeFrom <= Ip && it2->RangeTo >= Ip)  {
			return it2->Info;
		}
	}

	// Check
	if (!fDatabase.is_open())  {
		Result.Continent = "invisibland";
		Result.Country = "invisibland";
		return Result;
	}

	char firstchar;
	std::string from,to,reg,ctr,country;
	static CacheItem cach;
	while (!fDatabase.eof())  {

		// Check for comment
		fDatabase.read(&firstchar,1);
		if (firstchar == '#' || firstchar == ' ' || firstchar == '\r' || firstchar == '\n')  {
			fDatabase.ignore(256,'\n');
			continue;
		}

		std::getline(fDatabase,from,',');  // From
		std::getline(fDatabase,to,',');    // To
		std::getline(fDatabase,reg,',');   // Continent
		fDatabase.ignore(16,',');		   // Added date
		std::getline(fDatabase,ctr,',');   // Short country code
		fDatabase.ignore(8,',');		   // Long country code
		std::getline(fDatabase,country);   // Country name

		// Remove quotes
		from.erase(from.size()-1); // At the beginning " is skipped  by checking for comment
		to.erase(0,1); to.erase(to.size()-1);
		reg.erase(0,1); reg.erase(reg.size()-1);
		ctr.erase(0,1); ctr.erase(ctr.size()-1);
		country.erase(0,1); country.erase(country.size()-1);

		// Adjust the name
		ucfirst(country);

		// Cache the info
		cach.RangeFrom = from_string<uint>(from);
		cach.RangeTo = from_string<uint>(to);
		cach.Info.Country = country;
		cach.Info.CountryShortcut = ctr;

		// Continent
		if (country == "Australia")  // Small hack, Australia is considered as asia by the DB
			cach.Info.Continent = "Australia";
		else  {
			if (reg == "IANA")
				cach.Info.Continent = "Local Network";
			else if (reg == "ARIN")
				cach.Info.Continent = "North America";
			else if (reg == "LACNIC")
				cach.Info.Continent = "South America";
			else if (reg == "AFRINIC")
				cach.Info.Continent = "Africa";
			else if (reg == "RIPE")
				cach.Info.Continent = "Europe";
			else if (reg == "APNIC")
				cach.Info.Continent = "Asia";
			else
				cach.Info.Continent = "Outer space";
		}

		// Add to cache
		tDBCache.push_back(cach);

		// Check the range
		if (cach.RangeFrom <= Ip && cach.RangeTo >= Ip)  {
			return cach.Info;
		}
	}

	// Not found
	Result.Country = "unknown country";
	Result.Continent = "unknown continent";
	return Result;
}