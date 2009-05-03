#ifndef GUSANOS_LOADERS_LOSP_H
#define GUSANOS_LOADERS_LOSP_H

#include "../resource_locator.h"
#ifndef DEDSERV
#include "../font.h"
#endif

#ifndef DEDSERV

struct LOSPFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Font*, fs::path const& path);
	
	virtual const char* getName();
	
	static LOSPFontLoader instance;
};

#endif

#endif //GUSANOS_LOADERS_LOSP_H
