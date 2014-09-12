#ifndef VERMES_LOADERS_VERMES_H
#define VERMES_LOADERS_VERMES_H

#include "../resource_locator.h"
#include "../level.h"
#ifndef DEDICATED_ONLY
#include "../font.h"
#include "../menu.h"
#endif
#include "../script.h"

struct GusanosLevelLoader : ResourceLocator<CMap>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(CMap*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static GusanosLevelLoader instance;
};

#ifndef DEDICATED_ONLY
struct GusanosFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Font*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static GusanosFontLoader instance;
};

struct XMLLoader : ResourceLocator<XMLFile, false, false>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(XMLFile*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static XMLLoader instance;
};
#endif

struct LuaLoader : ResourceLocator<Script>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Script*, std::string const& path);
	
	virtual const char* getName();
	virtual std::string format();
	virtual std::string formatShort();
	
	static LuaLoader instance;
};

#endif //VERMES_LOADERS_VERMES_H
