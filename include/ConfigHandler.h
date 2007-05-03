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
int		GetString(const UCString& filename, const UCString& section, const UCString& key, UCString& string);


// Value reading
int		ReadString(const UCString& filename, const UCString& section, const UCString& key, UCString& value, const UCString& defaultv);
int		ReadInteger(const UCString& filename, const UCString& section, const UCString& key, int *value, int defaultv);
int		ReadFloat(const UCString& filename, const UCString& section, const UCString& key, float *value, float defaultv);
int		ReadColour(const UCString& filename, const UCString& section, const UCString& key, Uint32 *value, Uint32 defaultv);
int		ReadIntArray(const UCString& filename, const UCString& section, const UCString& key, int *array, int num_items);
//int		ReadVec2d(UCString& filename, UCString& section, UCString& key, CVec2d *value);
int		ReadVec(const UCString& filename, const UCString& section, const UCString& key, CVec *value);
//int		ReadRegion(UCString& filename, UCString& section, UCString& key, CRegion *value);




// Keywords
class keyword_t { public:
	UCString key;
	int  Value;
};


int		AddKeyword(const UCString& key, int value);
int		ReadKeyword(const UCString& filename, const UCString& section, const UCString& key, int *value, int defaultv);
bool	ReadKeyword(const UCString& filename, const UCString& section, const UCString& key, bool *value, bool defaultv);









#endif  //  __CONFIGHANDLER_H__
