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


#ifndef __CONFIGHANDLER_H__
#define __CONFIGHANDLER_H__

#include <string>
#include "CVec.h"
#include <SDL.h>

#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Internal
int		GetString(const std::string& filename, const std::string& section, const std::string& key, std::string& string);


// Value reading
int		ReadString(const std::string& filename, const std::string& section, const std::string& key, std::string& value, const std::string& defaultv);
int		ReadInteger(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv);
int		ReadFloat(const std::string& filename, const std::string& section, const std::string& key, float *value, float defaultv);
int		ReadColour(const std::string& filename, const std::string& section, const std::string& key, Uint32 *value, Uint32 defaultv);
int		ReadIntArray(const std::string& filename, const std::string& section, const std::string& key, int *array, int num_items);
//int		ReadVec2d(std::string& filename, std::string& section, std::string& key, CVec2d *value);
int		ReadVec(const std::string& filename, const std::string& section, const std::string& key, CVec *value);
//int		ReadRegion(std::string& filename, std::string& section, std::string& key, CRegion *value);




// Keywords
class keyword_t { public:
	std::string key;
	int  Value;
};


int		AddKeyword(const std::string& key, int value);
int		ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv);
bool	ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, bool *value, bool defaultv);









#endif  //  __CONFIGHANDLER_H__
