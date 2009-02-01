#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>

// A hack to prevent SDL overwriting our main
#define _SDL_main_h

#include "Version.h"
#include "StringUtils.h"

bool error = false;


//////////////////////
// Get the revision number for the given directory
int getRevNumber(const std::string& dir)
{
	// Open the SVN file
	std::ifstream fp((dir + ".svn/entries").c_str());
	if (!fp.is_open())  {
		error = true;
		std::cout << "Could not open the SVN file for reading" << std::endl;
		return 0;
	}

	// Get the fourth line (contains revision)
	std::string ln;
	for (int i = 0; i < 4; i++)  {
		if (!fp.good())  {
			error = true;
			std::cout << "The SVN file is too short" << std::endl;
			return 0;
		}
		std::getline(fp, ln);
	}

	// Convert to number
	bool fail = false;
	int res = from_string<int>(ln, fail);
	if (fail)  {
		error = true;
		std::cout << "The revision number has a bad format" << std::endl;
		return 0;
	}

	return res;
}

//////////////////////
// Returns true if the revision in the output file is up to date
bool hasFileRevision(const std::string& file, int revision)
{
	// Open
	std::ifstream fp(file.c_str());
	if (!fp.is_open())
		return false;

	// Get the string
	std::string line;
	std::getline(fp, line);

	// Close
	fp.close();

	// Make sure it contains the correct define
	if (line.find("#define LX_VERSION ") == std::string::npos)
		return false;

	// Starting quotes
	size_t pos1 = line.find('\"');
	if (pos1 == std::string::npos)
		return false;

	// Ending quotes
	size_t pos2 = line.find('\"', pos1 + 1);
	if (pos2 == std::string::npos)
		return false;

	// _rXXXX string
	size_t pos3 = line.find("_r", pos1);
	if (pos3 == std::string::npos)
		return false;

	// Get the revision number from the file
	pos3 += 2;
	bool fail = false;
	int rev = from_string<int>(line.substr(pos3, pos3 > pos2 ? 0 : pos2 - pos3), fail);
	if (fail)
		return false;

	// Compare the revisions
	return rev == revision;
}

////////////////////
// Saves the file
void saveFile(std::string& data, const std::string& file)
{
	std::ofstream fp(file.c_str());
	if (!fp.is_open())  {
		error = true;
		std::cout << "Cannot open the file " << file << " for writing" << std::endl;
		return;
	}

	// Write
	fp << data;

	fp.close();
}

////////////////////
// Main entry point
int main(int argc, char *argv[])
{
	if (argc < 3)  {
		error = true;
		std::cout << "Not enough parameters" << std::endl;
		return -1;
	}

	// Params
	std::string file = argv[1];
	std::string dir = argv[2];
	if (*dir.rbegin() != '\\' && *dir.rbegin() != '/')
		dir += '/';

	// Revision number
	int revision = getRevNumber(dir);

	// Check if we need to do anything
	if (hasFileRevision(file, revision))  {
		std::cout << "The revision file is up to date (at revision " << revision << ")" << std::endl;
		return 0;
	}

	// Save the file
	if (!error)  {
		saveFile("#define LX_VERSION \"" + std::string(LX_VERSION) + "_r" + to_string(revision) + "\"", file);
		std::cout << "The revision file successfully updated to revision " << revision << std::endl;
	}

	return 0;
}