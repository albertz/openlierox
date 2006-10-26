/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Custom browser class
// Created 7/6/02
// Jason Boettcher


#ifndef __CPARSER_H__
#define __CPARSER_H__


// Notes: Everything is an object. Tags are objects & strings of text are objects
// The renderer goes through each object. Tag objects setup the properties, and string objects get drawn


// Objects
enum {
	O_TEXT=0,
	O_BOLD,
	O_UNDERLINE,
	O_ITALIC,
	O_SHADOW,
	O_FRAME,
	O_IMG,
	O_BUTTON,
	O_CHECKBOX,
	O_COMBOBOX,
	O_INPUTBOX,
	O_LABEL,
	O_LISTVIEW,
	O_MENU,
	O_SCROLLBAR,
	O_SLIDER,
	O_TEXTBOX
};

// Errors
enum {
	ERR_OUTOFMEMORY=0,
	ERR_UNKNOWNPROPERTY
};

// Property flags
enum {
	PROP_BOLD =      0x0001,
	PROP_UNDERLINE = 0x0002,
	PROP_SHADOW =    0x0004
};


// Value structure
typedef struct  property_s {
	int		iValue;
	char	sValue[32];
	char	sName[32];
} property_t;

// Object structure
typedef struct  tag_object_s {
	int			iType;
	int			iEnd;
	property_t	*cProperties;
	int			iNumProperties;
	char		*strText;

	struct  tag_object_s *tNext;

} tag_object_t;



// Browser class
class CParser {
public:
	// Constructor
	CParser() {
		tObjects = NULL;
	}

private:
	// Attributes

	// Objects
	tag_object_t *tObjects;


	// Reading
	int			iLines;
	long		iPos;
	long		iLength;
	char		*sData;



public:
	// Methods


	void	Create(void);
	void	Destroy(void);

	void	BuildLayout(CGuiLayout *Layout);
	int		GetIdByName(char *name);

	// Error handling
	void	Error(int Code, char *Format, ...);

	// Loading
	int			Load(char *sFilename);
	void		ReadObject(void);
	void		ReadNewline(void);
	void		ReadTag(void);
	void		ReadText(void);
	void		AddObject(char *sText, property_t *cProperties, int iNumProperties, int iType, int iEnd);

};







#endif  //  __CBROWSER_H__
