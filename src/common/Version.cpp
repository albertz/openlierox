#include <iostream>
#include <cstdio>

#include "Version.h"
#include "LieroX.h"
#include "AuxLib.h"

using std::cout;
using std::endl;


std::string GetFullGameName() {
	std::string name = GetGameName();
	if(name == "") {
		printf("WARNING: gamename still undefined\n");
		name = "OpenLieroX";
	}
	return name + "/" + LX_VERSION;
}


inline void setByString__optionalPostCheck(const Version* version, const std::string& versionStr) {
#ifdef DEBUG
	if(version->asString() != versionStr) {
		cout << "WARNING: Version::setByString: '" << versionStr << "' get parsed as '" << version->asString() << "'" << endl;
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
		default: ret += "_"; break;
		}
		ret += itoa(subsubnum);
	}

	if(revnum > 0) {
		ret += "_r";
		ret += itoa(revnum);
	}

	return ret;
}


// For comparision, we ignore the following: revnum, gamename
// That means, a special revision of a baseversion should not change the behaviour (and it's only for debugging).
// And another game like Hirudo should keep the same version-counting. We can start Hirudo at version 1.0 or 0.99.

bool operator<(const Version& ver1, const Version& ver2) {
	if(ver1.num != ver2.num) return ver1.num < ver2.num;
	if(ver1.subnum != ver2.subnum) return ver1.subnum < ver2.subnum;
	if(ver1.releasetype != ver2.releasetype) return ver1.releasetype < ver2.releasetype;
	if(ver1.subsubnum != ver2.subsubnum) return ver1.subsubnum < ver2.subsubnum;
	return false;
}

bool operator==(const Version& ver1, const Version& ver2) {
	return
		ver1.num == ver2.num &&
		ver1.subnum == ver2.subnum &&
		ver1.releasetype == ver2.releasetype &&
		ver1.subsubnum == ver2.subsubnum;
}
