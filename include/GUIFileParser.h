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

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


// Notes: Everything is an object. Tags are objects & strings of text are objects
// The renderer goes through each object. Tag objects setup the properties, and string objects get drawn

// Macros
#define CMP(str1,str2)  !xmlStrcmp((const xmlChar *)str1,(const xmlChar *)str2)


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
	ERR_UNKNOWNPROPERTY,
	ERR_COULDNOTPARSE,
	ERR_EMPTYDOC,
	ERR_INVALIDROOT
};

// Property flags
enum {
	PROP_BOLD =      0x0001,
	PROP_UNDERLINE = 0x0002,
	PROP_SHADOW =    0x0004
};

// Layout IDs
enum {
	L_MAINMENU=0,
	L_LOCALPLAY,
	L_GAMESETTINGS,
	L_WEAPONOPTIONS,
	L_LOADWEAPONS,
	L_SAVEWEAPONS,
	L_NET,
	L_NETINTERNET,
	L_INTERNETDETAILS,
	L_ADDSERVER,
	L_NETLAN,
	L_LANDETAILS,
	L_NETHOST,
	L_NETFAVOURITES,
	L_FAVOURITESDETAILS,
	L_RENAMESERVER,
	L_ADDFAVOURITE,
	L_CONNECTING,
	L_NETJOINLOBBY,
	L_NETHOSTLOBBY,
	L_SERVERSETTINGS,
	L_BANLIST,
	L_PLAYERPROFILES,
	L_CREATEPLAYER,
	L_VIEWPLAYERS,
	L_LEVELEDITOR,
	L_NEWDIALOG,
	L_SAVELOADLEVEL,
	L_OPTIONS,
	L_OPTIONSCONTROLS,
	L_OPTIONSGAME,
	L_OPTIONSSYSTEM,
	L_MESSAGEBOXOK,
	L_MESSAGEBOXYESNO,
	LAYOUT_COUNT
};

// Generic events
typedef struct generic_events_s {
	char	onmouseover[64];
	char	onmouseout[64];
	char	onmousedown[64];
	char	onclick[64];
} generic_events_t;

// Widget list item structure
typedef struct widget_item_s {
	int				iID;
	char			*sName;
	widget_item_s	*tNext;
} widget_item_t;

// Widget list class
class CWidgetList {
public:
	CWidgetList() {
		tItems = NULL;
		iCount = 0;
	}

private:
	// Attributes
	widget_item_t	*tItems;
	int				iCount;

public:
	// Methods
	int		getCount(void)	{return iCount; }
	int		Add(char *Name);
	char	*getName(int ID);
	int		getID(const char *Name);
	void	Shutdown(void);
};


// Browser class
class CParser {
public:
	// Constructor
	CParser() {

	}

private:
	// Attributes
	xmlDocPtr	tDocument;
	xmlNodePtr	tCurrentNode;

	CWidgetList	LayoutWidgets[LAYOUT_COUNT];


public:
	// Contructor and destructor
	void		Create(void);
	void		Destroy(void);

	// Methods
	int			BuildLayout(CGuiLayout *Layout, char *sFilename);
	int			GetIdByName(char *Name,int LayoutID);
	CWidgetList	*GetLayoutWidgets(int LayoutID);

	// Error handling
	void	Error(int Code, char *Format, ...);
};







#endif  //  __CBROWSER_H__
