/*
	OpenLieroX

	reader for IpToCountry database
	
	code under LGPL
	by Albert Zeyer and Dark Charlie
*/


#ifdef _MSC_VER
#pragma warning(disable: 4786)  // WARNING: identifier XXX was truncated to 255 characters in the debug info
#pragma warning(disable: 4503)  // WARNING: decorated name length exceeded, name was truncated
#endif

#include <SDL.h>
#include <SDL_thread.h>
#include <iostream>
#include <map>

#include "Options.h"
#include "TSVar.h"
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

template<typename _handler, typename _PosType>
class DBEntryHandler {
public:
	_handler& handler;
	bool& breakSignal;
	ifstream* file;
	_PosType& filePos;
	DBEntryHandler(_handler& h, bool& b, ifstream* f, _PosType& fp)
		: handler(h), breakSignal(b), file(f), filePos(fp) {}
	
	inline bool operator()(const DBEntry& entry) {
		if(breakSignal) return false;
		filePos = file->tellg();
		return handler(entry);
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
	SDL_mutex*		mutex;
	SDL_Thread*		loader;
	size_t			fileSize;
	TSVar<size_t>	filePos;
	bool			dbReady; // false, if loaderThread is running
	bool			loaderBreakSignal;
	bool			justLoaded;
	
	IpToCountryData() : loader(NULL), fileSize(0), dbReady(true), loaderBreakSignal(false), justLoaded(false)
	{
		filePos = 0;
		mutex = SDL_CreateMutex();
	}
	
	~IpToCountryData() {
		breakLoaderThread();
		data.clear();
	}

	// Cancel the loading
	void breakLoaderThread()  {
		// Thread not created
		if (loader == NULL)
			return;

		if(!dbReady) {
			cout << "IpToCountryDB destroying thread: " << filename << " is still loading ..." << endl;
			loaderBreakSignal = true;
			while(!dbReady) { SDL_Delay(100); }
		}
		
		SDL_WaitThread(loader, NULL); // This destroys the thread

		// Cleanup
		loader = NULL;
		dbReady = true;
		justLoaded = false;
		loaderBreakSignal = false;
	}

	// HINT: this should only called from the mainthread
	// (or at least from the same thread where this DB is used),
	// because the handling of dbReady isn't threadsafe
	inline void loadFile(const std::string& fn) {
		if (filename == fn)
			return;

		// Destroy any previous loading
		breakLoaderThread();
		dbReady = false;
		justLoaded = false;
		loaderBreakSignal = false;
		
		filename = fn;
		data.clear();
		
		loader = SDL_CreateThread(loaderMain, this);
	}
	
	static int loaderMain(void* obj) {
		IpToCountryData* _this = (IpToCountryData*)obj;

		std::ifstream* file = OpenGameFileR(_this->filename);
		if(file == NULL) {
			cerr << "ERROR: cannot read " << _this->filename << endl;
			_this->dbReady = true;
			return 0; // TODO: other return? who got this?
		}
		file->seekg(0, std::ios::end);
		_this->fileSize = file->tellg();
		file->seekg(0, std::ios::beg);		
		
		cout << "IpToCountryDB: reading " << _this->filename << " ..." << endl;
		AddEntrysToDBData adder(_this->data);
		typedef DBEntryHandler<AddEntrysToDBData, TSVar<size_t> > DBEH;
		DBEH dbEntryHandler(adder, _this->loaderBreakSignal, file, _this->filePos);
		typedef CountryCsvReaderHandler<DBEH> CCRH;
		CCRH csvReaderHandler(dbEntryHandler);
		CsvReader<CCRH> csvReader(file, csvReaderHandler);
		if(csvReader.read()) {
			cout << "IpToCountryDB: reading finished, " << _this->data.size() << " entries" << endl;
		} else {
			cout << "IpToCountryDB: reading breaked, read " << _this->data.size() << " entries so far" << endl;
		}
		
		file->close();
		delete file;
		
		_this->dbReady = true;
		SDL_LockMutex(_this->mutex);
		_this->justLoaded = true;
		SDL_UnlockMutex(_this->mutex);
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

	
	inline int getProgress() {
		if(fileSize == 0) return 100;
		return (int)(((float)filePos / (float)fileSize) * 100.0f);
	}

	bool getJustLoaded()  {
		SDL_LockMutex(mutex);
		bool ret = justLoaded;
		justLoaded = false;
		SDL_UnlockMutex(mutex);
		return ret;
	}
	
};

DECLARE_INTERNDATA_CLASS(IpToCountryDB, IpToCountryData);


IpToCountryDB::IpToCountryDB(const std::string &dbfile) {
	init(); // needed INTERNCLASS-init function
	LoadDBFile(dbfile);
}

void IpToCountryDB::LoadDBFile(const std::string& dbfile) {
	if (tLXOptions->bUseIpToCountry)
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

	// User doesn't want to use this DB
	if (!tLXOptions->bUseIpToCountry)  {
		Result.Continent = "Unknown";
		Result.Country = "Unknown";
		Result.CountryShortcut = "UNK";
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

int IpToCountryDB::GetProgress()  {
	return IpToCountryDBData(this)->getProgress();
}

bool IpToCountryDB::Loaded()  {
	return IpToCountryDBData(this)->dbReady;
}

bool IpToCountryDB::JustLoaded()  {
	return IpToCountryDBData(this)->getJustLoaded();
}
