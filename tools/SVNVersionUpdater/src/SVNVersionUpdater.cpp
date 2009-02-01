#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>

template<typename T>
T from_string(const std::string& s, bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> t).fail();
	return t;
}

template<typename T>
std::string to_string(T val) {
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

bool error = false;

std::vector<std::string> getFileContents(const std::string& fname)
{
	std::vector<std::string> result;

	std::ifstream fp(fname.c_str());
	if (!fp.is_open())  {
		error = true;
		std::cout << "Could not open the file for reading" << std::endl;
		return result;
	}

	// Read
	while (fp.good())  {
		std::string ln;
		std::getline(fp, ln);
		result.push_back(ln);
	}

	fp.close();

	return result;
}

int getRevNumber(const std::string& dir)
{
	// Open the SVN file
	std::ifstream fp((dir + ".svn/entries").c_str());
	if (!fp.is_open())  {
		error = true;
		std::cout << "Could not open the SVN file to get the revision" << std::endl;
		return 0;
	}

	// Get the fourth line (contains revision)
	std::string ln;
	for (int i = 0; i < 4; i++)  {
		if (!fp.good())  {
			error = true;
			std::cout << "The entries SVN file is too short" << std::endl;
			return 0;
		}
		std::getline(fp, ln);
	}

	// Convert to number
	bool fail = false;
	int res = from_string<int>(ln, fail);
	if (fail)  {
		error = true;
		std::cout << "The revision number has an incorrect format" << std::endl;
		return 0;
	}

	return res;
}

void addRevToFile(std::vector<std::string>& data, const std::string& fname, int revision)
{
	// Find the correct line
	size_t line = 1;
	for (std::vector<std::string>::iterator it = data.begin(); it != data.end(); it++)  {
		if (it->find('#') != std::string::npos && it->find("LX_VERSION") != std::string::npos &&
			it->find("define") != std::string::npos)  {
			
			// Update the line

			// Text in quotes is the one we're looking for
			size_t pos1 = it->find('\"');
			if (pos1 == std::string::npos)  {
				error = true;
				std::cout << "First quote not found" << std::endl;
				break;
			}
			size_t pos2 = it->find('\"', pos1 + 1);
			if (pos2 == std::string::npos)  {
				error = true;
				std::cout << "Second quote not found" << std::endl;
				break;
			}

			// Check if the version already contains the revision, if so, remove it first
			size_t pos3 = it->find("_r", pos1 + 1);
			if (pos3 != std::string::npos)  {
				std::cout << fname << "(" << line << ") : warning S0001: " << "Revision number is already present, it will be overwriten" << std::endl;
				it->erase(pos3, pos2 - pos3);
			}

			// Insert the rev number before the second quote
			it->insert(pos2, "_r" + to_string(revision));

			return;
		}

		++line;
	}

	std::cout << "The version string not found" << std::endl;
}

void removeRevFromFile(std::vector<std::string>& data, const std::string& file)
{
	// Find the correct line
	size_t line = 1;
	for (std::vector<std::string>::iterator it = data.begin(); it != data.end(); it++)  {
		if (it->find('#') != std::string::npos && it->find("LX_VERSION") != std::string::npos &&
			it->find("define") != std::string::npos)  {
			
			// Update the line

			// Text in quotes is the one we're looking for
			size_t pos1 = it->find('\"');
			if (pos1 == std::string::npos)  {
				error = true;
				std::cout << "First quote not found" << std::endl;
				break;
			}
			size_t pos2 = it->find('\"', pos1 + 1);
			if (pos2 == std::string::npos)  {
				error = true;
				std::cout << "Second quote not found" << std::endl;
				break;
			}

			size_t pos3 = it->find("_r", pos1 + 1);
			if (pos3 == std::string::npos)  {
				error = true;
				std::cout << "Revision number not found";
				return;
			}

			// Erase the revision number
			it->erase(pos3, pos2 - pos3);

			return;
		}

		++line;
	}

	std::cout << "The version string not found" << std::endl;
}

void saveFile(std::vector<std::string>& data, const std::string& file)
{
	std::ofstream fp(file.c_str());
	if (!fp.is_open())  {
		error = true;
		std::cout << "Could not open the file for writing" << std::endl;
		return;
	}

	// Write the lines
	for (std::vector<std::string>::iterator it = data.begin(); it != data.end(); it++)  {
		fp << *it << std::endl;
	}

	fp.close();
}

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

	bool revert = false;
	if (argc >= 4)
		revert = std::string(argv[3]) == "-r";

	// Contents of Version.h
	std::vector<std::string> file_contents = getFileContents(file);

	int revision = 0;
	if (revert)  {
		// Remove the revision number
		removeRevFromFile(file_contents, file);
	} else {
		// Revision number
		revision = getRevNumber(dir);

		// Set the number
		if (revision > 0)
			addRevToFile(file_contents, file, revision);
	}

	// Save the file
	saveFile(file_contents, file);

	// Report
	if (!error)  {
		if (!revert)
			std::cout << "The revision number " << revision << " successfully added." << std::endl;
		else
			std::cout << "The revision number successfully removed." << std::endl;
	}

	return 0;
}