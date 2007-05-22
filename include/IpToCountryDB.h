/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __IPTOCOUNTRY_H__
#define	__IPTOCOUNTRY_H__

#include <string>
#include <map>
#include <vector>
#include "Utils.h"

struct IpInfo {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
};

struct CacheItem {
	uint			RangeFrom;
	uint			RangeTo;
	IpInfo			Info;
};

/*INTERNDATA_CLASS_BEGIN(IpToCountryDB)
public:	
	IpToCountryDB(const std::string& dbfile);
	void LoadDBFile(const std::string& dbfile);
	IpInfo	GetInfoAboutIP(const std::string& Address);
INTERNDATA_CLASS_END*/
typedef std::map<uint,IpInfo> IpCache;
typedef std::vector<CacheItem> DBCache;

class IpToCountryDB  {
public:
	IpToCountryDB(const std::string& dbfile);
	~IpToCountryDB();
private:
	IpCache			tIpCache;
	DBCache			tDBCache;
	std::ifstream	fDatabase;
public:
	IpInfo	GetInfoAboutIP(const std::string& Address);
};


#endif
