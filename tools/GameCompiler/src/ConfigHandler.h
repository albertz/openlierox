/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Config file handler
// Created 30/9/01
// By Jason Boettcher


#ifndef __CONFIGHANDLER_H__
#define __CONFIGHANDLER_H__

#include <string>
#include "CVec.h"


#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Internal
int		GetString(const char* filename, const char* section, const char* key, char* string);
//void	TrimSpaces(char *str);


// Value reading
int		ReadString(const char* filename, const char* section, const char* key, std::string& value, const char* defaultv);
int		ReadInteger(const char* filename, const char* section, const char* key, int *value, int defaultv);
int		ReadFloat(const char* filename, const char* section, const char* key, float *value, float defaultv);
//int		ReadVec2d(const char* filename, const char* section, const char* key, CVec2d *value);
int		ReadVec(const char* filename, const char* section, const char* key, CVec *value);
//int		ReadRegion(const char* filename, const char* section, const char* key, CRegion *value);




// Keywords
typedef struct {
	char key[64];
	int  Value;
} keyword_t;


int		AddKeyword(const char* key, int value);
int		ReadKeyword(const char* filename, const char* section, const char* key, int *value, int defaultv);









#endif  //  __CONFIGHANDLER_H__
