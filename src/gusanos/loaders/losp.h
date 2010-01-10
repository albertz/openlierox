#ifndef VERMES_LOADERS_LOSP_H
#define VERMES_LOADERS_LOSP_H

#include "../resource_locator.h"
#ifndef DEDICATED_ONLY
#include "../font.h"
#endif

#ifndef DEDICATED_ONLY

struct LOSPFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Font*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static LOSPFontLoader instance;
};

#endif

#endif //VERMES_LOADERS_LOSP_H
