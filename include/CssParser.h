// OpenLieroX

// CSS Parser
// Code taken from htmlcxx (http://htmlcxx.sourceforge.net/) (also LGPL)

#ifndef __CSS_PARSER_H__
#define __CSS_PARSER_H__

// TODO: recode everything here! (use UCString)

typedef struct property_s {
	char		*sName;
	char		*sValue;
	bool		bImportant;
	property_s	*tNext;
} property_t;

typedef struct node_s {
	char		*sName;
	bool		bClass; 
	property_t	*tProperties;
	node_s		*tNext;
} node_t;

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
	char	*sData;

	// Internal methods
	bool		AddNode(node_t *tNode);
	bool		AddProperty(property_t *tProperty,node_t *tNode);

	bool		SkipBlank(void);
	bool		SkipComments(void);
	node_t		*ReadNode(void);
	property_t	*ReadProperty(void);


public:
	// Methods
	void		Clear(void);

	bool		Parse(char *sFilename);
	void		BorderProperties(char *val,int *border,Uint32 *LightColour,Uint32 *DarkColour,uchar *type);
	node_t		*FindNode(char *sNodeName);
	node_t		*FindClass(char *sClassName);
	property_t	*GetProperty(char *sPropertyName, node_t *tNode);
};



#endif 
