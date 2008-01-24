/*
	OpenLieroX
	
	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/

#ifndef __INI_READER_H__
#define __INI_READER_H__

#include <string>

/*
	to use this class, you have to create a subclass from it and
	overload the OnNewSection or/and OnEntry
*/
class IniReader {
public:
	IniReader(const std::string& filename);
	virtual ~IniReader();
	
	// returns false if there was an error
	// if you break via the callbacks, this is also an error
	bool Parse();

	// if the return value is false, the parsing will break 
	virtual bool OnNewSection (const std::string& section) { return true; }
	virtual bool OnEntry (const std::string& section, const std::string& propname, const std::string& value) { return true; }

private:
	std::string m_filename;
};

#endif

