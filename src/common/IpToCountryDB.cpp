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
#include <SDL_mutex.h>

#include <map>

#include "Options.h"
#include "Debug.h"
#include "TSVar.h"
#include "IpToCountryDB.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CsvReader.h"
#include "Timer.h"
#include "InputEvents.h"

/*
How does this work?
Because the database contains approx. 90 000 items, it is quite time consuming
to load it in memory. Because users do not like waiting, we have to distribute
the loading in time to be unobtrusive for the user:
1. Start loading the database as soon as possible (right after loading the options, see main.cpp)
2. When starting to load it, read as many items as possible in a reasonable time (0.2 sec atm)
3. After the initial reading, move all the loading to a separate thread
4. When the user wants to read from the database when it is not fully loaded yet,
   the loading thread is paused and the so-far-read entries are searched 
   4.1 If the entry is found, it is returned and the thread searching continues
   4.2 If the entry is not found, we start "fast" reading (with no SDL_Delay) and try to find
       the entry in the remaining entries; if the entry is not found within a reasonable time,
	   it is considered as not found and the threading search continues
*/


#ifdef _MSC_VER  // MSVC 6 has problems with from_string<Uint32>
typedef unsigned int Ip;
#else
typedef Uint32 Ip;
#endif

#define READ_CHUNK_SIZE  256

struct DBEntry {
	Ip			RangeFrom;
	Ip			RangeTo;
	IpInfo		Info;
};


// key-value is ending-ip (RangeTo) of representing ip-range
typedef std::vector<DBEntry> DBData;



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
		
		if (entries.size() < 7)  {
			hints << "IpToCountry loader warning: number of entries is less than 7, ignoring the line " << line << "\n";
			if (entries.size())  {
				hints << "\"" << *entries.begin() << "\"";
				std::list<std::string>::const_iterator it = entries.begin(); it++;
				for (; it != entries.end(); it++)  {
					hints << ",\"" << *it << "\"";
				}
			}
			hints << endl;
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

		handler(entry);
		return true;
	}

};




class AddEntriesToDBData {
public:
	DBData& data;
	AddEntriesToDBData(DBData& d) : data(d) {}

	// NOTE: thread safety is ensured in threadLoader()
	void operator()(const DBEntry& entry) {
		data.push_back(entry);
	}
};

class IpToCountryData {
public:
	std::string		filename;
	std::ifstream	*file;
	DBData			data;

	SDL_Thread		*loader;
	SDL_mutex		*dataMutex;
	volatile bool	breakLoader;

	typedef CountryCsvReaderHandler<AddEntriesToDBData> CCRH;
	CCRH *csvReaderHandler;
	AddEntriesToDBData *adder;
	CsvReader<CCRH> csvReader;

	IpToCountryData() : file(NULL), loader(NULL), dataMutex(NULL), breakLoader(false),
		csvReaderHandler(NULL), adder(NULL)
	{}

	virtual ~IpToCountryData() {
		breakLoader = true;
		if (loader)
			SDL_WaitThread(loader, NULL);

		data.clear();
		if (csvReaderHandler)
			delete csvReaderHandler;
		if (adder)
			delete adder;
		if (dataMutex)
			SDL_DestroyMutex(dataMutex);
		if (file)  {
			file->close();
			delete file;
		}
	}

	// HINT: this should only called from the mainthread
	// (or at least from the same thread where this DB is used),
	// because the handling of dbReady isn't threadsafe
	void loadFile(const std::string& fn) {
		if (filename == fn)
			return;

		filename = fn;
		data.clear();

		//loader = SDL_CreateThread(loaderMain, this);
		file = OpenGameFileR(filename);
		if (!file || !file->is_open())  {
			warnings << "IpToCountry Database Error: Cannot find the database file." << endl;
			return; 
		}

		data.reserve(90000); // There are about 90 000 entries in the file, do this to avoid reallocations
		notes << "IpToCountryDB: reading " << filename << " ..." << endl;

		// Initialize the reader
		adder = new AddEntriesToDBData(data);
		csvReaderHandler = new CCRH(*adder);
		csvReader.setStream(file);
		csvReader.setHandler(csvReaderHandler);

		// Read as many entries as possible within a reasonable time
		float start = GetMilliSeconds();
		while (GetMilliSeconds() - start <= 0.2f)
			csvReader.readSome(READ_CHUNK_SIZE);

		// Start adding the entries in parallel
		startThreadAdding();
	}

	void startThreadAdding()
	{
		// Stop any previous loading and cleanup
		breakLoader = true;
		if (loader)
			SDL_WaitThread(loader, NULL);
		loader = NULL;
		breakLoader = false;

		// Start a new loading
		if( ! dataMutex )
			dataMutex = SDL_CreateMutex();
		loader = SDL_CreateThread(&threadLoader, (void *)this);
	}

	// Adds data simultaneously as the game is running
	static int threadLoader(void *param)
	{
		IpToCountryData *_this = (IpToCountryData *)param;
		float start = GetMilliSeconds();

		while (!_this->breakLoader)  {
			// Read another chunk
			SDL_LockMutex(_this->dataMutex);
			_this->csvReader.readSome(READ_CHUNK_SIZE);
			SDL_UnlockMutex(_this->dataMutex);

			// Sleep a bit so we don't lag single-core processors
			SDL_Delay(5);

			// Finished?
			SDL_LockMutex(_this->dataMutex);
			if (_this->csvReader.readingFinished())  {
				notes << "IpToCountry Database: reading finished, " << _this->data.size() << " entries, " << (GetMilliSeconds() - start) << " seconds" << endl;
				SDL_UnlockMutex(_this->dataMutex);
				break;
			}
			SDL_UnlockMutex(_this->dataMutex);
		}

		return 0;
	}

	const DBEntry getEntry(Ip ip) {
		float start = GetMilliSeconds();
		
		SDL_LockMutex(dataMutex);

		size_t search_start = 0;
		while (true)  {

			// Find the correct entry
			DBData::const_iterator it = data.begin() + search_start;
			for (; it != data.end(); it++)  {
				if (it->RangeFrom <= ip && it->RangeTo >= ip)
					break;
			}
			search_start = data.size();

			if (it != data.end())  { // in range?
				DBEntry result = *it;

				SDL_UnlockMutex(dataMutex);

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

				return result;

			// Not found
			} else  {
				// If the searching took too much time or the entry has not been found at all, throw an exception
				if (csvReader.readingFinished() || GetMilliSeconds() - start >= 0.3f)  {
					SDL_UnlockMutex(dataMutex);
					warnings << "IpToCountry Database: the entry has not been found within a reasonable time, giving up..." << endl;
					throw "The IP was not found in the database";
				}  else  {
					// Read chunk of data
					// HINT: the mutex is locked which means that the loader thread is paused
					// HINT: we are reading here and not waiting for the thread adder because this is faster (no sleeping)
					csvReader.readSome(READ_CHUNK_SIZE);
				}
			}
		}

		SDL_UnlockMutex(dataMutex);
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
	IpInfo Result;

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
	if (IpToCountryDBData(this)->file)
	{
		SDL_LockMutex(IpToCountryDBData(this)->dataMutex);
		bool finished = IpToCountryDBData(this)->csvReader.readingFinished();
		SDL_UnlockMutex(IpToCountryDBData(this)->dataMutex);
		return finished;
	}
	else
		return false;
}
