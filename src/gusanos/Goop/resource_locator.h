#ifndef GUSANOS_RESOURCE_LOCATOR_H
#define GUSANOS_RESOURCE_LOCATOR_H

#include <map>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <stdexcept>
//#include <console.h> //For IStrCompare
#include "util/text.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/utility.hpp>
namespace fs = boost::filesystem;

template<class T, bool Cache = true, bool ReturnResource = true>
struct ResourceLocator
{
	ResourceLocator()
	{
	}
	
	struct BaseLoader
	{
		// Should return true and set name to the resource name 
		// if the file/folder specified by path can be loaded.
		// Otherwise, it should return false.
		virtual bool canLoad(fs::path const& path, std::string& name) = 0;
		
		// Should load the resource located at path
		virtual bool load(T*, fs::path const& path) = 0;
		
		virtual const char* getName() = 0;
		
		virtual ~BaseLoader() { }
	};
	
	// Information about a resource found via refresh()
	struct ResourceInfo
	{
		ResourceInfo() : loader(0), cached(0) {}
		
		ResourceInfo(fs::path const& path_, BaseLoader* loader_)
		: path(path_), loader(loader_), cached(0)
		{

		}
		
		~ResourceInfo()
		{
			delete cached;
		}
				
		fs::path path; // Path to load from
		BaseLoader* loader;   // Loader to use
		T* cached;
	};
	
	typedef std::map<std::string, ResourceInfo, IStrCompare> NamedResourceMap;
	
	// Refreshes the internal resource list by
	// scanning the path list after resources that can
	// be loaded by any of the registered loaders.
	// Folders are tried as well. If a loader is found for
	// a folder, all the subfolders/files are ignored.
	void refresh();
	
	// Clears all resources and paths
	void clear()
	{
		m_paths.clear();
		m_namedResources.clear();
	}
	
	// Clears resources that can be safely removed
	void clearUncached()
	{
		for(typename NamedResourceMap::iterator i = m_namedResources.begin(); i != m_namedResources.end();)
		{
			typename NamedResourceMap::iterator next = boost::next(i);
			
			if(!i->second.cached)
			{
				m_namedResources.erase(i);
			}
			else
			{
				std::cout << "Resource " << i->first << " is cached, leaving it behind" << std::endl;
			}
			
			i = next;
		}
	}
	
	// Adds a path to the path list
	void addPath(fs::path const& path)
	{
		//m_paths.insert(path);
		if(std::find(m_paths.begin(), m_paths.end(), path) == m_paths.end())
			m_paths.push_back(path);
	}
	
	// Loads the named resource
	bool load(T*, std::string const& name);
	
	// Loads and returns the named resource or, if it was loaded already, returns a cached version
	T* load(std::string const& name);
	
	fs::path const& getPathOf(std::string const& name);
	
	// Returns true if the named resource can be loaded
	bool exists(std::string const& name);

	// Registers a new loader for this resource type
	void registerLoader(BaseLoader* loader)
	{
		m_loaders.push_back(loader);
	}
	
	NamedResourceMap const& getMap()
	{
		return m_namedResources;
	}
	
private:
	void refresh(fs::path const& path);
	
	NamedResourceMap m_namedResources; //The resource list
	
	std::list<BaseLoader *> m_loaders; // Registered loaders
	std::list<fs::path>     m_paths; // Paths to scan
};

template<class T, bool Cache, bool ReturnResource>
void ResourceLocator<T, Cache, ReturnResource>::refresh(fs::path const& path)
{
	//std::cout << "Scanning: " << path.native_file_string() << std::endl;
	try
	{
		fs::directory_iterator i(path), e;
		
		for(; i != e; ++i)
		{
			std::string name;
			BaseLoader* loader = 0;
			
			// Try loaders until a working one is found
			
			for(typename std::list<BaseLoader *>::iterator l = m_loaders.begin();
			    l != m_loaders.end();
			    ++l)
			{
				if((*l)->canLoad(*i, name))
				{
					loader = *l;
					break;
				}
			}
						
			if(loader)
			{
				// We found a loader
				std::pair<typename NamedResourceMap::iterator, bool> r = m_namedResources.insert(std::make_pair(name, ResourceInfo(*i, loader)));
				/*
				if(r.second)
				{
					std::cout << "Found resource: " << name << ", loader: " << loader->getName() << std::endl;
				}
				else
				{
					std::cout << "Duplicate resource: " << name << ", old path: " << r.first->second.path.native_file_string() << ", new path: " << i->native_file_string() << std::endl;
				}
				*/
			}
			else if(fs::is_directory(*i))
			{
				// If no loader was found and this is a directory, scan it
				refresh(*i);
			}
		}
	}
	catch(fs::filesystem_error& err)
	{
		std::cout << err.what() << std::endl;
	}
}

template<class T, bool Cache, bool ReturnResource>
void ResourceLocator<T, Cache, ReturnResource>::refresh()
{
	clearUncached();
	
	for(std::list<fs::path>::const_reverse_iterator p = m_paths.rbegin();
	    p != std::list<fs::path>::const_reverse_iterator(m_paths.rend());
	    ++p)
	{
		refresh(*p);
	}
}

template<class T, bool Cache, bool ReturnResource>
bool ResourceLocator<T, Cache, ReturnResource>::load(T* dest, std::string const& name)
{
	typename NamedResourceMap::iterator i = m_namedResources.find(name);
	if(i == m_namedResources.end())
		return false;

	return i->second.loader->load(dest, i->second.path);
}

template<class T, bool Cache, bool ReturnResource>
T* ResourceLocator<T, Cache, ReturnResource>::load(std::string const& name)
{
	if(!ReturnResource)
		return 0;
		
	typename NamedResourceMap::iterator i = m_namedResources.find(name);
	if(i == m_namedResources.end())
		return 0;
	
	if(Cache && i->second.cached)
		return i->second.cached; //Return the cached version
	
	T* resource = new T();
	bool r = i->second.loader->load(resource, i->second.path);
	if(!r)
	{
		// The loader failed, delete the resource
		delete resource;
		return 0;
	}
	
	// Cache the loaded resource
	if(Cache)
		i->second.cached = resource;
	
	return resource;
}

template<class T, bool Cache, bool ReturnResource>
fs::path const& ResourceLocator<T, Cache, ReturnResource>::getPathOf(std::string const& name)
{
	typename NamedResourceMap::iterator i = m_namedResources.find(name);
	if(i == m_namedResources.end())
		throw std::runtime_error("Resource does not exist");
		
	return i->second.path;
}

template<class T, bool Cache, bool ReturnResource>
bool ResourceLocator<T, Cache, ReturnResource>::exists(std::string const& name)
{
	typename NamedResourceMap::iterator i = m_namedResources.find(name);
	if(i == m_namedResources.end())
		return false;
		
	return true;
}

#endif //GUSANOS_RESOURCE_LOCATOR_H
