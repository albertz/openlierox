/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Config file handler
// Created 30/9/01
// By Jason Boettcher


#include <map>
#include <string>
#include "LieroX.h"
#include "ConfigHandler.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "MathLib.h"



typedef std::map<std::string, int, stringcaseless> KeywordMap;
static KeywordMap Keywords;


// Internal
static bool	GetString(const std::string& filename, const std::string& section, const std::string& key, std::string& string, bool abs_fn = false);


///////////////////
// Add a keyword to the list
bool AddKeyword(const std::string& key, int value)
{
	Keywords[key] = value;
	
	return true;
}



///////////////////
// Read a keyword from a file
bool ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv)
{
	std::string string;

	*value = defaultv;

	if(!GetString(filename,section,key,string))
		return false;

	// Try and find a keyword with matching keys
	KeywordMap::iterator f = Keywords.find(string);
	if(f != Keywords.end()) {
		//notes << filename << ":" << section << "." << key << ": " << f->first << "(" << string << ") = " << f->second << endl;
		*value = f->second;
		return true;
	}
	
	warnings << filename << ":" << section << "." << key << ": '" << string << "' is an unknown keyword" << endl;
	
	return false;
}

///////////////////
// Read a keyword from a file (bool version)
bool ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, bool *value, bool defaultv)
{
	int v = defaultv ? 1 : 0;
	bool ret = ReadKeyword(filename, section, key, &v, defaultv ? 1 : 0);
	*value = v != 0;
	return ret;
}

///////////////////
// Read bit flags specified by keywords from a file
bool ReadKeywordList(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv)
{
	std::string string;

	*value = defaultv;

	if(!GetString(filename,section,key,string))
		return false;

	std::vector<std::string> split = explode(string, ",");
	for (std::vector<std::string>::iterator it = split.begin(); it != split.end(); it++)  {
		TrimSpaces(*it);
		KeywordMap::iterator key = Keywords.find(*it);
		if (key != Keywords.end())
			*value |= key->second;
	}

	return true;
}


///////////////////
// Read an interger from a file
bool ReadInteger(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv)
{
	std::string string;

	*value = defaultv;

	if(!GetString(filename,section,key,string))
		return false;

	*value = from_string<int>(string);

	return true;
}


///////////////////
// Read a string from a file
bool ReadString(const std::string& filename, const std::string& section, const std::string& key, std::string& value, std::string defaultv, bool abs_fn)
{
	value = defaultv;

	return GetString(filename,section,key,value, abs_fn);

	/*int result = GetString(filename,section,key,value);

	if (strlen(value) <= 0)
		strcpy(value,defaultv);

	return result;*/
}



///////////////////
// Read a float from a file
bool ReadFloat(const std::string& filename, const std::string& section, const std::string& key, float *value, float defaultv)
{
	std::string string;

	*value = defaultv;

	if(!GetString(filename,section,key,string))
		return false;

	*value = (float)atof(string);

	return true;
}



bool ReadColour(const std::string& filename, const std::string& section, const std::string& key, Color& value, const Color& defaultv) {
	std::string string;
	
	value = defaultv;
	
	if(!GetString(filename,section,key,string))
		return false;
	
	value = StrToCol(string);
	
	return true;
}

//////////////////
// Reads an array of integers
bool ReadIntArray(const std::string& filename, const std::string& section, const std::string& key, int *array, int num_items)
{
	std::string string;

	if (!GetString(filename,section,key,string))
		return false;

	std::vector<std::string> arr = explode(string,",");
	for (register int i=0; i<MIN(num_items,(int)arr.size()); i++)
		array[i] = from_string<int>(arr[i]);

	return num_items == (int)arr.size();
}



///////////////////
// Read a string
static bool GetString(const std::string& filename, const std::string& section, const std::string& key, std::string& string, bool abs_fn)
{
	FILE	*config = NULL;
	std::string	Line;
	std::string	tmpLine;
	std::string	curSection;
	std::string	temp;
	std::string	curKey;
	size_t	chardest = 0;
	int		Position;
	bool	found = false;

	if(filename == "")
		return false;

	if(abs_fn) {
		config = fopen(Utf8ToSystemNative(filename).c_str(), "rt");
	} else
		config = OpenGameFile(filename,"rt");
	if(!config)
		return false;

	//string="";
	curSection="";
	temp="";
	curKey="";

	// Check for UTF-8 encoded file and skip the UTF-8 mark if it is
	uchar utf8mark[3];
	if(fread(utf8mark, sizeof(utf8mark)/sizeof(uchar), 1, config) == 0) {
		fclose(config);
		return false;
	}
	if (utf8mark[0] != 0xEF || utf8mark[1] != 0xBB || utf8mark[2] != 0xBF)
		fseek(config, 0, SEEK_SET); // Not a UTF-8 file, jump back to the beginning


	while(!feof(config) && !ferror(config))
	{
		// Parse the lines
		Line = ReadUntil(config, '\n');
		TrimSpaces(Line);

		///////////////////
		// Comment, Ignore
		if(Line.size() == 0 || Line[0] == '#')
			continue;

		////////////
		// Sections
		if(Line[0] == '[' && Line[Line.size()-1] == ']')
		{
			curSection = Line.substr(1);
			curSection.erase(curSection.size()-1);
			continue;
		}

		////////
		// Keys
		chardest = Line.find('=');
		if(chardest != std::string::npos)
		{
			// Key
			Position = (int)chardest;
			tmpLine = Line;
			tmpLine.erase(Position);
			TrimSpaces(tmpLine);
			curKey = tmpLine;

			// Check if this is the key were looking for under the section were looking for
			if(stringcasecmp(curKey,key) == 0 && stringcasecmp(curSection,section) == 0)
			{
				// Get the value
				tmpLine = Line.substr(Position+1);
				TrimSpaces(tmpLine);
				string = tmpLine;
				found = true;
				break;
			}
			continue;
		}
	}

	fclose(config);

	return found;
}


