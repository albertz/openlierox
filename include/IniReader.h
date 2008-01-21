/*
	OpenLieroX
	
	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/

#ifndef __INI_READER_H__
#define __INI_READER_H__

#include <string>

// If the return value is false, the parsing will break 
typedef bool (*OnSection) (const std::string& section, void *userData);
typedef bool (*OnEntry) (const std::string& section, const std::string& propname, const std::string& value, void *userData);

// Ini reader class
class IniReader {
public:
	IniReader(const std::string& filename, OnEntry newEntryCallback = NULL, OnSection newSectionCallback = NULL);
	~IniReader();

private:
	std::string m_filename;

public:
	OnSection	onSection;
	OnEntry		onEntry;

	bool Parse(void *userData = NULL);
};

#endif

