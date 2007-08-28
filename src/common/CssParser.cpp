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
#include <string.h>
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"

using namespace std;

// Disable this warnings before we turn this to std::string
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

/////////////////////
// Clears the parser
void CCssParser::Clear(void)
{
	iLength = 0;
	iPos = 0;
	sData = "";

	// Already freed
	if (!tNodes)
		return;

	node_t *node = tNodes;
	node_t *next = NULL;
	for(; node; node = next) {

		// Free the node properties
		next = node->tNext;
		property_t *property = node->tProperties;
		property_t *next_prop = NULL;
		if (!property)
			continue;

		for(;property;property = next_prop)  {
			next_prop = property->tNext;
			delete property;
		}

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
bool CCssParser::AddProperty(property_t *tProperty, node_t *tNode)
{
	if (!tNode || !tProperty)
		return false;

	tProperty->tNext = tNode->tProperties;
	tNode->tProperties = tProperty;

	return true;
}

////////////////////
// Finds the specified CSS class, returns NULL when not found
node_t *CCssParser::FindClass(const std::string& sClassName)
{
	// Check for valid parameters
	if (!tNodes)
		return NULL;

	// Find the class
	node_t *node = tNodes;
	for(; node; node=node->tNext)
		if (node->sName == sClassName && node->bClass)
			return node;

	// Not found
	return NULL;
}

///////////////////
// Finds the specified node, returns NULL when not found
node_t *CCssParser::FindNode(const std::string& sNodeName)
{
	// Check for valid parameters
	if (!tNodes)
		return NULL;

	// Find the node
	node_t *node = tNodes;
	for(; node; node=node->tNext)
		if (node->sName == sNodeName)
			return node;

	// Not found
	return NULL;
}

/////////////////////
// Finds the Property in Node, returns NULL if the node doesn't contain the property
property_t *CCssParser::GetProperty(const std::string& sPropertyName, node_t *tNode)
{
	// Check for valid parameters
	if (!tNode)
		return NULL;

	property_t *property = tNode->tProperties;
	if (!property)
		return NULL;

	// Find the property
	for(;property;property = property->tNext)  {
		if (!stringcasecmp(sPropertyName, property->sName))
			return property;
	}

	// Not found
	return NULL;
}

////////////////
// Skips blank characters
bool CCssParser::SkipBlank(void)
{
	if (iPos >= iLength)
		return false;

	// Read until blank characters are present
	while (sData[iPos] == ' ' || sData[iPos] == '\n' || sData[iPos] == '\r' || sData[iPos] == '\t') {
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
		if (sData[iPos] == '/' && sData[iPos+1] == '/')  {
			iPos += 2;
			// Read until new line
			while(sData[iPos] != '\n')  {
				// End of data
				if (iPos >= iLength)
					return false;
				iPos++;
			}
		}
		// Block comment
		else if (sData[iPos] == '/' && sData[iPos+1] == '*')  {
			iPos += 2;
			// Read until the end of comment
			while (sData[iPos] != '*' && sData[iPos+1] != '/')  {
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
	if (iPos >= iLength)
		return NULL;

	// Ending character of the node, no property can be read
	if (sData[iPos] == '}')
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

	Property->sName = "";
	Property->sValue = "";
	Property->bImportant = false;
	Property->tNext = NULL;

	// TODO: use std::string
	// HINT: and change sizeof(buf) later
	std::string buf;

	unsigned int i=0;

	//
	//	Property name
	//
	while (sData[iPos] != ':')  {
		// End of data
		if (iPos >= iLength)
			return Property;

		// Skip any blank characters
		if (!SkipBlank())  {
			delete Property;
			return NULL;
		}

		// After skipping blank characters, this can happen
		if (sData[iPos] == ':')
			break;

		// Check for syntax errors
		if (sData[iPos] < 65 || sData[iPos] > 90)
			if (sData[iPos] < 97 || sData[iPos] > 122)
				if (sData[iPos] != '_' && sData[iPos] != '-')  {
					cout
						<< "Syntax error: invalid character '"
						<< sData[iPos]
						<< "' on position " << iPos << endl;
					delete Property;
					Property = NULL;
					return NULL;
				}

		// Too long name
		// HINT: this magic constant 64 was the buffer-length before
		// TODO: should we remove this limit? (perhaps this engine works better with some limit)
		if (i >= 64)  {
			delete Property;
			return NULL;
		}

		buf[i++] = sData[iPos];
		iPos++;
	}

	// Trim spaces
	TrimSpaces(buf);

	// Skip the ':' character
	iPos++;

	// Copy the property name
	Property->sName = buf;

	//
	//	Property value
	//
	buf = "";
	i = 0;

	// Skip comments and blank characters
	if (!SkipBlank() || !SkipComments())  {
		Property->sName = "";
		delete Property;
		Property = NULL;
		return NULL;
	}


	while (sData[iPos] != ';' && sData[iPos] != '}')  {
		// End of data
		if (iPos >= iLength)
			return Property;

		// Skip linebreaks
		if (sData[iPos] == '\n' || sData[iPos] == '\r')  {
			iPos++;
			continue;
		}

		// Check for !important
		if (iPos > 0 && sData[iPos] == '!' && sData[iPos-1] == ' ')  {
			iPos++;
			const int imp_length = 9; // "important".size

			// Is the string equal to "important"?
			if (!stringcasecmp("important", sData.substr(iPos, imp_length)))  {
				Property->bImportant = true;
				iPos += imp_length;
				continue;
			}
		}


		// Too long value
		// HINT: and again, we have this magic constant; see above ...
		if (i > 64) {
			Property->sName = "";
			delete Property;
			Property = NULL;
			return NULL;
		}

		buf[i++] = sData[iPos];
		iPos++;
	}

	// Trim spaces
	TrimSpaces(buf);

	// Skip the ending character (only ;, NOT } because it's handled by ReadNode)
	if (sData[iPos] == ';')
		iPos++;

	// Copy the value
	Property->sValue = buf;

	return Property;
}

/////////////////
// Reads one node, returns NULL when error or end of data
node_t *CCssParser::ReadNode(void)
{
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
	node->sName = "";
	node->bClass = false;
	node->tProperties = NULL;
	node->tNext = NULL;

	std::string buf;

	unsigned int i=0;
	while(sData[iPos] != '{')  {
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
		if (sData[iPos] == '{')
			break;

		// Dot means a class
		if (sData[iPos] == '.')  {
			i = 0;
			iPos++;
			node->bClass = true;
			continue;
		}

		// Check for syntax errors
		if (sData[iPos] < 'A' || sData[iPos] > 'Z')
			if (sData[iPos] < 'a' || sData[iPos] > 'z')
				if (sData[iPos] != '_' && sData[iPos] != '-')  {
					cout
						<< "Syntax error: invalid character '"
						<< sData[iPos]
						<< "' on position " << iPos << endl;
					delete node;
					return NULL;
				}

		// Too long name
		// HINT: again the magic constant...; see above ...
		if (i > 64)  {
			delete node;
			return NULL;
		}

		buf[i++] = sData[iPos];
		iPos++;
	}

	// Trim the spaces
	TrimSpaces(buf);

	// Skip the { character
	iPos++;

	// Copy the name
	node->sName = buf;

	// Read & add the properties
	while (AddProperty(ReadProperty(), node))
		continue;

	// Skip the node ending character ('}')
	iPos++;

	return node;
}

////////////////////
// Parses the specified file
bool CCssParser::Parse(const std::string& sFilename)
{
	// Clear previous data
	Clear();

	// Check parameters
	if (sFilename == "")
		return false;

	// Open the file
	FILE *fp = OpenGameFile(sFilename, "rb");
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
	freadstr(sData, iLength, fp);

	// Set the properties
	iPos = 0;

	// Parse the file
	while (AddNode(ReadNode()))
		continue;

	// Free the data
	sData = "";
	iPos = 0;
	iLength = 0;

	return true;
}

/////////////////
// Parses border properties from a string
void CCssParser::BorderProperties(const std::string& val, int *border, Uint32 *LightColour, Uint32 *DarkColour, char *type)
{
	// Defaults
	*border = 2;
	*LightColour = MakeColour(200,200,200);
	*DarkColour = MakeColour(64,64,64);
	*type = BX_OUTSET; // Outset

	// Trim spaces
	std::string tmp = val;
	TrimSpaces(tmp);

	// Remove duplicate spaces
	while (replace(tmp,"  "," ",tmp))
		continue;
	
	size_t tok = tmp.find(" ");
	if(tok == tmp.npos)
		return;

	// Border width
	*border = atoi(tmp.substr(0, tok));
	tmp.erase(tok);
	tok = tmp.find(" ");
	if(tok == tmp.npos)
		return;

	// Border type
	if (!stringcasecmp("solid", tmp.substr(0, tok)))
		*type = BX_SOLID;
	else if (!stringcasecmp("inset", tmp.substr(0, tok)))
		*type = BX_INSET;
	else
		*type = BX_OUTSET;
	
	tmp.erase(tok);
	tok = tmp.find(" ");
	if(tok == tmp.npos)
		return;

	// Dark colour
	*DarkColour = StrToCol(tmp.substr(0, tok));
	tmp.erase(tok);
	tok = tmp.find(" ");
	if(tok == tmp.npos)
		return;

	// Light colour
	if (*type != BX_SOLID)  {
		*LightColour = StrToCol(tmp.substr(0, tok));
	}
	else
		*LightColour = *DarkColour;
}

// Enable the warning
#ifdef _MSC_VER
#pragma warning(default: 4996)
#endif

