#ifndef VERMES_MENU_H
#define VERMES_MENU_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "gui/omfggui.h"
#include "resource_locator.h"
#include "gconsole.h"
#include "blitters/context.h"

class Font;
class SpriteSet;
struct ALLEGRO_BITMAP;

namespace OmfgGUI
{

class GContext : public Context
{
public:
	GContext();
	
	void init();
	void clear();
	
	virtual LuaContext& luaContext();
		
	virtual Wnd* loadXMLFile(std::string const& name, Wnd* loadTo);
};

extern GContext menu;

}

class XMLFile
{
public:
	operator bool()
	{
		return (bool)f;
	}
	
	std::ifstream f;
};

extern ResourceLocator<XMLFile, false, false> xmlLocator;


#endif //VERMES_MENU_H
