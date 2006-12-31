/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Input box
// Created 31/7/02
// Jason Boettcher


#ifndef __CINPUTBOX_H__
#define __CINPUTBOX_H__


// Inputbox events
enum {
	INB_NONE=-1,
	INB_MOUSEUP
};


// inputbox messages
enum {
	INM_GETVALUE=0,
	INM_GETTEXT
};


class CInputbox : public CWidget {
public:
	// Constructor
	CInputbox(int val, char *_text, SDL_Surface *img, char *name) {
		iKeyvalue = val;
		fix_strncpy(sText,_text);
		fix_strncpy(sName, name);

		bmpImage = img;
		iType = wid_Inputbox;
		iMouseOver = false;
	}


private:
	// Attributes

	int			iKeyvalue;
	char		sText[32];
	SDL_Surface	*bmpImage;
	int			iMouseOver;
	char		sName[32];


public:
	// Methods

	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	inline int		MouseOver(mouse_t *tMouse)			{ iMouseOver = true; return INB_NONE; }
	inline int		MouseUp(mouse_t *tMouse, int nDown)		{ return INB_MOUSEUP; }
	inline int		MouseDown(mouse_t *tMouse, int nDown)	{ return INB_NONE; }
	inline int		MouseWheelDown(mouse_t *tMouse)		{ return INB_NONE; }
	inline int		MouseWheelUp(mouse_t *tMouse)		{ return INB_NONE; }
	inline int		KeyDown(int c)						{ return INB_NONE; }
	inline int		KeyUp(int c)						{ return INB_NONE; }	


	// Process a message sent
	inline int		SendMessage(int iMsg, DWORD Param1, DWORD Param2) {

				switch(iMsg) {
					case INM_GETVALUE:
						return iKeyvalue;
					case INM_GETTEXT:
						strcpy( (char *)Param1, sText);
						return 0;
				}

				return 0;
			}

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest);

	inline void	LoadStyle(void) {}


	inline int		getValue(void)						{ return iKeyvalue; }
	inline void	setValue(int _v)					{ iKeyvalue = _v; }
	inline char	*getText(void)						{ return sText; }
	inline void	setText(char *_t)					{ if(_t) fix_strncpy(sText,_t); }
	inline char	*getName(void)						{ return sName; }

};



#endif  //  __CINPUTBOX_H__
