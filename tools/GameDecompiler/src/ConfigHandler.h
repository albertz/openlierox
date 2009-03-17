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

#include "CVec.h"


#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Internal
int		GetString(char *filename, char *section, char *key, char *string);
//void	TrimSpaces(char *str);


// Value reading
int		ReadString(char *filename, char *section, char *key, char *value, char *defaultv);
int		ReadInteger(char *filename, char *section, char *key, int *value, int defaultv);
int		ReadFloat(char *filename, char *section, char *key, float *value, float defaultv);
//int		ReadVec2d(char *filename, char *section, char *key, CVec2d *value);
int		ReadVec(char *filename, char *section, char *key, CVec *value);
//int		ReadRegion(char *filename, char *section, char *key, CRegion *value);




// Keywords
typedef struct {
	char key[64];
	int  Value;
} keyword_t;


int		AddKeyword(char *key, int value);
int		ReadKeyword(char *filename, char *section, char *key, int *value, int defaultv);









#endif  //  __CONFIGHANDLER_H__