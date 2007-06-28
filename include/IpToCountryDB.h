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

#include <SDL/SDL_thread.h>
#include <string>
#include "Utils.h"

struct IpInfo {
	std::string		Country;
	std::string		Continent;
	std::string		CountryShortcut;
};

INTERNDATA_CLASS_BEGIN(IpToCountryDB)
private:
	bool			bDbReady;
	SDL_Thread		*loadThread;
	static int	threadMain(void *param);
public:
	IpToCountryDB(const std::string& dbfile);
	void LoadDBFile(const std::string& dbfile);
	IpInfo GetInfoAboutIP(const std::string& Address);
	inline bool isDbReady()  { return bDbReady; }
INTERNDATA_CLASS_END

#endif
