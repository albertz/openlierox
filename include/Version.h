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

struct Version;
extern const char* const fullGameName;
inline const char* GetFullGameName() { return fullGameName; }
const char* GetGameName();
const Version& GetGameVersion();


struct Version {
	Version() { reset(); }
	Version(const std::string& versionStr) { setByString(versionStr); }
	
	void reset() { gamename = "LieroX"; num = 0; subnum = 56; subsubnum = 0; revnum = 0; releasetype = RT_NORMAL; }
	void setByString(const std::string& versionStr);
	std::string asString() const;
	std::string asHumanString() const;
	std::string releaseType() const;
	
	bool isBanned() const;
	
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


// The following functions are declared inline because they are used very often.

// For comparision, we ignore the following: revnum, gamename
// That means, a special revision of a baseversion should not change the behaviour (and it's only for debugging).
// And another game like Hirudo should keep the same version-counting. We can start Hirudo at version 1.0 or 0.99.

inline bool operator<(const Version& ver1, const Version& ver2) {
	if(ver1.num != ver2.num) return ver1.num < ver2.num;
	if(ver1.subnum != ver2.subnum) return ver1.subnum < ver2.subnum;
	if(ver1.releasetype != ver2.releasetype) return ver1.releasetype < ver2.releasetype;
	if(ver1.subsubnum != ver2.subsubnum) return ver1.subsubnum < ver2.subsubnum;
	return false;
}

inline bool operator==(const Version& ver1, const Version& ver2) {
	return
	ver1.num == ver2.num &&
	ver1.subnum == ver2.subnum &&
	ver1.releasetype == ver2.releasetype &&
	ver1.subsubnum == ver2.subsubnum;
}

inline bool operator>(const Version& ver1, const Version& ver2) { return ver2 < ver1; }
inline bool operator<=(const Version& ver1, const Version& ver2) { return ver1 < ver2 || ver1 == ver2; }
inline bool operator>=(const Version& ver1, const Version& ver2) { return ver2 < ver1 || ver1 == ver2; }
inline bool operator!=(const Version& ver1, const Version& ver2) { return ! (ver1 == ver2); }

inline Version OLXBetaVersion(int betaversion) {
	Version v;
	v.gamename = fullGameName;
	v.num = 0;
	v.subnum = 57;
	v.subsubnum = betaversion;
	v.releasetype = Version::RT_BETA;
	return v;
}

inline Version OLXBetaVersion(int num, int subnum, int betaversion) {
	Version v;
	v.gamename = fullGameName;
	v.num = num;
	v.subnum = subnum;
	v.subsubnum = betaversion;
	v.releasetype = Version::RT_BETA;
	return v;
}

inline Version OLXRcVersion(int num, int subnum, int rcversion) {
	Version v;
	v.gamename = fullGameName;
	v.num = num;
	v.subnum = subnum;
	v.subsubnum = rcversion;
	v.releasetype = Version::RT_RC;
	return v;
}

inline Version OLXVersion(int num, int subnum, int subsubnum = 0) {
	Version v;
	v.gamename = fullGameName;
	v.num = num;
	v.subnum = subnum;
	v.subsubnum = subsubnum;
	v.releasetype = Version::RT_NORMAL;
	return v;	
}

#endif
