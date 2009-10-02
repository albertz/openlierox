/*
	OpenLieroX

	version parsing

	file created 29-02-2008 by albert
	code under LGPL
*/

#ifndef __VERSION_H__
#define __VERSION_H__

#include <string>
#include "StringUtils.h" // for itoa

#ifndef		LX_VERSION
#	define		LX_VERSION		"0.57_rc1"
#endif

#define		GAMENAME			"OpenLieroX"


std::string GetFullGameName();


class Version { public:
	Version() { reset(); }
	Version(const std::string& versionStr) { setByString(versionStr); }

	void reset() { setByString("LieroX/0.56"); }
	void setByString(const std::string& versionStr);
	std::string asString() const;

	int num; // 0
	int subnum; // 57
	int subsubnum; // 3 (Beta-version)
	int revnum; // 1007 (SVN)
	enum Releasetype {
		// HINT: keep in this order to allow direct comparsion here
		RT_UNKNOWN, RT_ALPHA, RT_BETA, RT_RC, RT_NORMAL
	} releasetype;
	std::string gamename; // OpenLieroX

};

bool operator<(const Version& ver1, const Version& ver2);
bool operator==(const Version& ver1, const Version& ver2);

inline bool operator>(const Version& ver1, const Version& ver2) { return ver2 < ver1; }
inline bool operator<=(const Version& ver1, const Version& ver2) { return ver1 < ver2 || ver1 == ver2; }
inline bool operator>=(const Version& ver1, const Version& ver2) { return ver2 < ver1 || ver1 == ver2; }
inline bool operator!=(const Version& ver1, const Version& ver2) { return ! (ver1 == ver2); }

inline Version GetGameVersion() { return Version(GetFullGameName()); }
inline Version OLXBetaVersion(int betaversion) {	return Version("OpenLieroX/0.57_beta" + itoa(betaversion)); }

#endif
