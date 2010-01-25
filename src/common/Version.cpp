/*
 OpenLieroX
 
 version parsing
 
 file created 29-02-2008 by albert
 code under LGPL
 */

#include "Version.h"
#include "LieroX.h"
#include "Debug.h"
#include "AuxLib.h"


#include "Version_generated.h"

#ifndef		LX_VERSION
#	define		LX_VERSION	"0.59_beta3"
#endif

#define		GAMENAME			"OpenLieroX"


#ifndef ONLY_MACRODEF  // The above defines are used in resurce.rc

const char* const fullGameName = GAMENAME "/" LX_VERSION;

const char* GetGameName() {
	return GAMENAME;
}

inline void setByString__optionalPostCheck(const Version* version, const std::string& versionStr) {
#ifdef DEBUG
	if(version->asString() != versionStr) {
		notes << "WARNING: Version::setByString: '" << versionStr << "' get parsed as '" << version->asString() << "'" << endl;
	}
#endif
}

void Version::setByString(const std::string& versionStr) {
	if(versionStr == "") { reset(); setByString__optionalPostCheck(this,versionStr); return; }

	std::string tmp = versionStr;

	size_t p = tmp.find_first_of(" /\\");
	if(p == std::string::npos)
		gamename = "OpenLieroX"; // old OLX sends sometimes not its name
	else {
		gamename = tmp.substr(0, p);
		tmp.erase(0, p + 1);
	}

	// reset some values
	num = subnum = subsubnum = revnum = 0;
	releasetype = RT_NORMAL;
	stringlwr(tmp); // to have beta instead of Beta

	// num
	p = tmp.find(".");
	bool fail = false;
	num = from_string<int>(tmp.substr(0, p), fail);
	if(fail) { num = 0; setByString__optionalPostCheck(this,versionStr); return; }
	if(p == std::string::npos) { setByString__optionalPostCheck(this,versionStr); return; }
	tmp.erase(0, p + 1);

	// subnum
	p = tmp.find_first_of("._-");
	subnum = from_string<int>(tmp.substr(0, p), fail);
	if(fail) { subnum = 0; setByString__optionalPostCheck(this,versionStr); return; }
	if(p == std::string::npos) { setByString__optionalPostCheck(this,versionStr); return; }
	tmp.erase(0, p + 1);

	// releasetype
	if(tmp == "") { setByString__optionalPostCheck(this,versionStr); return; }
	size_t nextNumP = tmp.find_first_of("0123456789");
	if(nextNumP == 0)
		releasetype = RT_NORMAL;
	else if(tmp.find("alpha") == 0)
		releasetype = RT_ALPHA;
	else if(tmp.find("beta") == 0)
		releasetype = RT_BETA;
	else if(tmp.find("rc") == 0)
		releasetype = RT_RC;
	else
		releasetype = RT_UNKNOWN;
	tmp.erase(0, nextNumP);

	// subsubnum
	if(tmp == "") { setByString__optionalPostCheck(this,versionStr); return; }
	if(tmp.find_first_of(".") == 0) tmp.erase(0, 1);
	p = tmp.find_first_of("._-");
	subsubnum = from_string<int>(tmp.substr(0, p), fail);
	if(fail) { subsubnum = 0; setByString__optionalPostCheck(this,versionStr); return; }
	if(p == std::string::npos) { setByString__optionalPostCheck(this,versionStr); return; }
	tmp.erase(0, p + 1);

	// revnum
	nextNumP = tmp.find_first_of("0123456789");
	if(nextNumP == std::string::npos) { setByString__optionalPostCheck(this,versionStr); return; }
	tmp.erase(0, nextNumP);
	revnum = from_string<int>(tmp, fail);
	if(fail) revnum = 0;

	setByString__optionalPostCheck(this,versionStr); return;
}


std::string Version::asString() const {
	std::string ret = gamename;
	ret += "/";
	ret += itoa(num);
	ret += ".";
	ret += itoa(subnum);

	if(subsubnum > 0) {
		switch(releasetype) {
		case RT_NORMAL: ret += "."; break;
		case RT_ALPHA: ret += "_alpha"; break;
		case RT_BETA: ret += "_beta"; break;
		case RT_RC: ret += "_rc"; break;
		case RT_UNKNOWN: ret += "_"; break;
		}
		ret += itoa(subsubnum);
	}

	if(revnum > 0) {
		ret += "_r";
		ret += itoa(revnum);
	}

	return ret;
}

std::string Version::asHumanString() const
{
	std::string ret = gamename;
	ret += " ";
	ret += itoa(num);
	ret += ".";
	ret += itoa(subnum);

	if(subsubnum > 0) {
		switch(releasetype) {
		case RT_NORMAL: ret += "."; break;
		case RT_ALPHA: ret += " alpha "; break;
		case RT_BETA: ret += " beta "; break;
		case RT_RC: ret += " rc "; break;
		case RT_UNKNOWN: ret += " "; break;
		}
		ret += itoa(subsubnum);
	}

	return ret;
}

std::string Version::releaseType() const
{
	switch(releasetype) {
	case RT_NORMAL: return "final";
	case RT_ALPHA: return "alpha";
	case RT_BETA: return "beta";
	case RT_RC: return "rc";
	case RT_UNKNOWN: return "unknown";
	}

	return "error"; // should not happen
}


static Version gameVersion(GetFullGameName());

const Version& GetGameVersion() {
	return gameVersion;
}


bool Version::isBanned() const {
	return *this == OLXBetaVersion(9) || *this == OLXBetaVersion(0,58,1);
}


#endif  // ONLY_MACRODEF
