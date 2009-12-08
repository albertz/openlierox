#ifndef VERMES_LOADERS_LIEROX_H
#define VERMES_LOADERS_LIEROX_H

#include "../resource_locator.h"
#include "../level.h"

struct LieroXLevelLoader : ResourceLocator<Level>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Level*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static LieroXLevelLoader instance;
};

#endif //VERMES_LOADERS_LIEROX_H
