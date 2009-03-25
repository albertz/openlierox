/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Config file handler
// Created 30/9/01
// By Jason Boettcher


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale>
#include "ConfigHandler.h"

// TODO: get rid of this file and use OLX code

#ifndef _MSC_VER
#define stricmp strcasecmp
#endif

int			NumKeywords = 0;
keyword_t	Keywords[MAX_KEYWORDS];


///////////////////
// Trim the leading & ending spaces from a string
char *TrimSpaces(char *szLine)
{
    // Remove preceeding spaces
    while( !isgraph(*szLine) && isspace(*szLine) )
        szLine++;

    // Get rid of the ending spaces
	int i;
    for (i = (int)strlen(szLine) - 1; i>=0; i--) {
        if (isgraph(szLine[i]) || !isspace(szLine[i]))
            break;
    }
    szLine[i+1] = '\0';

    return szLine;
}


///////////////////
// Add a keyword to the list
int AddKeyword(const char* key, int value)
{
	// Check for enough spaces
	if(NumKeywords >= MAX_KEYWORDS-1)
		return false;

	strcpy(Keywords[NumKeywords].key,key);
	Keywords[NumKeywords++].Value = value;

	return true;
}



///////////////////
// Read a keyword from a file
int ReadKeyword(const char* filename, const char* section, const char* key, int *value, int defaultv)
{
	int n;
	char string[MAX_MINOR_LENGTH];
	
	*value = defaultv;

	if(!GetString(filename,section,key,string))
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
// Read an interger from a file
int ReadInteger(const char* filename, const char* section, const char* key, int *value, int defaultv)
{
	char string[MAX_MINOR_LENGTH];

	*value = defaultv;
	
	if(!GetString(filename,section,key,string))
		return false;

	*value = atoi(string);

	return true;
}


///////////////////
// Read a string from a file
int ReadString(const char* filename, const char* section, const char* key, std::string& value, const char* defaultv)
{
	value = defaultv;

	char tmp[256]; tmp[0] = 0;
	int ret = 0;
	if(ret = GetString(filename,section,key,tmp)) {
		value = tmp;
	}
	return ret;
}


///////////////////
// Read a float from a file
int ReadFloat(const char* filename, const char* section, const char* key, float *value, float defaultv)
{
	char string[MAX_MINOR_LENGTH];

	*value = defaultv;
	
	if(!GetString(filename,section,key,string))
		return false;

	*value = (float)atof(string);

	return true;
}








///////////////////
// Read a string
int GetString(const char* filename, const char* section, const char* key, char* string)
{
	FILE	*config;
	char	Line[MAX_STRING_LENGTH];
	char	curSection[256];
	char	temp[MAX_STRING_LENGTH];
	char	curKey[MAX_STRING_LENGTH];
	char	*chardest = NULL;
	int		Position;
	int		found = false;
	
	if(strcmp(filename,"") == 0)
		return false;

	config = fopen(filename,"rt");
	if(!config)	
		return false;

	strcpy(string,"");
	strcpy(curSection,"");
	strcpy(temp,"");
	strcpy(curKey,"");


	while(!feof(config))
	{
		// Parse the lines
		fscanf(config,"%[^\n]\n",Line);
		strcpy(Line, TrimSpaces(Line));
		
		
		///////////////////
		// Comment, Ignore
		if(Line[0] == '#')				
			continue;

		////////////
		// Sections
		if(Line[0] == '[' && Line[strlen(Line)-1] == ']')
		{
			strcpy(temp,Line+1);
			temp[strlen(temp)-1] = '\0';
			strcpy(curSection,temp);
			continue;
		}

		////////
		// Keys
		chardest = strchr(Line,'=');
		if(chardest != NULL)
		{
			// Key
			Position = (int)(chardest - Line) + 1;
			strcpy(curKey,Line);
			curKey[Position-1] = '\0';
			strcpy(curKey, TrimSpaces(curKey));

			// Check if this is the key were looking for under the section were looking for
			if(stricmp(curKey,key) == 0 && stricmp(curSection,section) == 0)
			{				
				// Get the value
				strcpy(string,Line+Position);
				strcpy(string, TrimSpaces(string));
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
	strcpy(temp,str+i);


	// proceeding spaces
	for(n=strlen(temp)-1;n>0;n--)
		if(temp[n] != ' ' && temp[n] != '\t')
		{
			temp[n+1] = '\0';
			break;
		}

	strcpy(str,temp);
}*/
