#ifndef GUSANOS_LOADERS_LIEROX_H
#define GUSANOS_LOADERS_LIEROX_H

#include "../resource_locator.h"
#include "../level.h"

struct LieroXLevelLoader : ResourceLocator<Level>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Level*, fs::path const& path);
	
	virtual const char* getName();
	
	static LieroXLevelLoader instance;
};

#endif //GUSANOS_LOADERS_LIEROX_H
