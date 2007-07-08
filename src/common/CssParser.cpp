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

#include <iostream>
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"

using namespace std;

/////////////////////
// Clears the parser
void CCssParser::Clear(void)
{
	iLength = 0;
	iPos = 0;
	sData = NULL;

	// Already freed
	if (!tNodes)
		return;

	node_t *node = tNodes;
	node_t *next = NULL;
	for(;node;node = next) {

		// Free the node properties
		next = node->tNext;
		property_t *property = node->tProperties;
		property_t *next_prop = NULL;
		if (!property)
			continue;

		for(;property;property = next_prop)  {
			next_prop = property->tNext;
			if (property->sName)
				delete[] property->sName;
			if (property->sValue)
				delete[] property->sValue;
			property->sName = NULL;
			property->sValue = NULL;
			delete property;
		}

		// Free the node name
		if (node->sName)
			delete[] node->sName;
		node->sName = NULL;

		// Free the node
		delete node;
	}

	tNodes = NULL;
}

////////////////////
// Adds a node to the list
bool CCssParser::AddNode(node_t *tNode)
{
	if (!tNode)
		return false;

	tNode->tNext = tNodes;
	tNodes = tNode;

	return true;
}

////////////////////
// Adds a property to the node
bool CCssParser::AddProperty(property_t *tProperty,node_t *tNode)
{
	if (!tNode || !tProperty)
		return false;

	tProperty->tNext = tNode->tProperties;
	tNode->tProperties = tProperty;

	return true;
}

////////////////////
// Finds the specified CSS class, returns NULL when not found
node_t *CCssParser::FindClass(char *sClassName)
{
	// Check for valid parameters
	if (!tNodes || !sClassName)
		return NULL;

	// Find the class
	node_t *node = tNodes;
	for(;node;node=node->tNext)
		if (!strcmp(node->sName,sClassName) && node->bClass)
			return node;

	// Not found
	return NULL;
}

///////////////////
// Finds the specified node, returns NULL when not found
node_t *CCssParser::FindNode(char *sNodeName)
{
	// Check for valid parameters
	if (!tNodes || !sNodeName)
		return NULL;

	// Find the node
	node_t *node = tNodes;
	for(;node;node=node->tNext)
		if (!strcmp(node->sName,sNodeName))
			return node;

	// Not found
	return NULL;
}

/////////////////////
// Finds the Property in Node, returns NULL if the node doesn't contain the property
property_t *CCssParser::GetProperty(char *sPropertyName,node_t *tNode)
{
	// Check for valid parameters
	if (!sPropertyName || !tNode)
		return NULL;

	property_t *property = tNode->tProperties;
	if (!property)
		return NULL;

	// Find the property
	for(;property;property = property->tNext)  {
		if (!property->sName)
			continue;
		if (!stricmp(sPropertyName,property->sName))
			return property;
	}

	// Not found
	return NULL;
}

////////////////
// Skips blank characters
bool CCssParser::SkipBlank(void)
{
	if (!sData)
		return false;

	if (iPos >= iLength)
		return false;

	// Read until blank characters are present
	while (*(sData+iPos) == ' ' || *(sData+iPos) == '\n' || *(sData+iPos) == '\r' || *(sData+iPos) == '\t') {
		// End of data
		if (iPos >= iLength)
			return false;
		iPos++;
	}

	return true;
}

////////////////
// Skips the comment
bool CCssParser::SkipComments(void)
{
	if (!sData)
		return false;

	if (iPos >= iLength)
		return false;

	bool bAllSkipped = false;

	while (!bAllSkipped)  {
		// Skip any blanks
		if (!SkipBlank())
			return false;

		// End of data
		if (iPos+1 >= iLength)
			return false;

		// One line comment
		if (*(sData+iPos) == '/' && *(sData+iPos+1) == '/')  {
			iPos += 2;
			// Read until new line
			while(*(sData+iPos) != '\n')  {
				// End of data
				if (iPos >= iLength)
					return false;
				iPos++;
			}
		}
		// Block comment
		else if (*(sData+iPos) == '/' && *(sData+iPos+1) == '*')  {
			iPos += 2;
			// Read until the end of comment
			while (*(sData+iPos) != '*' && *(sData+iPos+1) != '/')  {
				// End of data
				if (iPos+1 >= iLength)
					return false;
				iPos++;
			}

			// Skip the comment ending (*/)
			iPos += 2;
		}
		// No comment
		else
			bAllSkipped = true;
	}


	return true;
}

/////////////////////////
// Reads one property
property_t *CCssParser::ReadProperty(void)
{
	if (!sData)
		return NULL;

	if (iPos >= iLength)
		return NULL;

	// Ending character of the node, no property can be read
	if (*(sData+iPos) == '}')
		return NULL;

	// Skip blank space and comments
	if (!SkipBlank())
		return NULL;
	if (!SkipComments())
		return NULL;

	// Allocate the property
	property_t *Property = new property_t;
	if (!Property)
		return NULL;

	Property->sName = NULL;
	Property->sValue = NULL;
	Property->bImportant = false;
	Property->tNext = NULL;

	// TODO: use std::string
	// HINT: and change sizeof(buf) later
	static char buf[64];
	buf[63] = '\0';

	unsigned int i=0;

	//
	//	Property name
	//
	while (*(sData+iPos) != ':')  {
		// End of data
		if (iPos >= iLength)
			return Property;

		// Skip any blank characters
		if (!SkipBlank())  {
			delete Property;
			return NULL;
		}

		// After skipping blank characters, this can happen
		if (*(sData+iPos) == ':')
			break;

		// Check for syntax errors
		if (*(sData+iPos) < 65 || *(sData+iPos) > 90)
			if (*(sData+iPos) < 97 || *(sData+iPos) > 122)
				if (*(sData+iPos) != '_' && *(sData+iPos) != '-')  {
					cout
						<< "Syntax error: invalid character '"
						<< *(sData+iPos) << "' ("
						<< (int)*(sData+iPos) 
						<< ") on position " << iPos << endl;
					delete Property;
					Property = NULL;
					return NULL;
				}

		// Too long name
		if (i >= sizeof(buf))  {
			delete Property;
			return NULL;
		}

		buf[i++] = *(sData+iPos);
		iPos++;
	}
	// Terminate the string
	buf[i] = '\0';

	// Trim spaces
	TrimSpaces(buf);

	// Skip the ':' character
	iPos++;

	// Allocate the property name
	size_t buflen = fix_strnlen(buf);
	Property->sName = new char[buflen+1];
	if (!Property->sName)  {
		delete Property;
		return NULL;
	}

	// Copy the property name
	memcpy(Property->sName,buf,buflen+1);

	//
	//	Property value
	//
	buf[0] = 0;
	i = 0;

	// Skip comments and blank characters
	if (!SkipBlank() || !SkipComments())  {
		if (Property->sName)
			delete[] Property->sName;
		Property->sName = NULL;
		delete Property;
		Property = NULL;
		return NULL;
	}


	while (*(sData+iPos) != ';' && *(sData+iPos) != '}')  {
		// End of data
		if (iPos >= iLength)
			return Property;

		// Skip linebreaks
		if (*(sData+iPos) == '\n' || *(sData+iPos) == '\r')  {
			iPos++;
			continue;
		}

		// Check for !important
		if (*(sData+iPos) == '!' && *(sData+iPos-1) == ' ')  {
			iPos++;
			const int imp_length = 9;
			char important[imp_length+1];
			strncpy(important,sData+iPos,imp_length);
			important[imp_length] = '\0';

			// Is the string equal to "important"?
			if (!stricmp("important",important))  {
				Property->bImportant = true;
				iPos += imp_length;
				continue;
			}
		}


		// Too long value
		if (i > sizeof(buf)) {
			if (Property->sName)
				delete[] Property->sName;
			Property->sName = NULL;
			delete Property;
			Property = NULL;
			return NULL;
		}

		buf[i++] = *(sData+iPos);
		iPos++;
	}

	// Terminate the value
	buf[i] = '\0';

	// Trim spaces
	TrimSpaces(buf);

	// Skip the ending character (only ;, NOT } because it's handled by ReadNode)
	if (*(sData+iPos) == ';')
		iPos++;

	// Allocate the value
	buflen = fix_strnlen(buf);
	Property->sValue = new char[buflen+1];
	if (!Property->sValue)  {
		delete[] Property->sName;
		Property->sName = NULL;
		delete Property;
		Property = NULL;
		return NULL;
	}

	// Copy the value
	memcpy(Property->sValue,buf,buflen+1);

	return Property;
}

/////////////////
// Reads one node, returns NULL when error or end of data
node_t *CCssParser::ReadNode(void)
{
	if (!sData)
		return NULL;

	if (iPos >= iLength)
		return NULL;

	// Skip blank space and comments
	if (!SkipBlank())
		return NULL;
	if (!SkipComments())
		return NULL;

	// Allocate the node
	node_t *node = new node_t;
	if (!node)
		return NULL;
	node->sName = NULL;
	node->bClass = false;
	node->tProperties = NULL;
	node->tNext = NULL;

	static char buf[64];
	buf[0] = '\0';

	unsigned int i=0;
	while(*(sData+iPos) != '{')  {
		// End of data
		if (iPos >= iLength) {
			delete node;
			return NULL;
		}

		// Skip any blank characters
		if (!SkipBlank())  {
			delete node;
			return NULL;
		}

		// After blank skipping, this can happen
		if (*(sData+iPos) == '{')
			break;

		// Dot means a class
		if (*(sData+iPos) == '.')  {
			i = 0;
			iPos++;
			node->bClass = true;
			continue;
		}

		// Check for syntax errors
		if (*(sData+iPos) < 'A' || *(sData+iPos) > 'Z')
			if (*(sData+iPos) < 'a' || *(sData+iPos) > 'z')
				if (*(sData+iPos) != '_' && *(sData+iPos) != '-')  {
					cout
						<< "Syntax error: invalid character '"
						<< *(sData+iPos) << "' ("
						<< (int)*(sData+iPos) 
						<< ") on position " << iPos << endl;
					delete node;
					return NULL;
				}

		// Too long name
		if (i > sizeof(buf))  {
			delete node;
			return NULL;
		}

		buf[i++] = *(sData+iPos);
		iPos++;
	}
	// Terminate the buffer
	buf[i] = '\0';

	// Trim the spaces
	TrimSpaces(buf);

	// Skip the { character
	iPos++;

	// Allocate the name
	size_t buflen = fix_strnlen(buf);
	node->sName = new char[buflen+1];
	if (!node->sName)  {
		delete node;
		return NULL;
	}

	// Copy the name
	memcpy(node->sName,buf,buflen+1);

	// Read & add the properties
	while (AddProperty(ReadProperty(),node))
		continue;

	// Skip the node ending character ('}')
	iPos++;

	return node;
}

////////////////////
// Parses the specified file
bool CCssParser::Parse(char *sFilename)
{
	// Clear previous data
	Clear();

	// Check parameters
	if (!sFilename)
		return false;
	if (!strcmp("",sFilename))
		return false;

	// Open the file
	FILE *fp = OpenGameFile(sFilename,"rb");
	if (!fp)
		return false;

	// Get file size
	if (fseek(fp,0,SEEK_END))  {
		fclose(fp);
		return false;
	}
	iLength = ftell(fp);
	if (fseek(fp,0,SEEK_SET))  {
		fclose(fp);
		return false;
	}

	// Get the data
	sData = new char[iLength+1];
	if (!sData)
		return false;
	iLength = fread(sData,1,iLength,fp);
	sData[iLength] = '\0';

	// Set the properties
	iPos = 0;

	// Parse the file
	while (AddNode(ReadNode()))
		continue;

	// Free the data
	delete[] sData;
	sData = NULL;
	iPos = 0;
	iLength = 0;

	return true;
}

/////////////////
// Parses border properties from a string
// TODO: make it use std::string!!
void CCssParser::BorderProperties(char *val,int *border,Uint32 *LightColour,Uint32 *DarkColour,uchar *type)
{
	// Defaults
	*border = 2;
	*LightColour = MakeColour(200,200,200);
	*DarkColour = MakeColour(64,64,64);
	*type = BX_OUTSET; // Outset

	// Check
	if (!val)
		return;

	// Trim spaces
	TrimSpaces(val);

	// Remove duplicate spaces
	std::string tmp = val;
	while (replace(tmp,"  "," ",tmp))
		continue;
	strcpy(val, tmp.c_str()); // TODO: this has to be done better...
	

	char *tok = strtok(val," ");
	if (!tok)
		return;

	// Border width
	*border = atoi(tok); tok = strtok(NULL," ");
	if (!tok)
		return;

	// Border type
	if (!stricmp("solid",tok)) *type = BX_SOLID;
	else if (!stricmp("inset",tok)) *type = BX_INSET;
	else *type = BX_OUTSET;
	tok = strtok(NULL," ");
	if (!tok)
		return;

	// Dark colour
	*DarkColour = StrToCol(tok);
	tok = strtok(NULL," ");
	if (!tok)
		return;

	// Light colour
	if (*type != BX_SOLID)  {
		*LightColour = StrToCol(tok);
	}
	else
		*LightColour = *DarkColour;
}

