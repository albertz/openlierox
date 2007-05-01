/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Custom browser class
// Created 7/6/02
// Jason Boettcher


#ifndef __CBROWSER_H__
#define __CBROWSER_H__


#include "InputEvents.h"


// Notes: Everything is an object. Tags are objects & strings of text are objects
// The renderer goes through each object. Tag objects setup the properties, and string objects get drawn


// Hyper text Objects
enum {
	HTO_TEXT=0,
	HTO_BOLD,
	HTO_UNDERLINE,
	HTO_SHADOW,
	HTO_BOX,
	HTO_COLOUR,
	HTO_TAB,
	HTO_STAB,
	HTO_NEWLINE,
	HTO_LINE
};


// Property flags
enum {
	PRP_BOLD =      0x0001,
	PRP_UNDERLINE = 0x0002,
	PRP_SHADOW =    0x0004
};


// Object structure
class ht_object_t { public:
	int		iType;
	int		iEnd;
	Uint32	iValue;
	std::string	strText;

	ht_object_t *tNext;

};


// Browser events
enum {
	BRW_NONE=-1,
	BRW_SCROLL=0
};


// Browser messages
enum {
	BRM_LOAD=0
};



// Browser class
class CBrowser : public CWidget {
public:
	// Constructor
	CBrowser() {
		tObjects = NULL;
	}

private:
	// Attributes

	// Window attributes
	CScrollbar	cScrollbar;
	int			iLines;
	int			iUseScroll;

	// Properties
	int			iProperties;
	Uint32		iTextColour;

	// Objects
	ht_object_t *tObjects;


	// Reading
	size_t		iPos;
	size_t		iLength;
	char		*sData;



public:
	// Methods


	void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return BRW_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return BRW_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return BRW_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		KeyDown(UnicodeChar c)						{ return BRW_NONE; }
	int		KeyUp(UnicodeChar c)						{ return BRW_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface *bmpDest);
	void	LoadStyle(void) {}

	// Loading
	int			Load(const std::string& sFilename);
	void		ReadObject(void);
	void		ReadNewline(void);
	void		ReadTag(void);
	void		ReadText(void);
	void		AddObject(const std::string& sText, const std::string& sVal, int iType, int iEnd);

};







#endif  //  __CBROWSER_H__
