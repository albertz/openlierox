#ifndef LIERO_FILESYSTEM_HPP
#define LIERO_FILESYSTEM_HPP

#include <string>
#include <cstdio>
#include <memory>
#include "OLXModInterface.h"
using namespace OlxMod;

std::string changeLeaf(std::string const& path, std::string const& newLeaf);
std::string getRoot(std::string const& path);
std::string getBasename(std::string const& path);
std::string getExtension(std::string const& path);
void toUpperCase(std::string& str);
std::string joinPath(std::string const& root, std::string const& leaf);

bool fileExists(std::string const& path);

FILE* tolerantFOpen(std::string const& name, char const* mode);

std::size_t fileLength(FILE* f);

struct dir_itr_imp;
typedef std::auto_ptr<dir_itr_imp> dir_itr_imp_ptr;


void dir_itr_init( dir_itr_imp_ptr & m_imp, char const* dir_path );
std::string& dir_itr_dereference(dir_itr_imp_ptr const& m_imp );
void dir_itr_increment( dir_itr_imp_ptr & m_imp );

struct DirectoryIterator
{
	dir_itr_imp_ptr m_imp;
	
	DirectoryIterator(std::string const& dir);
	~DirectoryIterator();
	
	operator void*()
	{
		return m_imp.get();
	}
	
	std::string const& operator*() const
	{
		return dir_itr_dereference( m_imp );
	}
	
	void operator++()
	{
		dir_itr_increment(m_imp);
	}
};

struct ScopedFile
{
	ScopedFile(FILE* f)
	: f(f)
	{
	}
	
	~ScopedFile()
	{
		if(f) fclose(f);
	}
	
	operator FILE*()
	{
		return f;
	}
	
	FILE* f;
};

#endif // LIERO_FILESYSTEM_HPP
