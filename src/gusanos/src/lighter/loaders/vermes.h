#ifndef VERMES_LOADERS_VERMES_H
#define VERMES_LOADERS_VERMES_H

#include "../level.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

struct VermesLevelLoader
{
	virtual bool canLoad(fs::path const& path, std::string& name);
	
	virtual bool load(Level*, fs::path const& path);
	
	virtual const char* getName();
	
	static VermesLevelLoader instance;
	
	virtual ~VermesLevelLoader()
	{
	}
};


#endif //VERMES_LOADERS_VERMES_H
