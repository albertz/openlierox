#ifndef GUSANOS_LOADERS_GUSANOS_H
#define GUSANOS_LOADERS_GUSANOS_H

#include "../resource_locator.h"
#include "../level.h"
#ifndef DEDSERV
#include "../font.h"
#include "../menu.h"
#endif
#include "../script.h"

struct GusanosLevelLoader : ResourceLocator<Level>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Level*, fs::path const& path);
	
	virtual const char* getName();
	
	static GusanosLevelLoader instance;
};

#ifndef DEDSERV
struct GusanosFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Font*, fs::path const& path);
	
	virtual const char* getName();
	
	static GusanosFontLoader instance;
};

struct XMLLoader : ResourceLocator<XMLFile, false, false>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(XMLFile*, fs::path const& path);
	
	virtual const char* getName();
	
	static XMLLoader instance;
};

struct GSSLoader : ResourceLocator<GSSFile>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(GSSFile*, fs::path const& path);
	
	virtual const char* getName();
	
	static GSSLoader instance;
};
#endif

struct LuaLoader : ResourceLocator<Script>::BaseLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Script*, fs::path const& path);
	
	virtual const char* getName();
	
	static LuaLoader instance;
};

#endif //GUSANOS_LOADERS_GUSANOS_H
