#ifndef RESOURCE_LIST_H
#define RESOURCE_LIST_H

#include <map>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include "util/macros.h"
#include "Debug.h"
#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

/*
namespace fs {
	struct path {
		std::string p;
		path(const std::string& _p) : p(_p) {}
		path operator/(const path& op) { return p + "/" + op.p; }
		std::string native_file_string() const { return p; }
	};
}
*/

using std::cerr;
using std::endl;

template<typename T1>
class ResourceList
{
public:

	typedef std::map<std::string, T1*> MapT;
	
	ResourceList()
	: m_locked(false)
	{
		
	}
	
	void clear()
	{
		m_paths.clear();
		typename MapT::iterator item = m_resItems.begin();
		for (; item != m_resItems.end(); ++item)
		{
			item->second->deleteThis();
		}
		m_resItems.clear();
		m_locked = false;
		m_resItemsIndex.clear();
	}
	
	void addPath(std::string const& path)
	{
		if(std::find(m_paths.begin(), m_paths.end(), path) == m_paths.end())
			m_paths.push_back(path);
	}
	
	bool load(std::string const& name, T1& resource)
	{
		std::list<std::string>::iterator i = m_paths.begin();
		for(; i != m_paths.end(); ++i)
		{
			if(resource.load(*i + "/" + name))
				return true;
		}
		return false;
	}
		
	T1* load( std::string const& filename, bool suppressError = false )
	{
		if ( m_locked )
		{
			errors << "Attempt to load resource after indexation" << endl;
			return NULL;
		}

		typename MapT::iterator item = m_resItems.find(filename);
		if (item != m_resItems.end())
		{
			return item->second;
		}
		else
		{
			T1 *i = new T1;
			m_resItems[filename] = i;

			if(load(filename, *i))
			{
				return i;
			}
			else
			{
				i->deleteThis();
				item = m_resItems.find(filename);
				m_resItems.erase(item);
				if(!suppressError)
					errors << "Gus Resource: Could not load " << filename << endl;
				return NULL;
			}
		}
	}
	
	void think()
	{
		typename MapT::iterator i = m_resItems.begin();
		for (; i != m_resItems.end(); ++i)
		{
			i->second->think();
		}
	}
	
	void indexate()
	{
		m_locked = true;
		typename MapT::iterator item = m_resItems.begin();
		for( size_t i = 0; item != m_resItems.end() ; ++item, ++i )
		{
			m_resItemsIndex.push_back( item->second );
			item->second->setIndex(i);
		}
	}
	
	uint32_t crc(bool posIndependent = true)
	{
		if(posIndependent) // crc does not depend on the filename ordering
		{
			uint32_t v = 0;
			const_foreach(i, m_resItems)
			{
				v ^= i->second->crc;
			}
			return v;
		}
		else // crc depends on the filename ordering
		{
			uint32_t v = 0;
			
			const_foreach(i, m_resItems)
			{
				v ^= i->second->crc;
				++v;
			}
			return v;
		}
	}
	
	size_t size() const
	{
		return m_resItemsIndex.size();
	}
	
	T1* operator[]( size_t index ) const
	{
		if( index < m_resItemsIndex.size() )
		{
			return m_resItemsIndex[index];
		}
		else
			return 0;
	}
	
private:
	
	bool m_locked;
	std::vector<T1*> m_resItemsIndex;
	MapT m_resItems;
	std::list<std::string>     m_paths; // Paths to scan
};

#endif // _RESOURCE_LIST_H_
