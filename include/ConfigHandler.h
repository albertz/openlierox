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


#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Internal
int		GetString(const tString& filename, const tString& section, const tString& key, tString& string);


// Value reading
int		ReadString(const tString& filename, const tString& section, const tString& key, tString& value, const tString& defaultv);
int		ReadInteger(const tString& filename, const tString& section, const tString& key, int *value, int defaultv);
int		ReadFloat(const tString& filename, const tString& section, const tString& key, float *value, float defaultv);
int		ReadColour(const tString& filename, const tString& section, const tString& key, Uint32 *value, Uint32 defaultv);
int		ReadIntArray(const tString& filename, const tString& section, const tString& key, int *array, int num_items);
//int		ReadVec2d(tString& filename, tString& section, tString& key, CVec2d *value);
int		ReadVec(const tString& filename, const tString& section, const tString& key, CVec *value);
//int		ReadRegion(tString& filename, tString& section, tString& key, CRegion *value);




// Keywords
class keyword_t { public:
	tString key;
	int  Value;
};


int		AddKeyword(const tString& key, int value);
int		ReadKeyword(const tString& filename, const tString& section, const tString& key, int *value, int defaultv);
bool	ReadKeyword(const tString& filename, const tString& section, const tString& key, bool *value, bool defaultv);









#endif  //  __CONFIGHANDLER_H__
