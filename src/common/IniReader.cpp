/*
	OpenLieroX

	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/




#include "IniReader.h"
#include "FindFile.h"
#include "Debug.h"


IniReader::KeywordList IniReader::DefaultKeywords;


IniReader::IniReader(const std::string& filename, KeywordList& keywords) : m_filename(filename), m_keywords(keywords) { 
	if (IniReader::DefaultKeywords.empty())  {
		(IniReader::DefaultKeywords)["true"] = true;
		(IniReader::DefaultKeywords)["false"] = false;
	}
}

IniReader::~IniReader() {}

bool IniReader::Parse() {
	FILE* f = OpenGameFile(m_filename, "r");
	if(f == NULL)
		return false;

	bool res = true;
	enum ParseState {
		S_DEFAULT, S_IGNORERESTLINE, S_PROPNAME, S_PROPVALUE, S_SECTION };
	ParseState state = S_DEFAULT;
	std::string propname;
	std::string section;
	std::string value;
	
	while(!feof(f) && !ferror(f)) {
		unsigned char c = 0;
		if(fread(&c, 1, 1, f) == 0) break;

		if(c == '\r') continue; // ignore this
		
		switch(state) {
		case S_DEFAULT:
			if(c >= 128) break; // just ignore unicode-stuff when we are in this state (UTF8 bytes at beginning are also handled by this)
			else if(isspace(c)) break; // ignore spaces and newlines
			else if(c == '#') { state = S_IGNORERESTLINE; /* this is a comment */ break; }
			else if(c == '[') { state = S_SECTION; section = ""; break; }
			else if(c == '=') {
				warnings << "WARNING: \"=\" is not allowed as the first character in a line of " << m_filename << endl;
				break; /* ignore */ }
			else { state = S_PROPNAME; propname = c; break; }

		case S_SECTION:
			if(c == ']') {
				if( ! OnNewSection(section) )  { res = false; goto parseCleanup; }
				state = S_DEFAULT; NewSection(section); break; }
			else if(c == '\n') {
				warnings << "WARNING: section-name \"" << section << "\" of " << m_filename << " is not closed correctly" << endl;
				state = S_DEFAULT; break; }
			else if(isspace(c)) {
				warnings << "WARNING: section-name \"" << section << "\" of " << m_filename << " contains a space" << endl;
				break; /* ignore */ }
			else { section += c; break; }
			
		case S_PROPNAME:
			if(c == '\n') {
				warnings << "WARNING: property \"" << propname << "\" of " << m_filename << " incomplete" << endl;
				state = S_DEFAULT; break; }
			else if(isspace(c)) break; // just ignore spaces
			else if(c == '=') { state = S_PROPVALUE; value = ""; break; }
			else { propname += c; break; }
			
		case S_PROPVALUE:
			if(c == '\n' || c == '#') { 
				if( ! OnEntry(section, propname, value) ) { res = false; goto parseCleanup; }
				NewEntryInSection(propname, value);
				if(c == '#') state = S_IGNORERESTLINE; else state = S_DEFAULT;
				break; }
			else if(isspace(c) && value == "") break; // ignore heading spaces
			else { value += c; break; }
		
		case S_IGNORERESTLINE:
			if(c == '\n') state = S_DEFAULT;
			break; // ignore everything
		}
	}

	// In case the endline is missing at the end of file, finish the parsing of the last line
	if (state == S_PROPVALUE)  {
		if( ! OnEntry(section, propname, value) ) { res = false; goto parseCleanup; }
		NewEntryInSection(propname, value);
	}

	// DEBUG: dump the file
	/*notes << "Dump of " << m_filename << endl;
	for (SectionMap::iterator it = m_sections.begin(); it != m_sections.end(); ++it)  {
		notes << "[" << it->first << "]" << endl;
		for (Section::iterator k = it->second.begin(); k != it->second.end(); ++k)
			notes << k->first << "=" << k->second << endl;
		notes << endl;
	}
	notes << endl;*/
	
parseCleanup:
	fclose(f);
	
	return res;
}

void IniReader::NewSection(const std::string& name)
{
	m_curSection = &m_sections[name];
}

void IniReader::NewEntryInSection(const std::string& name, const std::string& value)
{
	if (!m_curSection)  {
		warnings << "Cannot add item " << name << " to any section, because the current section is unset" << endl;
		return;
	}

	(*m_curSection)[name] = value;
}

bool IniReader::GetString(const std::string& section, const std::string& key, std::string& string) const
{
	// Get the section
	SectionMap::const_iterator sect = m_sections.find(section);
	if (sect == m_sections.end())
		return false;

	// Get the key=value pair
	Section::const_iterator item = sect->second.find(key);
	if (item == sect->second.end())
		return false;

	string = item->second;
	return true;
}

bool IniReader::ReadString(const std::string& section, const std::string& key, std::string& value, std::string defaultv) const
{
	bool res = GetString(section, key, value);
	if (!res)
		value = defaultv;
	return res;
}

bool IniReader::ReadInteger(const std::string& section, const std::string& key, int *value, int defaultv) const
{
	std::string string;

	*value = defaultv;

	if(!GetString(section, key, string))
		return false;

	*value = from_string<int>(string);

	return true;
}

bool IniReader::ReadFloat(const std::string &section, const std::string &key, float *value, float defaultv) const
{
	std::string string;

	*value = defaultv;

	if(!GetString(section, key, string))
		return false;

	*value = (float)atof(string);

	return true;
}

bool IniReader::ReadColour(const std::string &section, const std::string &key, Color &value, const Color &defaultv) const
{
	std::string string;
	
	value = defaultv;
	
	if(!GetString(section, key, string))
		return false;
	
	value = StrToCol(string);
	
	return true;
}

bool IniReader::ReadIntArray(const std::string &section, const std::string &key, int *array, int num_items) const
{
	std::string string;

	if (!GetString(section, key, string))
		return false;

	std::vector<std::string> arr = explode(string,",");
	for (int i=0; i < MIN(num_items,(int)arr.size()); i++)  {
		TrimSpaces(arr[i]);
		array[i] = from_string<int>(arr[i]);
	}

	return num_items == (int)arr.size();
}

bool IniReader::ReadKeyword(const std::string &section, const std::string &key, int *value, int defaultv) const
{
	std::string string;

	*value = defaultv;

	if(!GetString(section, key, string))
		return false;

	// Try and find a keyword with matching keys
	KeywordList::const_iterator f = m_keywords.find(string);
	if(f != m_keywords.end()) {
		//notes << filename << ":" << section << "." << key << ": " << f->first << "(" << string << ") = " << f->second << endl;
		*value = f->second;
		return true;
	}
	
	warnings << m_filename << ":" << section << "." << key << ": '" << string << "' is an unknown keyword" << endl;
	
	return false;
}

bool IniReader::ReadKeyword(const std::string &section, const std::string &key, bool *value, bool defaultv) const
{
	int v = defaultv ? 1 : 0;
	bool ret = ReadKeyword(section, key, &v, defaultv ? 1 : 0);
	*value = v != 0;
	return ret;
}

bool IniReader::ReadKeywordList(const std::string &section, const std::string &key, int *value, int defaultv) const
{
	std::string string;

	*value = defaultv;

	if (!GetString(section, key, string))
		return false;

	std::vector<std::string> split = explode(string, ",");
	for (std::vector<std::string>::iterator it = split.begin(); it != split.end(); it++)  {
		TrimSpaces(*it);
		KeywordList::const_iterator key = m_keywords.find(*it);
		if (key != m_keywords.end())
			*value |= key->second;
	}

	return true;
}
