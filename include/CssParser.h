// OpenLieroX
// code under LGPL

// CSS Parser
// Code taken from htmlcxx (http://htmlcxx.sourceforge.net/) (also LGPL)
// recoded with std::string (28-08-07 by albert)

#ifndef __CSS_PARSER_H__
#define __CSS_PARSER_H__

#include <SDL.h>
#include <string>

struct property_t {
	std::string	sName;
	std::string	sValue;
	bool		bImportant;
	property_t	*tNext;
};

struct node_t {
	std::string	sName;
	bool		bClass; 
	property_t	*tProperties;
	node_t		*tNext;
};

class CCssParser {
public:
	// Constructor
	CCssParser() {
		tNodes = NULL;
	}


private:
	// Attributes
	node_t	*tNodes;

	size_t	iPos;
	size_t	iLength;
	std::string	sData;

	// Internal methods
	bool		AddNode(node_t *tNode);
	bool		AddProperty(property_t *tProperty, node_t *tNode);

	bool		SkipBlank(void);
	bool		SkipComments(void);
	node_t		*ReadNode(void);
	property_t	*ReadProperty(void);


public:
	// Methods
	void		Clear(void);

	bool		Parse(const std::string& sFilename);
	void		BorderProperties(const std::string& val, int *border, Uint32 *LightColour, Uint32 *DarkColour, char *type);
	node_t		*FindNode(const std::string& sNodeName);
	node_t		*FindClass(const std::string& sClassName);
	property_t	*GetProperty(const std::string& sPropertyName, node_t *tNode);
};



#endif 
