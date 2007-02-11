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


#include "defs.h"
#include "LieroX.h"


int			NumKeywords = 0;
keyword_t	Keywords[MAX_KEYWORDS];


///////////////////
// Add a keyword to the list
int AddKeyword(char *key, int value)
{
	// Check for enough spaces
	if(NumKeywords >= MAX_KEYWORDS-1)
		return false;

	fix_strncpy(Keywords[NumKeywords].key,key);
	Keywords[NumKeywords++].Value = value;

	return true;
}



///////////////////
// Read a keyword from a file
int ReadKeyword(const char *filename, const char *section, const char *key, int *value, int defaultv)
{
	int n;
	static char string[MAX_MINOR_LENGTH];
	
	*value = defaultv;

	if(!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;

	// Try and find a keyword with matching keys
	for(n=0;n<NumKeywords;n++) {
		if(stricmp(string,Keywords[n].key) == 0) {
			*value = Keywords[n].Value;
			return true;
		}
	}

	return false;
}

///////////////////
// Read a keyword from a file (bool version)
bool ReadKeyword(const char *filename, const char *section, const char *key, bool *value, bool defaultv)
{
	int n;
	static char string[MAX_MINOR_LENGTH];
	
	*value = defaultv;

	if(!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;

	// Try and find a keyword with matching keys
	for(n=0;n<NumKeywords;n++) {
		if(stricmp(string,Keywords[n].key) == 0) {
			*value = Keywords[n].Value != 0;
			return true;
		}
	}

	return false;
}


///////////////////
// Read an interger from a file
int ReadInteger(const char *filename, const char *section, const char *key, int *value, int defaultv)
{
	static char string[MAX_MINOR_LENGTH];

	*value = defaultv;
	
	if(!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;
	fix_markend(string);
	
	*value = atoi(string);

	return true;
}


///////////////////
// Read a string from a file
int ReadString(const char *filename, const char *section, const char *key, char *value, size_t maxvaluelen, const char *defaultv)
{
	if(defaultv != NULL) dyn_strncpy(value,defaultv,maxvaluelen);

	return GetString(filename,section,key,value,maxvaluelen);

	/*int result = GetString(filename,section,key,value);

	if (strlen(value) <= 0)
		strcpy(value,defaultv);

	return result;*/
}


///////////////////
// Read a float from a file
int ReadFloat(const char *filename, const char *section, const char *key, float *value, float defaultv)
{
	static char string[MAX_MINOR_LENGTH];

	*value = defaultv;
	
	if(!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;

	*value = (float)atof(string);

	return true;
}


//////////////////
// Read a colour
int ReadColour(const char *filename, const char *section, const char *key, Uint32 *value, Uint32 defaultv)
{
	static char string[MAX_MINOR_LENGTH];

	*value = defaultv;
	
	if(!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;

	*value = StrToCol(string);

	return true;
	
}

//////////////////
// Reads an array of integers
int ReadIntArray(const char *filename, const char *section, const char *key, int *array, int num_items)
{
	static char string[MAX_MINOR_LENGTH];

	if (!GetString(filename,section,key,string,MAX_MINOR_LENGTH))
		return false;

	char *tok = strtok(string,",");
	int i=0;
	while(tok && i < num_items)  {
		array[i++] = atoi(tok);
		tok = strtok(NULL,",");
	}

	return i == num_items-1;
}



///////////////////
// Read a string
// HINT: string has to be MAX_MINOR_LENGTH long
int GetString(const char *filename, const char *section, const char *key, char *string, size_t maxstrlen)
{
	FILE	*config;
	static char	Line[MAX_STRING_LENGTH];
	static char	tmpLine[MAX_STRING_LENGTH];
	static char	curSection[512];
	static char	temp[MAX_STRING_LENGTH];
	static char	curKey[MAX_STRING_LENGTH];
	char	*chardest = NULL;
	int		Position;
	int		found = false;

	if (!filename || !section || !key || !string)
		return false;
	
	if(strcmp(filename,"") == 0)
		return false;

	config = OpenGameFile(filename,"rt");
	if(!config)	
		return false;

	strcpy(string,"");
	strcpy(curSection,"");
	strcpy(temp,"");
	strcpy(curKey,"");


	while(!feof(config))
	{
		// Parse the lines
		fscanf(config,"%[^\n]\n",tmpLine);
		fix_strncpy(Line, TrimSpaces(tmpLine));
		
		///////////////////
		// Comment, Ignore
		if(Line[0] == '#')				
			continue;

		////////////
		// Sections
		if(Line[0] == '[' && Line[fix_strnlen(Line)-1] == ']')
		{
			fix_strncpy(temp,Line+1);
			temp[fix_strnlen(temp)-1] = '\0';
			fix_strncpy(curSection,temp);
			continue;
		}

		////////
		// Keys
		chardest = strchr(Line,'=');
		if(chardest != NULL)
		{
			// Key
			Position = chardest - Line + 1;
			fix_strncpy(tmpLine,Line);
			tmpLine[Position-1] = '\0';
			fix_strncpy(curKey, TrimSpaces(tmpLine));

			// Check if this is the key were looking for under the section were looking for
			if(stricmp(curKey,key) == 0 && stricmp(curSection,section) == 0)
			{				
				// Get the value
				fix_strncpy(tmpLine,Line+Position);
				dyn_strncpy(string, TrimSpaces(tmpLine), maxstrlen);
				found = true;
				break;
			}
			continue;
		}
	}

	fclose(config);

	return found;
}



///////////////////
// Trim the spaces from the start & end of a string
/*void TrimSpaces(char *str)
{
	if(!str)
		return;

	char temp[MAX_STRING_LENGTH];
	unsigned int n = 0;
	unsigned int i = 0;
	strcpy(temp,"");


	// Leading spaces
	for(n=0,i=0;n<strlen(str);n++)
	{
		if(str[n] != ' ' && temp[n] != '\t')
			break;
		i++;
	}
	fix_strncpy(temp,str+i);


	// proceeding spaces
	for(n=strlen(temp)-1;n>0;n--)
		if(temp[n] != ' ' && temp[n] != '\t')
		{
			temp[n+1] = '\0';
			break;
		}

	strcpy(str,temp);
}*/
