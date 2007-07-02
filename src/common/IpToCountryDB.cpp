/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <iostream>
#include <map>

#include "IpToCountryDB.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CsvReader.h"

#ifdef _MSC_VER  // MSVC 6 has problems with from_string<Uint32>
typedef unsigned int Ip;
#else
typedef Uint32 Ip;
#endif

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
class CountryCsvReaderHandler {
public:
	_handler& handler;
	
	bool finished_entry;
	DBEntry entry;
	
	CountryCsvReaderHandler(_handler& h)
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
			ucfirst(entry.Info.Country);
			// Small hack, Australia is considered as Asia by the database
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


using namespace std;

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
	SDL_Thread*		loader;
	bool			dbReady;

	IpToCountryData() : loader(NULL), dbReady(true) {}
	
	~IpToCountryData() {
		if(!dbReady) {
			cout << "IpToCountryDB destroying: " << filename << " is still loading ..." << endl;
			while(!dbReady) { SDL_Delay(100); }
		}
		// SDL_WaitThread(thread, NULL);
	
#ifdef _MSC_VER
#ifndef _DEBUG
		// Probably because of some bug in MSVC, data.clear() in release mode is incredibly slow (30 secs)
		// This does the same but faster
		// FIXME: fix this asap
		// TODO: this CANNOT be the reason for the slowness, so don't add some hack which works with current code because of randomness; search the real bug instead
/*		while (data.size())
			data.erase(data.begin()); */
#endif
#endif
		data.clear();
	}

	// HINT: this should only called from the mainthread
	// (or at least from the same thread where this DB is used),
	// because the handling of dbReady isn't threadsafe
	inline void loadFile(const std::string& fn) {
		if(!dbReady) {
			cout << "IpToCountryDB loadFile: other file " << filename << " is still loading ..." << endl;
			while(!dbReady) { SDL_Delay(100); }
		}
		dbReady = false;
		
		filename = fn;
		data.clear();
		
		loader = SDL_CreateThread(loaderMain, this);
	}
	
	static int loaderMain(void* obj) {
		IpToCountryData* _this = (IpToCountryData*)obj;

		std::ifstream* f = OpenGameFileR(_this->filename);
		if(f == NULL) {
			cerr << "ERROR: cannot read " << _this->filename << endl;
			_this->dbReady = true;
			return 0; // TODO: other return? who got this?
		}
		f->seekg(0);		
		
		cout << "IpToCountryDB: reading " << _this->filename << " ..." << endl;
		AddEntrysToDBData adder(_this->data);
		CountryCsvReaderHandler<AddEntrysToDBData> csvReaderHandler(adder);
		CsvReader<CountryCsvReaderHandler<AddEntrysToDBData> > csvReader(f, csvReaderHandler);
		csvReader.read();
		cout << "IpToCountryDB: reading finished, " << _this->data.size() << " entries" << endl;
		
		f->close();
		delete f;
		
		_this->dbReady = true;
		return 0;
	}
	
	inline const DBEntry* getEntry(Ip ip) {
		if(!dbReady) {
			cout << "IpToCountryDB getEntry: " << filename << " is still loading ..." << endl;
			while(!dbReady) { SDL_Delay(100); }
		}

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
	IpToCountryDBData(this)->loadFile( dbfile );
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
