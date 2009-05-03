#include "gusanos.h"
#include "../gfx.h"
#include "../blitters/types.h"
#include "../glua.h"
#include "luaapi/context.h"
#include "../events.h"
#ifndef DEDSERV
#include "../menu.h"
#endif
#include "../game_actions.h"
#include "../parser.h"
#include "util/macros.h"
#include <string>

#include "omfg_script.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

#include <iostream>
using namespace std;

GusanosLevelLoader GusanosLevelLoader::instance;

bool GusanosLevelLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::exists(path / "config.cfg"))
	{
		name = path.leaf();
		return true;
	}
	return false;
}

namespace{

	enum type
	{
		GameStart,
		GameEnd
	};

	LevelConfig* loadConfig( fs::path const& filename )
	{
		fs::ifstream fileStream(filename, std::ios::binary | std::ios::in);

		if (!fileStream )
			return false;
		
		OmfgScript::Parser parser(fileStream, gameActions, filename.native_file_string());
		
		parser.addEvent("game_start", GameStart, 0);
		parser.addEvent("game_end", GameEnd, 0);
		
		if(!parser.run())
		{
			parser.error("Trailing garbage");
			return false;
		}
		
		LevelConfig* returnConf = new LevelConfig;
		
		OmfgScript::TokenBase* tmpProp = parser.getProperty("spawnpoints");
		if ( tmpProp->isList() )
		{
			std::list<OmfgScript::TokenBase*>::const_iterator sp;
			const_foreach(sp, tmpProp->toList())
			{
				OmfgScript::TokenBase& v = **sp;
				if ( v.assertList() )
				{
					std::list<OmfgScript::TokenBase*> const& spParams = v.toList();
					std::list<OmfgScript::TokenBase*>::const_iterator iter = spParams.begin();
					
					float x = 0;
					float y = 0;
					int team = -1;
					
					if ( iter != spParams.end() )
					{
						x = (*iter)->toDouble();
						++iter;
					}
					if ( iter != spParams.end() )
					{
						y = (*iter)->toDouble();
						++iter;
					}
					if ( iter != spParams.end() )
					{
						team = (*iter)->toInt();
						++iter;
					}
					
					returnConf->spawnPoints.push_back(SpawnPoint( Vec(x,y), team ));
				}
			}
		}
		
		returnConf->darkMode = parser.getBool("dark_mode");
		
		OmfgScript::Parser::EventIter i(parser);
		for(; i; ++i)
		{
			std::vector<OmfgScript::TokenBase*> const& p = i.params();
			switch(i.type())
			{
				case GameStart:
					returnConf->gameStart = new Event(i.actions());
				break;
				
				case GameEnd:
					returnConf->gameEnd = new Event(i.actions());
				break;
			}
		}
		return returnConf;
	}

}
	
bool GusanosLevelLoader::load(Level* level, fs::path const& path)
{
	std::string materialPath = (path / "material").native_file_string();
	
	level->path = path.native_directory_string();
	
	{
		LocalSetColorDepth cd(8);
		level->material = gfx.loadBitmap(materialPath.c_str(), 0);
	}
	
	if (level->material)
	{
		level->setEvents( loadConfig( path / "config.cfg" ) );
#ifndef DEDSERV
		std::string imagePath = (path / "level").native_file_string();
		
		level->image = gfx.loadBitmap(imagePath.c_str(), 0);
		if (level->image)
		{			
			std::string backgroundPath = (path / "background").native_file_string();
			
			level->background = gfx.loadBitmap(backgroundPath.c_str(),0);
			
			std::string paralaxPath = (path / "paralax").native_file_string();
			level->paralax = gfx.loadBitmap(paralaxPath.c_str(),0);
			
			if(!level->paralax)
				std::cerr << "Paralax not loaded" << std::endl;
			
			std::string lightmapPath = (path / "lightmap").native_file_string();
		
			BITMAP* tempLightmap = gfx.loadBitmap(lightmapPath.c_str(),0);
			if ( tempLightmap )
			{
				{
					LocalSetColorDepth cd(8);
					level->lightmap = create_bitmap(level->material->w, level->material->h);
				}
				for ( int x = 0; x < level->lightmap->w ; ++x )
				for ( int y = 0; y < level->lightmap->h ; ++y )
				{
					putpixel( level->lightmap, x, y, getg(getpixel(tempLightmap, x, y)) );
				}
				destroy_bitmap( tempLightmap );
			}
			
			level->loaderSucceeded();
			return true;
		}
		

#else
		level->loaderSucceeded();
		return true;
#endif
	}
	level->unload();
	return false;
}

const char* GusanosLevelLoader::getName()
{
	return "Gusanos 0.9 level loader";
}

#ifndef DEDSERV
GusanosFontLoader GusanosFontLoader::instance;

bool GusanosFontLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".bmp" || fs::extension(path) == ".png")
	{
		name = basename(path);
		return true;
	}
	return false;
}
	
bool GusanosFontLoader::load(Font* font, fs::path const& path)
{
	font->free();
	
	{
		LocalSetColorDepth cd(8);
		LocalSetColorConversion cc(COLORCONV_REDUCE_TO_256 | COLORCONV_KEEP_TRANS);
	
		font->m_bitmap = load_bitmap(path.native_file_string().c_str(), 0);
		if(!font->m_bitmap)
			return false;
		
		// Change all non-transparent pixels to full opacity
		for(int y = 0; y < font->m_bitmap->h; ++y)
		for(int x = 0; x < font->m_bitmap->w; ++x)
		{
			Pixel8* p = font->m_bitmap->line[y] + x;
			if(*p != 0)
				*p = 255;
		}
	}
		
	font->m_supportColoring = true;
		
	int monoWidth = font->m_bitmap->w / 256;
	int monoHeight = font->m_bitmap->h;
	
	if(monoWidth <= 0 || monoHeight <= 0)
		return false;

	int x = 0;
	for (int i = 0; i < 256; ++i)
	{
		/*BITMAP* character = create_bitmap(width,tempBitmap->h);
		blit(tempBitmap,character,i * width,0,0,0,width,character->h);*/
		font->m_chars.push_back(Font::CharInfo(Rect(x, 0, x + monoWidth, monoHeight), 0));
		
		x += monoWidth;
	}
	
	font->buildSubBitmaps();
	
	return true;
	/*
	BITMAP *tempBitmap = load_bmp(path.native_file_string().c_str(),0);
	if (tempBitmap)
	{
		int width = tempBitmap->w / 256;
		for (int i = 0; i < 256; ++i)
		{
			BITMAP* character = create_bitmap(width,tempBitmap->h);
			blit(tempBitmap,character,i * width,0,0,0,width,character->h);
			font->m_char.push_back(character);
		}
		destroy_bitmap(tempBitmap);
		return true;
	}

	return false;*/
}

const char* GusanosFontLoader::getName()
{
	return "Gusanos 0.9 font loader";
}

XMLLoader XMLLoader::instance;

bool XMLLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".xml")
	{
		name = basename(path);
		return true;
	}
	return false;
}
	
bool XMLLoader::load(XMLFile* xml, fs::path const& path)
{
	xml->f.open(path, std::ios::binary);
	
	if(!xml->f)
		return false;
		
	return true;
}

const char* XMLLoader::getName()
{
	return "XML loader";
}


GSSLoader GSSLoader::instance;

bool GSSLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".gss")
	{
		name = basename(path);
		return true;
	}
	return false;
}
	
bool GSSLoader::load(GSSFile* gss, fs::path const& path)
{
	//gss->f.open(path, std::ios::binary);
	fs::ifstream f(path, std::ios::binary);
	
	if(!f)
		return false;
	
	OmfgGUI::menu.loadGSS(f, path.string());
		
	return true;
}

const char* GSSLoader::getName()
{
	return "GSS loader";
}
#endif

LuaLoader LuaLoader::instance;

bool LuaLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".lua")
	{
		name = basename(path);
		return true;
	}
	return false;
}
	
bool LuaLoader::load(Script* script, fs::path const& path)
{
	fs::ifstream f(path, std::ios::binary | std::ios::in);
	if(!f)
		return false;
		
	// Create the table to store the functions in	
	std::string name = basename(path);
	lua_pushstring(lua, name.c_str());
	lua_rawget(lua, LUA_GLOBALSINDEX);
	if(lua_isnil(lua, -1))
	{
		// The table does not exist, create it
		
		lua_pushstring(lua, name.c_str());
		lua_newtable(lua);
		lua_rawset(lua, LUA_GLOBALSINDEX);
	}
	lua_settop(lua, -2); // Pop table or nil
	
	lua.load(path.native_file_string().c_str(), f);
	
	script->lua = &lua;
	script->table = name;
	
	return true;
}

const char* LuaLoader::getName()
{
	return "Lua loader";
}

/*
GusanosParticleLoader GusanosParticleLoader::instance;

bool GusanosParticleLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".obj")
	{
		name = path.leaf();
		return true;
	}
	return false;
}*/
