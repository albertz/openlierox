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

#include <map>
#include <string>
#include <vector>

struct IpInfo {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
};

typedef std::map<std::string, IpInfo> ipcache_t;

class IpToCountryDB  {
public:
	IpToCountryDB(const std::string& dbfile);
	~IpToCountryDB();
private:
	std::string		sFile;
	ipcache_t		tIPCache;
	std::ifstream	*tDatabase;
	void			*tReader;
public:
	IpInfo	GetInfoAboutIP(const std::string& Address);
};

#endif
