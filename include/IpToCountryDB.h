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

typedef struct _ipinfo_t {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
} ipinfo_t;

typedef std::map<std::string,ipinfo_t> ipcache_t;

class CIpToCountry  {
public:
	CIpToCountry(const std::string& dbfile);
	~CIpToCountry();
private:
	std::string		sFile;
	ipcache_t		tIPCache;
	std::ifstream	*tDatabase;
	void			*tReader;
public:
	ipinfo_t	GetInfoAboutIP(const std::string& Address);
};

#endif
