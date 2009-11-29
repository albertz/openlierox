#include "vermes.h"
#include "../gfx.h"
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

#include <iostream>
using namespace std;

VermesLevelLoader VermesLevelLoader::instance;

bool VermesLevelLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::exists(path / "config.cfg"))
	{
		name = path.leaf();
		return true;
	}
	return false;
}
	
bool VermesLevelLoader::load(Level* level, fs::path const& path)
{
	std::string materialPath = (path / "material").native_file_string();
	
	level->path = path.native_directory_string();
	
	{
		LocalSetColorDepth cd(8);
		level->material = gfx.loadBitmap(materialPath.c_str(), 0);
	}
	
	if (level->material)
	{

		level->loaderSucceeded();
		return true;

	}
	level->unload();
	return false;
}

const char* VermesLevelLoader::getName()
{
	return "Vermes level loader";
}
