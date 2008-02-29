#include "Version.h"
#include "LieroX.h"
#include "AuxLib.h"


std::string GetFullGameName() {
	return GetGameName() + "/" + LX_VERSION;
}
