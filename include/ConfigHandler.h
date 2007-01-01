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


#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Internal
int		GetString(const char *filename, const char *section, const char *key, char *string);
//void	TrimSpaces(char *str);


// Value reading
int		ReadString(const char *filename, const char *section, const char *key, char *value, const char *defaultv);
int		ReadInteger(const char *filename, const char *section, const char *key, int *value, int defaultv);
int		ReadFloat(const char *filename, const char *section, const char *key, float *value, float defaultv);
int		ReadColour(const char *filename, const char *section, const char *key, Uint32 *value, Uint32 defaultv);
//int		ReadVec2d(char *filename, char *section, char *key, CVec2d *value);
int		ReadVec(const char *filename, const char *section, const char *key, CVec *value);
//int		ReadRegion(char *filename, char *section, char *key, CRegion *value);




// Keywords
typedef struct {
	char key[64];
	int  Value;
} keyword_t;


int		AddKeyword(char *key, int value);
int		ReadKeyword(const char *filename, const char *section, const char *key, int *value, int defaultv);
bool	ReadKeyword(const char *filename, const char *section, const char *key, bool *value, bool defaultv);









#endif  //  __CONFIGHANDLER_H__
