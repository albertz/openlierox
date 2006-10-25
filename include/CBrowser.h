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


#ifndef __CBROWSER_H__
#define __CBROWSER_H__


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
typedef struct  ht_object_s {
	int		iType;
	int		iEnd;
	Uint32	iValue;
	char	*strText;

	struct  ht_object_s *tNext;

} ht_object_t;


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
	long		iPos;
	long		iLength;
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
	int		KeyDown(int c)						{ return BRW_NONE; }
	int		KeyUp(int c)						{ return BRW_NONE; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2);

	void	Draw(SDL_Surface *bmpDest);

	// Loading
	int			Load(char *sFilename);
	void		ReadObject(void);
	void		ReadNewline(void);
	void		ReadTag(void);
	void		ReadText(void);
	void		AddObject(char *sText, char *sVal, int iType, int iEnd);

};







#endif  //  __CBROWSER_H__
