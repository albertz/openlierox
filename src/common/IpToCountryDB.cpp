/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include <map>
#include <vector>

#include "StringUtils.h"
#include "FindFile.h"
#include "IpToCountryDB.h"

// TODO: enhance cache, use hashmap
// TODO: merge these two classes // NO, don't do this, this makes no sence

class CountryCvsReader {
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
	~IpToCountryData() {
		if (tDatabase)  {
			tDatabase->close();
			delete tDatabase;
		}
		tDatabase = NULL;
		if (tReader)
			delete ((CountryCvsReader *)tReader);
		tReader = NULL;
		tIPCache.clear();
	}
	
	std::string		sFile;
	ipcache_t		tIPCache;
	std::ifstream	*tDatabase;
	CountryCvsReader	*tReader;
};

DECLARE_INTERNDATA_CLASS(IpToCountryDB, IpToCountryData);


IpToCountryDB::IpToCountryDB(const std::string &dbfile) {
	init(); // needed INTERNCLASS-init function
	IpToCountryDBData(this)->sFile = dbfile;
	IpToCountryDBData(this)->tDatabase = OpenGameFileR(dbfile);
	IpToCountryDBData(this)->tReader = new CountryCvsReader;
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
