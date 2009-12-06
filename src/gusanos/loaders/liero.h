#ifndef VERMES_LOADERS_LIERO_H
#define VERMES_LOADERS_LIERO_H

#include "../resource_locator.h"
#include "../level.h"
#ifndef DEDICATED_ONLY
#include "../font.h"
#endif

struct LieroLevelLoader : ResourceLocator<Level>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Level*, std::string const& path);
	
	virtual const char* getName();
	
	static LieroLevelLoader instance;
};

#ifndef DEDICATED_ONLY

struct LieroFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Font*, std::string const& path);
	
	virtual const char* getName();
	
	static LieroFontLoader instance;
};

#endif

#endif //VERMES_LOADERS_LIERO_H
