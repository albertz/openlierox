#include "vermes.h"
#include "../gfx.h"
#include "../blitters/types.h"
#include "../glua.h"
#include "../luaapi/context.h"
#include "../events.h"
#ifndef DEDICATED_ONLY
#include "../menu.h"
#endif
#include "../game_actions.h"
#include "../parser.h"
#include "util/macros.h"
#include "CMap.h"
#include <string>

#include "../omfgscript/omfg_script.h"
#include "FindFile.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

#include <iostream>
using namespace std;

VermesLevelLoader VermesLevelLoader::instance;

bool VermesLevelLoader::canLoad(std::string const& path, std::string& name)
{
	if(IsFileAvailable(path + "/config.cfg"))
	{
		name = GetBaseFilenameWithoutExt(path);
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

	void fillSpawnPointList(OmfgScript::Parser& parser, const std::string& propname, std::vector<SpawnPoint>& pts) {
		OmfgScript::TokenBase* tmpProp = parser.getProperty(propname);
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
						x = (float)(*iter)->toDouble();
						++iter;
					}
					if ( iter != spParams.end() )
					{
						y = (float)(*iter)->toDouble();
						++iter;
					}
					if ( iter != spParams.end() )
					{
						team = (*iter)->toInt();
						++iter;
					}
					
					pts.push_back(SpawnPoint( Vec(x,y), team ));
				}
			}
		}		
	}
	
	LevelConfig* loadConfig( std::string const& filename )
	{
		std::ifstream fileStream;
		OpenGameFileR(fileStream, filename, std::ios::binary | std::ios::in);

		if (!fileStream )
			return false;
		
		OmfgScript::Parser parser(fileStream, gameActions, filename);
		
		parser.addEvent("game_start", GameStart, 0);
		parser.addEvent("game_end", GameEnd, 0);
		
		if(!parser.run())
		{
			parser.error("Trailing garbage");
			return false;
		}
		
		LevelConfig* returnConf = new LevelConfig;
		
		fillSpawnPointList(parser, "spawnpoints", returnConf->spawnPoints);
		fillSpawnPointList(parser, "teambases", returnConf->teamBases);
		
		returnConf->darkMode = parser.getBool("dark_mode");
		
		OmfgScript::Parser::GameEventIter i(parser);
		for(; i; ++i)
		{
			//std::vector<OmfgScript::TokenBase*> const& p = i.params(); //unused
			switch(i.type())
			{
				case GameStart:
					returnConf->gameStart = boost::shared_ptr<GameEvent>( new GameEvent(i.actions()) );
				break;
				
				case GameEnd:
					returnConf->gameEnd = boost::shared_ptr<GameEvent>( new GameEvent(i.actions()) );
				break;
			}
		}
		return returnConf;
	}

	void parseCtfBasesFromLua(const std::string& filename, LevelConfig* config) {
		FILE* f = OpenGameFile(filename, "r");
		if(!f) return;
		
		while(!ferror(f) && !feof(f)) {
			std::string line = ReadUntil(f);
			TrimSpaces(line);
			stringlwr(line);
			
			static const char* parseStart = "ctf.maketeam(";
			static const size_t parseStartLen = strlen(parseStart);
			if(strStartsWith(line, parseStart) && line[line.size()-1] == ')') {
				std::vector<std::string> params = explode( line.substr(parseStartLen, line.size() - parseStartLen - 1), "," );
				if(params.size() != 3)
					warnings << "ctf.maketeam found in " << filename << " but not 3 params: " << line << endl;
				else {
					int iParams[3] = {0,0,0};
					bool failed = false;
					for(short i = 0; i < 3; ++i) {
						iParams[i] = from_string<int>(params[i], failed);
						if(failed) {
							warnings << "failed to parse " << params[i] << " to number in: " << line << endl;
							break;
						}
					}
					if(!failed) {
						if(iParams[2] < 1 || iParams[2] > 4)
							warnings << "team-number must be in [1,4], got: " << line << endl;
						else
							config->teamBases.push_back(SpawnPoint( Vec((float)iParams[0], (float)iParams[1]), iParams[2] ));
					}
				}
			}
		}
		
		fclose(f);
	}
	
	void swapTeam_1_2(std::vector<SpawnPoint>& pts) {
		for(size_t i = 0; i < pts.size(); ++i) {
			if(pts[i].team == 1) pts[i].team = 2;
			else if(pts[i].team == 2) pts[i].team = 1;
		}
	}
	
}

bool VermesLevelLoader::load(CMap* level, std::string const& path)
{
	std::string materialPath = path + "/material";
	
	level->FileName = path;
	
	{
		LocalSetColorDepth cd(8);
		level->material = gfx.loadBitmap(materialPath.c_str());
	}
	
	if (level->material)
	{
		level->setEvents( loadConfig( path + "/config.cfg" ) );
		
		if(level->config() && level->config()->teamBases.size() == 0)
			parseCtfBasesFromLua(path + "/scripts/map_" + GetBaseFilenameWithoutExt(path) + ".lua", level->config() );
		
#ifndef DEDICATED_ONLY
		std::string imagePath = path + "/level";
		
		level->image = gfx.loadBitmap(imagePath.c_str());
		if (level->image)
		{			
			std::string backgroundPath = path + "/background";
			
			level->background = gfx.loadBitmap(backgroundPath.c_str());
			
			std::string paralaxPath = path + "/paralax";
			level->paralax = gfx.loadBitmap(paralaxPath.c_str());
			
			if(!level->paralax)
				notes << "Paralax not loaded" << endl;
			
			std::string lightmapPath = path + "/lightmap";
		
			ALLEGRO_BITMAP* tempLightmap = gfx.loadBitmap(lightmapPath.c_str());
			
			if ( tempLightmap )
			{
				{
					LocalSetColorDepth cd(8);
					level->lightmap = create_bitmap(level->material->w, level->material->h);
				}
				for ( int x = 0; x < level->lightmap->w ; ++x )
				for ( int y = 0; y < level->lightmap->h ; ++y )
				{
					putpixel( level->lightmap, x, y, /*getg(*/getpixel(tempLightmap, x, y)/*)*/ );
				}
				destroy_bitmap( tempLightmap );
			}
			
		}
#endif		
		if(level->config()) {
			// seems that Gusanos interprets first team as red and second as blue
			// in OLX, it is the opposite, thus we swap these teams to have colors nicer
			swapTeam_1_2(level->config()->teamBases);
			swapTeam_1_2(level->config()->spawnPoints);		
		}
		
		level->loaderSucceeded();
		level->m_gusLoaded = true;
		return true;
	}
	
	errors << "VermesLevelLoader: none of " << materialPath << "{.bmp,.png,''} found" << endl;
	
	level->gusShutdown();
	return false;
}

const char* VermesLevelLoader::getName()
{
	return "Vermes 0.9 level loader";
}

std::string VermesLevelLoader::format() { return "Gusanos 0.9 level"; }
std::string VermesLevelLoader::formatShort() { return "Gus"; }


#ifndef DEDICATED_ONLY
VermesFontLoader VermesFontLoader::instance;

bool VermesFontLoader::canLoad(std::string const& path, std::string& name)
{
	if(fs::extension(path) == ".bmp" || fs::extension(path) == ".png")
	{
		name = GetBaseFilenameWithoutExt(path);
		return true;
	}
	return false;
}
	
bool VermesFontLoader::load(Font* font, std::string const& path)
{
	font->free();
	
	{
		LocalSetColorDepth cd(8);
		LocalSetColorConversion cc( /*COLORCONV_REDUCE_TO_256 | COLORCONV_KEEP_TRANS*/ 0);
	
		font->m_bitmap = load_bitmap(path.c_str(), 0);
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
		/*ALLEGRO_BITMAP* character = create_bitmap(width,tempBitmap->h);
		blit(tempBitmap,character,i * width,0,0,0,width,character->h);*/
		font->m_chars.push_back(Font::CharInfo(Rect(x, 0, x + monoWidth, monoHeight), 0));
		
		x += monoWidth;
	}
	
	font->buildSubBitmaps();
	
	return true;
	/*
	ALLEGRO_BITMAP *tempBitmap = load_bmp(path.native_file_string().c_str(),0);
	if (tempBitmap)
	{
		int width = tempBitmap->w / 256;
		for (int i = 0; i < 256; ++i)
		{
			ALLEGRO_BITMAP* character = create_bitmap(width,tempBitmap->h);
			blit(tempBitmap,character,i * width,0,0,0,width,character->h);
			font->m_char.push_back(character);
		}
		destroy_bitmap(tempBitmap);
		return true;
	}

	return false;*/
}

const char* VermesFontLoader::getName()
{
	return "Vermes 0.9 font loader";
}

std::string VermesFontLoader::format() { return "Gusanos 0.9 font"; }
std::string VermesFontLoader::formatShort() { return "Gus"; }

XMLLoader XMLLoader::instance;

bool XMLLoader::canLoad(std::string const& path, std::string& name)
{
	if(fs::extension(path) == ".xml")
	{
		name = GetBaseFilenameWithoutExt(path);
		return true;
	}
	return false;
}
	
bool XMLLoader::load(XMLFile* xml, std::string const& path)
{
	OpenGameFileR(xml->f, path, std::ios::binary);
	
	if(!xml->f)
		return false;
		
	return true;
}

const char* XMLLoader::getName()
{
	return "XML loader";
}

std::string XMLLoader::format() { return "Gusanos XML"; }
std::string XMLLoader::formatShort() { return "GusXML"; }



GSSLoader GSSLoader::instance;

bool GSSLoader::canLoad(std::string const& path, std::string& name)
{
	if(fs::extension(path) == ".gss")
	{
		name = GetBaseFilenameWithoutExt(path);
		return true;
	}
	return false;
}
	
bool GSSLoader::load(GSSFile* gss, std::string const& path)
{
	std::ifstream f;
	OpenGameFileR(f, path, std::ios::binary);
	
	if(!f)
		return false;
	
	OmfgGUI::menu.loadGSS(f, path);
		
	return true;
}

const char* GSSLoader::getName()
{
	return "GSS loader";
}

std::string GSSLoader::format() { return "Gusanos GSS"; }
std::string GSSLoader::formatShort() { return "GSS"; }


#endif

LuaLoader LuaLoader::instance;

bool LuaLoader::canLoad(std::string const& path, std::string& name)
{
	if(fs::extension(path) == ".lua")
	{
		name = GetBaseFilenameWithoutExt(path);
		return true;
	}
	return false;
}
	
bool LuaLoader::load(Script* script, std::string const& path)
{
	std::ifstream f;
	OpenGameFileR(f, path, std::ios::binary | std::ios::in);
	if(!f)
		return false;
		
	// Create the table to store the functions in	
	std::string name = GetBaseFilenameWithoutExt(path);
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
	
	lua.load(path, f);
	
	script->lua = &lua;
	script->table = name;
	
	return true;
}

const char* LuaLoader::getName()
{
	return "Lua loader";
}

std::string LuaLoader::format() { return "Gusanos Lua"; }
std::string LuaLoader::formatShort() { return "Lua"; }


/*
VermesParticleLoader VermesParticleLoader::instance;

bool VermesParticleLoader::canLoad(std::string const& path, std::string& name)
{
	if(fs::extension(path) == ".obj")
	{
		name = path.leaf();
		return true;
	}
	return false;
}*/
