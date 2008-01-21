/*
	OpenLieroX

	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/

#include <iostream>

#include "IniReader.h"
#include "FindFile.h"

using namespace std;

///////////////////
// Constructor and destructor
IniReader::IniReader(const std::string& filename, OnEntry newEntryCallback,  OnSection newSectionCallback) :
	m_filename(filename),
	onSection(newSectionCallback),
	onEntry(newEntryCallback) {}

IniReader::~IniReader() {}

///////////////////
// Main parsing function, returns true if the file was parsed successfully
bool IniReader::Parse(void *userData) {
	FILE* f = OpenGameFile(m_filename, "r");
	if(f == NULL)
		return false;

	enum ParseState {
		S_DEFAULT, S_IGNORERESTLINE, S_PROPNAME, S_PROPVALUE, S_SECTION };
	ParseState state = S_DEFAULT;
	std::string propname;
	std::string section;
	std::string value;
	
	while(!feof(f)) {
		unsigned char c;
		fread(&c, 1, 1, f);

		switch(state) {
		case S_DEFAULT:
			if(c >= 128) break; // just ignore unicode-stuff when we are in this state (UTF8 bytes at beginning are also handled by this)
			else if(isspace(c)) break; // ignore spaces and newlines
			else if(c == '#') { state = S_IGNORERESTLINE; /* this is a comment */ break; }
			else if(c == '[') { state = S_SECTION; section = ""; break; }
			else if(c == '=') {
				cout << "WARNING: \"=\" is not allowed as the first character in a line of options.cfg" << endl;
				break; /* ignore */ }
			else { state = S_PROPNAME; propname = c; break; }

		case S_SECTION:
			if(c == ']') {
				if (onSection)
					if( ! onSection(section, userData) ) goto parseCleanup;
				state = S_DEFAULT; break; }
			else if(c == '\n') {
				cout << "WARNING: section-name \"" << section << "\" of options.cfg is not closed correctly" << endl;
				state = S_DEFAULT; break; }
			else if(isspace(c)) {
				cout << "WARNING: section-name \"" << section << "\" of options.cfg contains a space" << endl;
				break; /* ignore */ }
			else { section += c; break; }
			
		case S_PROPNAME:
			if(c == '\n') {
				cout << "WARNING: property \"" << propname << "\" of options.cfg incomplete" << endl;
				state = S_DEFAULT; break; }
			else if(isspace(c)) break; // just ignore spaces
			else if(c == '=') { state = S_PROPVALUE; value = ""; break; }
			else { propname += c; break; }
			
		case S_PROPVALUE:
			if(c == '\n' || c == '#') {
				if (onEntry)
					if( ! onEntry(section, propname, value, userData) ) goto parseCleanup;
				if(c == '#') state = S_IGNORERESTLINE; else state = S_DEFAULT;
				break; }
			else if(isspace(c) && value == "") break; // ignore heading spaces
			else { value += c; break; }
		
		case S_IGNORERESTLINE:
			if(c == '\n') state = S_DEFAULT;
			break; // ignore everything
		}
	}
	
parseCleanup:
	fclose(f);
	
	return true;
}
