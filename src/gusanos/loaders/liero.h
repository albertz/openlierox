#ifndef VERMES_LOADERS_LIERO_H
#define VERMES_LOADERS_LIERO_H

#include "../resource_locator.h"
#include "../level.h"
#ifndef DEDICATED_ONLY
#include "../font.h"
#endif
#include "game/CMap.h"

#ifndef DEDICATED_ONLY

struct LieroFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Font*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static LieroFontLoader instance;
};

#endif

#endif //VERMES_LOADERS_LIERO_H
