/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include <iostream>
#include <map>

#include "IpToCountryDB.h"
#include "StringUtils.h"
#include "CvsReader.h"
#include "FindFile.h"


typedef Uint32 Ip;

struct DBEntry {
	Ip			RangeFrom;
	Ip			RangeTo;
	IpInfo		Info;
};

// key-value is ending-ip (RangeTo) of representing ip-range
typedef std::map<Ip,DBEntry> DBData;


/*
	_handler has to be a functor, which is compatible to:
	
	(called after we have a new entry)
		bool _handler(const DBEntry& dbentry);
	whereby
		return: if false, it will break
*/
template<typename _handler>
class CountryCvsReaderHandler {
public:
	_handler& handler;
	
	bool finished_entry;
	DBEntry entry;
	
	CountryCvsReaderHandler(_handler& h)
		: handler(h), finished_entry(false) {}

	inline bool operator()(int tindex, const std::string& token) {
		switch(tindex) {
		case 0:
			entry.RangeFrom = from_string<Ip>(token);
			return true;
			
		case 1:
			entry.RangeTo = from_string<Ip>(token);
			return true;
		
		case 2:
			if (token == "IANA")
				entry.Info.Continent = "Local Network";
			else if (token == "ARIN")
				entry.Info.Continent = "North America";
			else if (token == "LACNIC")
				entry.Info.Continent = "South America";
			else if (token == "AFRINIC")
				entry.Info.Continent = "Africa";
			else if (token == "RIPE")
				entry.Info.Continent = "Europe";
			else if (token == "APNIC")
				entry.Info.Continent = "Asia";
			else
				entry.Info.Continent = token;
			return true;
			
		case 3:
		case 5:
			// ignore
			return true;

		case 4: 
			entry.Info.CountryShortcut = token;
			return true;
			
		case 6:
			entry.Info.Country = token;
			//ucfirst(result.Country); // TODO: realy needed?
			// Small hack, Australia is considered as asia by the database
			if(entry.Info.Country == "Australia")
				entry.Info.Continent = "Australia";
				
			finished_entry = true;
			return false;
		
		default:
			return false;
		}
	}
	
	inline bool operator()() {
		if(finished_entry) {
			if(!handler(entry)) return false;		
			finished_entry = false;
		}
		
		return true;
	}


};


class AddEntrysToDBData {
public:
	DBData& data;
	AddEntrysToDBData(DBData& d) : data(d) {}
	
	inline bool operator()(const DBEntry& entry) {
		data[entry.RangeTo] = entry;
		return true;
	}
};

class IpToCountryData {
public:
	std::string		filename;
	DBData			data;
	
	inline void loadFile(const std::string& fn) {
		filename = fn;		
		data.clear();
		
		std::ifstream* f = OpenGameFileR(fn);
		if(f == NULL) {
			std::cerr << "ERROR: cannot read " << fn << std::endl;
			return;
		}
		
		AddEntrysToDBData adder(data);
		CountryCvsReaderHandler<AddEntrysToDBData> cvsReaderHandler(adder);
		CvsReader<CountryCvsReaderHandler<AddEntrysToDBData> > cvsReader(f, cvsReaderHandler);
		cvsReader.read();
		
		f->close();
		delete f;
	}
	
	inline const DBEntry* getEntry(Ip ip) {
		DBData::const_iterator it = data.lower_bound(ip);
		if(it != data.end() && it->second.RangeFrom <= ip) // in range?
			return &it->second;
		else
			return NULL;
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

	// Convert the IP to the numeric representation
	Ip ip = from_string<Ip>(ip_e[0]) * 16777216 + from_string<Ip>(ip_e[1]) * 65536 + from_string<Ip>(ip_e[2]) * 256 + from_string<Ip>(ip_e[3]);

	const DBEntry* entry = IpToCountryDBData(this)->getEntry(ip);
	if(entry == NULL) {
		Result.Continent = "unknown continent";
		Result.Country = "unknown country";	
	} else {
		Result = entry->Info;
	}
	
	return Result;	
}
