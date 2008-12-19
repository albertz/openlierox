#include "reader.hpp"
#include "filesystem.hpp"
#include <SDL/SDL.h>
#include <map>
#include <stdexcept>

std::string lieroOPT;
std::string lieroEXERoot;

namespace
{

struct ReaderFile
{
	unsigned int lastTouch;
	FILE* f;
};

typedef std::map<std::string, ReaderFile> ReaderFileMap;

std::string lieroEXE;
std::string lieroCHR;
std::string lieroSND;

ReaderFileMap readerFiles;

void closeReaderFile(ReaderFileMap::iterator i)
{
	fclose(i->second.f);
	readerFiles.erase(i);
}


}

FILE* openFile(std::string const& name)
{
	ReaderFileMap::iterator i = readerFiles.find(name);
	if(i != readerFiles.end())
	{
		i->second.lastTouch = SDL_GetTicks();
		return i->second.f;
	}

	FILE* f = tolerantFOpen(name.c_str(), "rb");
	if(!f)
		throw std::runtime_error("Could not open '" + name + '\'');
	ReaderFile& rf = readerFiles[name];
	rf.f = f;
	rf.lastTouch = SDL_GetTicks();
	return f;
}

FILE* openLieroEXE()
{
	return openFile(lieroEXE);
}

FILE* openLieroSND()
{
	return openFile(lieroSND);
}

FILE* openLieroCHR()
{
	return openFile(lieroCHR);
}

void processReader()
{
	unsigned int now = SDL_GetTicks();
	for(ReaderFileMap::iterator i = readerFiles.begin(); i != readerFiles.end(); )
	{
		ReaderFileMap::iterator cur = i;
		++i;
		
		if((now - cur->second.lastTouch) > 5000)
		{
			closeReaderFile(cur);
		}
	}
}

void closeAllCachedFiles()
{
	for(ReaderFileMap::iterator i = readerFiles.begin(); i != readerFiles.end(); )
	{
		ReaderFileMap::iterator cur = i;
		++i;
		closeReaderFile(cur);
	}
}

void setLieroEXE(std::string const& path)
{
	//TODO: Close cached files
	
	lieroEXE = path;
	lieroCHR = changeLeaf(path, "LIERO.CHR");
	lieroSND = changeLeaf(path, "LIERO.SND");
	lieroOPT = changeLeaf(path, "LIERO.OPT");
	
	lieroEXERoot = getRoot(lieroEXE);
}
