#ifndef VERMES_LOADERS_VERMES_H
#define VERMES_LOADERS_VERMES_H

#include "../resource_locator.h"
#include "../level.h"
#ifndef DEDSERV
#include "../font.h"
#include "../menu.h"
#endif
#include "../script.h"

struct VermesLevelLoader : ResourceLocator<Level>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Level*, std::string const& path);
	
	virtual const char* getName();
	
	static VermesLevelLoader instance;
};

#ifndef DEDSERV
struct VermesFontLoader : ResourceLocator<Font>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Font*, std::string const& path);
	
	virtual const char* getName();
	
	static VermesFontLoader instance;
};

struct XMLLoader : ResourceLocator<XMLFile, false, false>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(XMLFile*, std::string const& path);
	
	virtual const char* getName();
	
	static XMLLoader instance;
};

struct GSSLoader : ResourceLocator<GSSFile>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(GSSFile*, std::string const& path);
	
	virtual const char* getName();
	
	static GSSLoader instance;
};
#endif

struct LuaLoader : ResourceLocator<Script>::BaseLoader
{
	virtual bool canLoad(std::string const& path, std::string& name);
	
	virtual bool load(Script*, std::string const& path);
	
	virtual const char* getName();
	
	static LuaLoader instance;
};

#endif //VERMES_LOADERS_VERMES_H
