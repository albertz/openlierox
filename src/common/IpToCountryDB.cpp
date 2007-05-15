/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include "defs.h"
#include "StringUtils.h"
#include "FindFile.h"
#include "IpToCountryDB.h"


// TODO: enhance cache
// TODO: merge these two classes

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
	ipinfo_t result;
	
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
	
	ipinfo_t readAndReturnInfo() {
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


CIpToCountry::CIpToCountry(const std::string &dbfile)
{
	sFile = dbfile;
	tDatabase = OpenGameFileR(dbfile);
	tReader = new CountryCvsReader;
}

CIpToCountry::~CIpToCountry()
{
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

ipinfo_t CIpToCountry::GetInfoAboutIP(const std::string& Address)
{
	static ipinfo_t Result;
	Result.Continent = "";
	Result.Country = "";
	Result.CountryShortcut = "";

	static CountryCvsReader *reader = (CountryCvsReader *) tReader;

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
	ipcache_t::iterator it = tIPCache.find(Address);
	if (it != tIPCache.end())  {
		return it->second;
	}

	// Convert the IP to the numeric representation
	reader->myIP = from_string<int>(ip_e[0]) * 16777216 + from_string<int>(ip_e[1]) * 65536 + from_string<int>(ip_e[2]) * 256 + from_string<int>(ip_e[3]);

	// Open the database
	tDatabase->seekg(0);  // To the beginning
	reader->file = tDatabase;
	if(!tDatabase)  {
		Result.Continent = "outer space";
		Result.Country = "outer space";
		return Result;
	}

	Result = reader->readAndReturnInfo();
	if(Result.Country == "")  {
		Result.Continent = "unknown continent";
		Result.Country = "unknown country";
	}

	// Add to cache
	tIPCache[Address] = Result;
		
	
	return Result;	
}
