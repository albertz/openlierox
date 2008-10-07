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
#include "debug.h"
#include "TSVar.h"
#include "IpToCountryDB.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CsvReader.h"
#include "Timer.h"
#include "InputEvents.h"


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
typedef std::vector<DBEntry> DBData;


// TODO: remove all printf usage here

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
	size_t line;

	CountryCsvReaderHandler(_handler& h)
		: handler(h), line(0) {}

	void setHandler(_handler& h) { handler = h; }

	bool operator()(const std::list<std::string>& entries) {
		using namespace std;
		if (entries.size() < 7)  {
			cout << "IpToCountry loader warning: number of entries is less than 7, ignoring the line " << line << endl;
			if (entries.size())  {
				printf("\"%s\"", entries.begin()->c_str());
				std::list<std::string>::const_iterator it = entries.begin(); it++;
				for (; it != entries.end(); it++)  {
					printf(",\"%s\"", it->c_str());
				}
			}
			printf("\n");
			return true; // Not a critical error, just move on
		}

		line++;

		DBEntry entry;
		std::list<std::string>::const_iterator it = entries.begin();
		entry.RangeFrom = from_string<Ip>(*it);
		it++;
		entry.RangeTo = from_string<Ip>(*it);
		it++;
		entry.Info.Continent = *it;
		it++;
		it++;
		entry.Info.CountryShortcut = *it;
		it++;
		it++;
		entry.Info.Country = *it;

		return handler(entry);
	}

};


using namespace std;

template<typename _handler>
class DBEntryHandler {
public:
	_handler& handler;
	bool& breakSignal;
	ifstream* file;
	DBEntryHandler(_handler& h, bool& b, ifstream* f)
		: handler(h), breakSignal(b), file(f) {}

	inline bool operator()(const DBEntry& entry) {
		return !breakSignal && handler(entry);
	}
};

class AddEntrysToDBData {
public:
	DBData& data;
	AddEntrysToDBData(DBData& d) : data(d) {}

	bool operator()(const DBEntry& entry) {
		data.push_back(entry);
		return true;
	}
};

class IpToCountryData {
public:
	std::string		filename;
	DBData			data;
	SDL_Thread*		loader;
	bool			dbReady; // false, if loaderThread is running
	bool			loaderBreakSignal;

	typedef DBEntryHandler<AddEntrysToDBData> DBEH;
	typedef CountryCsvReaderHandler<DBEH> CCRH;
	CsvReader<CCRH> csvReader;

	IpToCountryData() : loader(NULL), dbReady(true), loaderBreakSignal(false)
	{}

	virtual ~IpToCountryData() {
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
		loaderBreakSignal = false;
	}

	// HINT: this should only called from the mainthread
	// (or at least from the same thread where this DB is used),
	// because the handling of dbReady isn't threadsafe
	void loadFile(const std::string& fn) {
		if (filename == fn)
			return;

		// Destroy any previous loading
		breakLoaderThread();
		dbReady = false;
		loaderBreakSignal = false;

		filename = fn;
		data.clear();

		loader = SDL_CreateThread(loaderMain, this);
	}

	static int loaderMain(void* obj) {
		float timer = 0.0f;
		IpToCountryData* _this = (IpToCountryData*)obj;

		std::ifstream* file = OpenGameFileR(_this->filename);
		if(file == NULL) {
			cerr << "ERROR: cannot read " << _this->filename << endl;
			_this->dbReady = true;
			return 0; // TODO: other return? who got this?
		}

		timer = GetMilliSeconds();
		_this->data.reserve(90000); // There are about 90 000 entries in the file, do this to avoid reallocations
		cout << "IpToCountryDB: reading " << _this->filename << " ..." << endl;
		AddEntrysToDBData adder(_this->data);
		DBEH dbEntryHandler(adder, _this->loaderBreakSignal, file);
		CCRH csvReaderHandler(dbEntryHandler);
		_this->csvReader.setStream(file);
		_this->csvReader.setHandler(&csvReaderHandler);

		if(_this->csvReader.read()) {
			cout << "IpToCountryDB: reading finished, " << _this->data.size() << " entries" << endl;
		} else {
			cout << "IpToCountryDB: reading breaked, read " << _this->data.size() << " entries so far" << endl;
		}
		cout << "IpToCountryDB: loadtime " << (GetMilliSeconds() - timer) << " seconds" << endl;

		file->close();
		delete file;

		_this->dbReady = true;

		// Notify that the DB has been loaded
		SendSDLUserEvent(&onDummyEvent, EventData());

		return 0;
	}

	const DBEntry getEntry(Ip ip) {
		float start = GetMilliSeconds();
		if(!dbReady) {
			cout << "IpToCountryDB getEntry: " << filename << " is still loading ..." << endl;
			throw "The database is still loading...";
		}

		// Find the correct entry
		DBData::const_iterator it = data.begin();
		for (; it != data.end(); it++)  {
			if (it->RangeFrom <= ip && it->RangeTo >= ip)
				break;
		}

		if(it != data.end())  { // in range?
			DBEntry result = *it;

			ucfirst(result.Info.Country);

			// Small hack, Australia is considered as Asia by the database
			if(result.Info.Country == "Australia")
				result.Info.Continent = "Australia";

			// Convert the IANA code to a continent
			else if (result.Info.Continent == "IANA")
				result.Info.Continent = "Local Network";
			else if (result.Info.Continent == "ARIN")
				result.Info.Continent = "North America";
			else if (result.Info.Continent == "LACNIC")
				result.Info.Continent = "South America";
			else if (result.Info.Continent == "AFRINIC")
				result.Info.Continent = "Africa";
			else if (result.Info.Continent == "RIPE")
				result.Info.Continent = "Europe";
			else if (result.Info.Continent == "APNIC")
				result.Info.Continent = "Asia";

			printf("Getting the entry took %f\n", GetMilliSeconds() - start);

			return result;

		} else
			throw "The IP was not found in the database";
	}


	inline int getProgress() {
		/*if(csvReader.bufLen == 0) return 100;
		return (int)(((float)csvReader.bufPos / (float)csvReader.bufLen) * 100.0f);*/
		return 0;
	}

};

DECLARE_INTERNDATA_CLASS(IpToCountryDB, IpToCountryData);


IpToCountryDB::IpToCountryDB(const std::string &dbfile) {
	INTERNDATA__init(); // needed INTERNCLASS-init function
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

	DBEntry entry;
	try  {
		entry = IpToCountryDBData(this)->getEntry(ip);
		Result = entry.Info;
	} catch (...)  {
		Result.Continent = "Unknown continent";
		Result.Country = "Unknown country";
	}

	return Result;
}

int IpToCountryDB::GetProgress()  {
	return IpToCountryDBData(this)->getProgress();
}

bool IpToCountryDB::Loaded()  {
	return IpToCountryDBData(this)->dbReady;
}
